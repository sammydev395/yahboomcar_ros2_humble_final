#!/usr/bin/env bash
# prepare_context.sh — populate the Docker build context with source trees
# that the Dockerfile expects to COPY from. Run this on the Jetson before
# invoking `docker build`.
#
# Source trees copied:
#   - Rosmaster_Lib    (vendor Python lib, installed as egg by Dockerfile step 6)
#   - astra_camera     (vendor source — no apt pkg exists)
#   - ydlidar_driver   (vendor source — needed for /scan in master.launch.py)
#
# Run from the directory containing the Dockerfile, OR pass the docker
# build-context directory as the only argument.
#
# Idempotent: removes any prior context/ subtree before populating.

set -euo pipefail

CTX_DIR="${1:-.}"
JETSON_SW_ROOT=/home/jetson/yahboomcar_ros2_ws_new/software
JETSON_LIB_WS_SRC="${JETSON_SW_ROOT}/library_ws/src"

if [ ! -d "${JETSON_SW_ROOT}" ]; then
    echo "ERROR: expected ${JETSON_SW_ROOT} on the Jetson. Run this on jetsonnanodev." >&2
    exit 1
fi

cd "${CTX_DIR}"
rm -rf context
mkdir -p context

# Rosmaster_Lib: the vendor distributes this as a proper packaged
# source under software/py_install_V<ver>/ (has setup.py +
# Rosmaster_Lib/ package + builds the 3.3.1 egg the running container
# uses). A bare Rosmaster_Lib/ module dir also exists elsewhere but has
# NO setup.py — copying that makes the Dockerfile's
# `if [ -f setup.py ]` guard silently no-op (the 2026-05-16 bug). So
# prefer the py_install dir with setup.py.
echo "Copying Rosmaster_Lib (prefer py_install_V*/ with setup.py)..."
ROSM_SRC=""
# Highest version py_install dir that actually has setup.py.
for d in $(ls -d "${JETSON_SW_ROOT}"/py_install_V* 2>/dev/null | sort -rV); do
    if [ -f "$d/setup.py" ]; then ROSM_SRC="$d"; break; fi
done
# Fallbacks: explicit Rosmaster_Lib dir with setup.py, then any dir.
[ -z "$ROSM_SRC" ] && [ -f "${JETSON_SW_ROOT}/Rosmaster_Lib/setup.py" ] && \
    ROSM_SRC="${JETSON_SW_ROOT}/Rosmaster_Lib"
[ -z "$ROSM_SRC" ] && ROSM_SRC=$(dirname "$(find "${JETSON_SW_ROOT}" -maxdepth 3 -name setup.py -path '*[Rr]osmaster*' 2>/dev/null | head -1)" 2>/dev/null)
[ -z "$ROSM_SRC" ] && ROSM_SRC=$(find "${JETSON_SW_ROOT}" -maxdepth 3 -type d -name 'Rosmaster_Lib' 2>/dev/null | head -1)

if [ -n "$ROSM_SRC" ] && [ -d "$ROSM_SRC" ]; then
    cp -r "$ROSM_SRC" context/Rosmaster_Lib
    echo "  source: $ROSM_SRC"
    # Strip build cruft + editor backups so the in-container
    # setup.py install is clean and the context stays small.
    ( cd context/Rosmaster_Lib && \
      rm -rf build dist *.egg-info __pycache__ 2>/dev/null
      find . -name '*.py~' -delete 2>/dev/null || true )
    if [ ! -f context/Rosmaster_Lib/setup.py ]; then
        echo "  WARN: copied Rosmaster_Lib has NO setup.py — Dockerfile will" >&2
        echo "        fall back to package-dir copy (still works, see Dockerfile)" >&2
    fi
else
    echo "WARN: Rosmaster_Lib source not found under ${JETSON_SW_ROOT}; skipping" >&2
fi

# astra_camera lives one level deeper, under ros2_astra_camera/, and
# ships as TWO packages: astra_camera + astra_camera_msgs. It bundles
# its own OpenNI2 redistributables (openni2_redist/arm64/) so no
# separate OpenNI SDK is needed — only libuvc (apt) at build time.
# Copy the whole ros2_astra_camera/ dir; colcon discovers both pkgs.
echo "Copying ros2_astra_camera (astra_camera + astra_camera_msgs)..."
if [ -d "${JETSON_LIB_WS_SRC}/ros2_astra_camera" ]; then
    cp -r "${JETSON_LIB_WS_SRC}/ros2_astra_camera" context/astra_camera
