#!/usr/bin/env bash
# swap_container.sh — A/B switch from the live container to a new image,
# with a smoke test and automatic rollback if it fails.
#
#   swap_container.sh <NEW_IMAGE_TAG> [STAGING_NAME]
#
#   NEW_IMAGE_TAG  required, e.g. yahboom_ros2_humble:v1.0-dev
#   STAGING_NAME   default: <current>_staging
#
# Sequence:
#   1. Read current CONTAINER/IMAGE from container.env.
#   2. Stop the watchdog + camera-watcher systemd units (so they don't
#      fight the swap or false-restart against a half-up container).
#   3. docker stop the current container (kept, NOT removed → rollback).
#   4. run_container.sh NEW_IMAGE_TAG STAGING_NAME.
#   5. Smoke test the staging container (ROS + overlay + web_video_server
#      prefix + Rosmaster_Lib import + entrypoint).
#   6a. PASS → rewrite container.env (CONTAINER=staging, IMAGE=new),
#       restart watchdog units (now managing staging), print promote +
#       rollback instructions. Old container left stopped for rollback.
#   6b. FAIL → stop+rm staging, docker start the old container, restart
#       watchdog units on the original, exit 1.
#
# This never auto-renames. After a green swap you run on the staging
# container; promote (rename) is an explicit, separate, human step.
#
# Run on the Jetson host.

set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ENV_FILE="${SCRIPT_DIR}/container.env"
UNITS="astra-watchdog oak-watchdog ros2-camera-watcher"

NEW_IMAGE="${1:-}"
if [ -z "$NEW_IMAGE" ]; then
    echo "usage: swap_container.sh <NEW_IMAGE_TAG> [STAGING_NAME]" >&2
    exit 2
fi

# shellcheck disable=SC1090
[ -f "$ENV_FILE" ] && . "$ENV_FILE"
CUR_CONTAINER="${CONTAINER:-yahboom_ros2_humble}"
CUR_IMAGE="${IMAGE:-unknown}"
STAGING="${2:-${CUR_CONTAINER}_staging}"

say() { echo "[swap $(date +%H:%M:%S)] $*"; }

if ! docker image inspect "$NEW_IMAGE" >/dev/null 2>&1; then
    say "ERROR: image '$NEW_IMAGE' not found locally."; exit 1
fi
if docker ps -a --format '{{.Names}}' | grep -qx "$STAGING"; then
    say "ERROR: staging container '$STAGING' already exists. Remove it first:";
    say "       docker rm -f $STAGING"; exit 1
fi

say "current : $CUR_CONTAINER  (image $CUR_IMAGE)"
say "new     : $STAGING  (image $NEW_IMAGE)"

# ── 2. quiesce the watchdog stack ────────────────────────────────────
say "stopping watchdog/camera-watcher units..."
# shellcheck disable=SC2086
sudo systemctl stop $UNITS 2>/dev/null || true

# ── 3. stop (keep) the current container ─────────────────────────────
say "stopping current container $CUR_CONTAINER (kept for rollback)..."
docker stop "$CUR_CONTAINER" >/dev/null 2>&1 || true

rollback() {
    say "ROLLBACK: removing staging, restarting $CUR_CONTAINER..."
    docker rm -f "$STAGING" >/dev/null 2>&1 || true
    docker start "$CUR_CONTAINER" >/dev/null 2>&1 || true
    # shellcheck disable=SC2086
    sudo systemctl start $UNITS 2>/dev/null || true
    say "rolled back to $CUR_CONTAINER ($CUR_IMAGE). watchdogs restarted."
}

# ── 4. launch staging ────────────────────────────────────────────────
say "launching staging container..."
if ! bash "${SCRIPT_DIR}/run_container.sh" "$NEW_IMAGE" "$STAGING"; then
    say "run_container.sh failed."
    rollback
    exit 1
fi

# give the keepalive a moment; entrypoint sourcing is fast
sleep 5

