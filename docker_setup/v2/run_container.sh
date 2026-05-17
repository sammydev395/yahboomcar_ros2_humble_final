#!/usr/bin/env bash
# run_container.sh — launch a yahboom ROS2 container with the full,
# correct set of mounts / devices / networking. Scriptable replacement
# for the old interactive run_docker_ros2.sh.
#
#   run_container.sh [IMAGE_TAG] [CONTAINER_NAME] [--replace]
#
#   IMAGE_TAG       default: $IMAGE      from container.env (fallback yahboom_ros2_humble:latest)
#   CONTAINER_NAME  default: $CONTAINER  from container.env (fallback yahboom_ros2_humble)
#   --replace       if a container of that name exists, stop+rm it first
#                   (without this flag the script REFUSES to clobber an
#                    existing container — measure twice, cut once)
#
# Does NOT exec into the container and does NOT prompt — it `docker run -d`
# and returns. Device list is pinned to what the running production
# container actually uses (captured 2026-05-15 via docker inspect).
#
# NOTE: this launches the keepalive container only. Cameras + ROS launch
# come up via ros2-camera-watcher.service → start_ros2_nodes.sh, exactly
# as today. swap_container.sh orchestrates a safe A/B switch.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ENV_FILE="${SCRIPT_DIR}/container.env"
# shellcheck disable=SC1090
[ -f "$ENV_FILE" ] && . "$ENV_FILE"

IMAGE_TAG="${1:-${IMAGE:-yahboom_ros2_humble:latest}}"
CONTAINER_NAME="${2:-${CONTAINER:-yahboom_ros2_humble}}"
REPLACE=false
for a in "$@"; do [ "$a" = "--replace" ] && REPLACE=true; done

if docker ps -a --format '{{.Names}}' | grep -qx "${CONTAINER_NAME}"; then
    if [ "$REPLACE" = true ]; then
        echo "Removing existing container ${CONTAINER_NAME}..."
        docker stop "${CONTAINER_NAME}" >/dev/null 2>&1 || true
        docker rm   "${CONTAINER_NAME}" >/dev/null 2>&1 || true
    else
        echo "ERROR: container '${CONTAINER_NAME}' already exists." >&2
        echo "       Pass --replace to stop+rm it, or pick another name." >&2
        exit 1
    fi
fi

if ! docker image inspect "${IMAGE_TAG}" >/dev/null 2>&1; then
    echo "ERROR: image '${IMAGE_TAG}' not found locally." >&2
    echo "       docker images | grep yahboom" >&2
    exit 1
fi

# X11 (best-effort; harmless headless)
xhost + >/dev/null 2>&1 || true

WS_HOST=/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws
WS_NEW_HOST=/home/jetson/yahboomcar_ros2_ws_new

echo "Launching ${CONTAINER_NAME} from ${IMAGE_TAG}..."
# Device list intentionally uses bind mounts of /dev/bus/usb plus
# explicit --device for the stable serial/video nodes. The udev-created
# symlinks (/dev/myserial, /dev/yahboom_stm32, /dev/ydlidar) live on the
# host and are visible because /dev/bus/usb + the ttyUSB devices are
# passed through.
docker run -d \
    --name "${CONTAINER_NAME}" \
    --privileged \
    --gpus all \
    --runtime=nvidia \
    --net=host \
    --restart unless-stopped \
    --env "DISPLAY=:0" \
    --env "QT_X11_NO_MITSHM=1" \
    --env "NO_AT_BRIDGE=1" \
    --env "ROS_DOMAIN_ID=${ROS_DOMAIN_ID:-100}" \
    --env "RMW_IMPLEMENTATION=rmw_cyclonedds_cpp" \
    -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
    -v /home/jetson/.Xauthority:/root/.Xauthority:rw \
    -v "${WS_HOST}:/root/yahboomcar_ros2_ws:rw" \
    -v "${WS_NEW_HOST}:/root/yahboomcar_ros2_ws_new:rw" \
    -v /home/jetson/temp:/root/temp:rw \
    -v /home/jetson/ultralytics:/root/ultralytics:rw \
    -v /dev/bus/usb:/dev/bus/usb:rw \
    --device=/dev/ttyUSB0:/dev/ttyUSB0 \
    --device=/dev/ttyUSB1:/dev/ttyUSB1 \
    --device=/dev/ttyUSB2:/dev/ttyUSB2 \
    --device=/dev/input \
    --device=/dev/video0:/dev/video0 \
    --device=/dev/video1:/dev/video1 \
    --device=/dev/video2:/dev/video2 \
    --device=/dev/video3:/dev/video3 \
    --device-cgroup-rule='c 189:* rmw' \
    -p 9090:9090 -p 8888:8888 -p 6000:6000 \
    "${IMAGE_TAG}" \
    /ros_entrypoint.sh tail -f /dev/null

echo "Container ${CONTAINER_NAME} started from ${IMAGE_TAG}."
docker ps --filter "name=${CONTAINER_NAME}" --format '  {{.Names}}  {{.Status}}  ({{.Image}})'