elif [ -d "${JETSON_LIB_WS_SRC}/astra_camera" ]; then
    cp -r "${JETSON_LIB_WS_SRC}/astra_camera" context/astra_camera
else
    ASTRA_SRC=$(find "${JETSON_SW_ROOT}" -maxdepth 4 -type d -name 'ros2_astra_camera' 2>/dev/null | head -1)
    if [ -n "$ASTRA_SRC" ]; then
        cp -r "$ASTRA_SRC" context/astra_camera
    else
        echo "WARN: astra_camera source not found; skipping" >&2
    fi
fi

# YDLidar-SDK — standalone CMake C++ lib that ydlidar_ros2_driver
# find_package()s as `ydlidar_sdk`. Vendor `make install`'d it
# system-wide; the Dockerfile rebuilds + installs it to /usr/local
# BEFORE the overlay colcon build so the ROS driver can find it.
echo "Copying YDLidar-SDK..."
if [ -d "${JETSON_SW_ROOT}/YDLidar-SDK" ]; then
    cp -r "${JETSON_SW_ROOT}/YDLidar-SDK" context/ydlidar_sdk
else
    SDK_SRC=$(find "${JETSON_SW_ROOT}" -maxdepth 3 -type d -iname 'YDLidar-SDK*' 2>/dev/null | head -1)
    if [ -n "$SDK_SRC" ]; then
        cp -r "$SDK_SRC" context/ydlidar_sdk
    else
        echo "WARN: YDLidar-SDK source not found; ydlidar driver build WILL fail" >&2
    fi
fi
# Strip the vendor's in-source build cruft so the in-container cmake
# doesn't trip over stale absolute Jetson paths.
if [ -d context/ydlidar_sdk ]; then
    ( cd context/ydlidar_sdk && \
      rm -rf build CMakeFiles CMakeCache.txt cmake_install.cmake \
             Makefile *.a *.so ydlidar_sdkConfig*.cmake \
             ydlidar_sdkConfigVersion.cmake 2>/dev/null || true )
fi

echo "Copying ydlidar_ros2_driver..."
if [ -d "${JETSON_LIB_WS_SRC}/ydlidar_ros2_driver" ]; then
    cp -r "${JETSON_LIB_WS_SRC}/ydlidar_ros2_driver" context/ydlidar_driver
else
    # Some vendor layouts include the SDK as a sibling — copy both if found.
    YDLIDAR=$(find "${JETSON_LIB_WS_SRC}" -maxdepth 2 -type d -name 'ydlidar_ros2_driver' 2>/dev/null | head -1)
    if [ -n "$YDLIDAR" ]; then
        cp -r "$YDLIDAR" context/ydlidar_driver
    else
        echo "WARN: ydlidar_ros2_driver source not found; skipping" >&2
    fi
fi

# ── udev rules ────────────────────────────────────────────────────────
# The Dockerfile does `COPY udev_rules/ /opt/udev_rules.d/` so the image
# is self-describing about the device topology it expects. These are
# baked in for documentation; the HOST-side /etc/udev/rules.d/ remains
# the actual source of truth at runtime (the container's /dev is
# bind-mounted from the host). Gather the yahboom-relevant rules from
# the Jetson's live /etc/udev/rules.d/.
echo "Assembling udev_rules/ ..."
rm -rf udev_rules
mkdir -p udev_rules
for r in 99-astra.rules 99-yahboom-joy.rules 99-yahboom-stm32.rules \
         serial.rules ydlidar.rules ydlidar-2303.rules ydlidar-V2.rules; do
    if [ -f "/etc/udev/rules.d/${r}" ]; then
        cp "/etc/udev/rules.d/${r}" "udev_rules/${r}"
    fi
done
# Guarantee the directory is non-empty so the COPY never fails even if
# none of the named rules exist (fresh host, rules not yet deployed).
[ -z "$(ls -A udev_rules 2>/dev/null)" ] && \
    echo "# udev rules are host-managed; see provision/jetson/*.rules" \
    > udev_rules/README

# Make sure incidental build outputs in copied trees don't bloat the context.
find context -type d \( -name build -o -name install -o -name log -o -name __pycache__ \) -prune -exec rm -rf {} + 2>/dev/null || true

echo
echo "Context populated:"
du -sh context/* 2>/dev/null || echo "  (empty)"
echo
echo "Now run, from this directory:"
echo "  docker build -t yahboom_ros2_humble:v1.0-dev ."