# ── 5. smoke test ────────────────────────────────────────────────────
say "smoke testing $STAGING ..."
SMOKE=$(docker exec "$STAGING" bash -lc '
  set -e
  source /opt/ros/humble/setup.bash
  [ -f /opt/ros/humble_overlays/install/setup.bash ] && source /opt/ros/humble_overlays/install/setup.bash
  test "$ROS_DOMAIN_ID" = "100"
  wv=$(ros2 pkg prefix web_video_server)
  test "$wv" = "/opt/ros/humble"          # NOT the library_ws overlay
  ros2 pkg prefix astra_camera           >/dev/null
  ros2 pkg prefix ydlidar_ros2_driver    >/dev/null
  ros2 pkg prefix depthai_ros_driver     >/dev/null
  ros2 pkg prefix mecanum_drive_controller >/dev/null
  python3 -c "from Rosmaster_Lib import Rosmaster"
  python3 -c "import claude_agent_sdk"
  echo SMOKE_OK
' 2>&1) && echo "$SMOKE" | tail -3 || true

if ! echo "$SMOKE" | grep -q SMOKE_OK; then
    say "SMOKE TEST FAILED:"
    echo "$SMOKE" | sed 's/^/    /'
    rollback
    exit 1
fi
say "smoke test PASSED."

# ── 6a. commit the swap ──────────────────────────────────────────────
say "rewriting container.env → CONTAINER=$STAGING IMAGE=$NEW_IMAGE"
{
    grep -vE '^(CONTAINER|IMAGE)=' "$ENV_FILE" 2>/dev/null || true
    echo "CONTAINER=$STAGING"
    echo "IMAGE=$NEW_IMAGE"
} > "${ENV_FILE}.tmp" && mv "${ENV_FILE}.tmp" "$ENV_FILE"

# ros2-camera-watcher reacts to a `docker events ... start` for the
# managed container. The staging container ALREADY started (during this
# script) before container.env was repointed, so the watcher would miss
# that edge and cameras would never auto-launch. Bring them up
# explicitly now (start_ros2_nodes.sh reads the just-rewritten
# container.env → targets $STAGING).
say "launching cameras into $STAGING (start_ros2_nodes.sh)..."
/home/jetson/start_ros2_nodes.sh >/dev/null 2>&1 || \
    say "WARN: start_ros2_nodes.sh returned non-zero (check /tmp/*.log)"

say "restarting watchdog units (now managing $STAGING)..."
# shellcheck disable=SC2086
sudo systemctl start $UNITS 2>/dev/null || true

# Give Astra + OAK + web_video_server time to come up, then probe.
say "waiting 40s for camera stack, then probing :8090 ..."
sleep 40
ASTRA_BYTES=$(curl -s -o /dev/null -w '%{size_download}' --max-time 8 \
  'http://localhost:8090/stream?topic=/color/image_raw&type=mjpeg&quality=10&width=160&rate=1' 2>/dev/null || echo 0)
OAK_BYTES=$(curl -s -o /dev/null -w '%{size_download}' --max-time 8 \
  'http://localhost:8090/stream?topic=/rosmaster_oak/rgb/image_raw&type=mjpeg&quality=10&width=160&rate=1' 2>/dev/null || echo 0)
say "Astra stream: ${ASTRA_BYTES} bytes   OAK stream: ${OAK_BYTES} bytes"
if [ "${ASTRA_BYTES:-0}" -lt 1000 ] 2>/dev/null; then
    say "WARN: Astra stream looks dead (<1000 bytes). Camera may still be"
    say "      initializing, or v1.0-dev has a camera regression. Eyeball"
    say "      it before promoting; rollback path printed below."
fi

cat <<EOF

────────────────────────────────────────────────────────────────────
 SWAP OK. Now running: $STAGING  ($NEW_IMAGE)
 Old container kept stopped: $CUR_CONTAINER  ($CUR_IMAGE)

 Validate the robot, then EITHER:

   PROMOTE (make staging the canonical name):
     sudo systemctl stop $UNITS
     docker stop $STAGING
     docker rename $CUR_CONTAINER ${CUR_CONTAINER}_old_$(date +%Y%m%d)
     docker rename $STAGING $CUR_CONTAINER
     # set container.env CONTAINER=$CUR_CONTAINER IMAGE=$NEW_IMAGE
     bash ${SCRIPT_DIR}/run_container.sh $NEW_IMAGE $CUR_CONTAINER --replace
     sudo systemctl start $UNITS

   ROLLBACK (revert to the old container):
     sudo systemctl stop $UNITS
     docker rm -f $STAGING
     docker start $CUR_CONTAINER
     # set container.env CONTAINER=$CUR_CONTAINER IMAGE=$CUR_IMAGE
     sudo systemctl start $UNITS
────────────────────────────────────────────────────────────────────
EOF
