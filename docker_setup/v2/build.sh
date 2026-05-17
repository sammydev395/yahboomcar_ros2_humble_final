#!/usr/bin/env bash
# build.sh — build the yahboom_ros2_humble container image.
#
# Usage:
#   ./build.sh [dev|prod] [version]
#
# Examples:
#   ./build.sh                  →  yahboom_ros2_humble:v1.0-dev   (default)
#   ./build.sh dev              →  yahboom_ros2_humble:v1.0-dev
#   ./build.sh prod v1.0        →  yahboom_ros2_humble:v1.0-prod
#   ./build.sh prod v1.1        →  yahboom_ros2_humble:v1.1-prod
#
# Must be run ON THE JETSON (jetsonnanodev) — Dockerfile installs arm64
# packages. Cross-builds from amd64 hosts are NOT supported here.
#
# Always runs prepare_context.sh first (idempotent — assembles the
# base-stage build context: Rosmaster_Lib + ros2_astra_camera +
# ydlidar_driver + YDLidar-SDK + udev_rules + cyclonedds_config.xml).
# For prod it ALSO rsyncs the Jetson ros2_ws/src into
# ros2_ws_staging/src/ so the prod stage can COPY + colcon-build it.
# dev skips that staging.
#
# base is VARIANT-independent: dev and prod share its layer cache, so
# a prod build right after a dev build = base (cached) + COPY src +
# colcon (minutes), not a full dependency recompile.

set -euo pipefail

VARIANT="${1:-dev}"
VERSION="${2:-v1.0}"

case "${VARIANT}" in
    dev|prod) ;;
    *) echo "ERROR: variant must be 'dev' or 'prod' (got '${VARIANT}')" >&2; exit 2 ;;
esac

TAG="yahboom_ros2_humble:${VERSION}-${VARIANT}"
CONTEXT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROS2_WS_HOST="${ROS2_WS_HOST:-/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws}"
STAGING_DIR="${CONTEXT_DIR}/ros2_ws_staging"

echo "=== build.sh ==="
echo "variant : ${VARIANT}"
echo "version : ${VERSION}"
echo "tag     : ${TAG}"
echo "context : ${CONTEXT_DIR}"

# --- always: assemble the base-stage build context -----------------------
echo "=== prepare_context.sh ==="
bash "${CONTEXT_DIR}/prepare_context.sh" "${CONTEXT_DIR}" >/dev/null
echo "context assembled (Rosmaster_Lib + astra + ydlidar + ydlidar_sdk + udev + cyclonedds)"

# --- prod-only: stage ros2_ws/src into the build context -----------------
if [ "${VARIANT}" = "prod" ]; then
    if [ ! -d "${ROS2_WS_HOST}/src" ]; then
        echo "ERROR: prod build needs ${ROS2_WS_HOST}/src (override via ROS2_WS_HOST=)" >&2
        exit 3
    fi
    echo "staging ${ROS2_WS_HOST}/src/  →  ${STAGING_DIR}/src/"
    rm -rf "${STAGING_DIR}"
    mkdir -p "${STAGING_DIR}/src"
    # Stage only src/ — colcon ignores the rest of the host workspace
    # tree (backup files, core dumps, IDE state, vendor scripts, prior
    # build/install/log) which we do NOT want in the image.
    rsync -a --delete \
        --exclude='build/' --exclude='install/' --exclude='log/' \
        --exclude='.git/' --exclude='.cursor/' --exclude='.specstory/' \
        --exclude='.vscode/' --exclude='*.bak' --exclude='*.bak-*' \
        --exclude='*.py~' --exclude='__pycache__/' \
        "${ROS2_WS_HOST}/src/" "${STAGING_DIR}/src/"
else
    # dev: ensure no stale staging dir lingers in the context. The prod
    # COPY would fail without it, but the dev target never reaches the
    # prod stage.
    rm -rf "${STAGING_DIR}"
fi

# --- build ---------------------------------------------------------------
# NOTE: no --build-arg VARIANT. The Dockerfile has no ARG VARIANT;
# --target alone selects the leaf. This is what guarantees the base
# layer cache is shared between dev and prod.
echo "=== docker build --target ${VARIANT} ==="
docker build \
    --target "${VARIANT}" \
    -t "${TAG}" \
    -f "${CONTEXT_DIR}/Dockerfile" \
    "${CONTEXT_DIR}"

echo
echo "=== built ${TAG} ==="
docker images "${TAG}" --format '  {{.Repository}}:{{.Tag}}  {{.Size}}  {{.ID}}'

# --- cleanup -------------------------------------------------------------
if [ "${VARIANT}" = "prod" ]; then
    echo "removing staging dir ${STAGING_DIR}"
    rm -rf "${STAGING_DIR}"
fi
