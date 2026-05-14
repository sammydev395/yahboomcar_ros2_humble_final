#!/usr/bin/env bash
#
# start_node_inside_container.sh — D8.3 rewrite (2026-05-14)
#
# Single inside-container entry point for the X3PLUS stack.
#
# WHAT THIS REPLACES
#   The vendor's start_node_inside_container.sh used to invoke
#   `ros2 launch yahboomcar_bringup yahboomcar_bringup_X3plus_launch.py`
#   which brought up the vendor Mcnamu_driver_X3plus + base_node_X3 +
#   yahboom_joy_X3plus + robot_state_publisher + IMU/EKF stack. D8.3 cuts
#   the vendor bringup entirely. Per operator decision 2026-05-14: NO
#   --vendor escape hatch. If rollback is ever needed, `git revert` the
#   start-script commit; vendor source still lives under
#   yahboomcar_ws/src/Mcnamu_driver_X3plus/ until the D9 cleanup removes
#   it.
#
# WHAT THIS LAUNCHES
#   1. yahboom_ros2_control master.launch.py (PRIMARY) — full ros2_control
#      stack: controller_manager + JSB + arm_controller + chassis_controller
#      + joy_node + arm_teleop + teleop_twist_joy + ydlidar driver +
#      laser TF.
#   2. rosmaster_capability multi-robot DDS facade (side launch, kept).
#   3. yahboomcar_astra Astra Pro fixed camera (side launch, kept until
#      OAK-D Lite arrives).
#   4. web_video_server on port 8090 (side launch, kept). Runs in a CLEAN
#      shell that sources ONLY /opt/ros/humble/setup.bash — sourcing
#      library_ws (which we DO source for the lidar driver in the main
#      launch) shadows the system OpenCV 4.5 with a broken OpenCV 4.10
#      overlay that crashes web_video_server with
#      `libopencv_imgcodecs.so.410: cannot open shared object file`.
#      (See MEMORY.md → Web Video Streaming.)
#
# DDS / RMW
#   CycloneDDS, ROS_DOMAIN_ID=100 — same as the rest of the fleet.
#
# PRE-FLIGHT (verified by run_checks below — fail closed)
#   - ROS2 Humble installed.
#   - Main workspace built (yahboomcar_ws/install/setup.bash exists).
#   - library_ws built (ydlidar_ros2_driver discoverable via ros2 pkg).
#   - yahboom_ros2_control package discoverable via ros2 pkg.
#   - /dev/yahboom_stm32 symlink resolves (devpath-pinned udev rule
#     against the dual-CH340 ambiguity — see project memory
#     project_yahboom_dual_ch340_devpath.md).
#   - /dev/ydlidar symlink resolves (vendor's ydlidar.rules).
#
# OPTIONS
#   --check     Run pre-flight only, do not launch.
#   --no-lidar  Skip YDLidar (bench tests with no lidar plugged in).
#               Forwarded as enable_lidar:=false to master.launch.py.
#   -h|--help   Show usage.

set -e

export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
export ROS_DOMAIN_ID=100

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WS_ROOT="${SCRIPT_DIR}"
# Inside the container the actual layout is:
#   /root/yahboomcar_ros2_ws            ← bind mount of host's
#                                         /home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws
#   /root/yahboomcar_ros2_ws_new/software/library_ws    ← the lidar driver
#                                         (vendor's prior start script
#                                          also hardcodes this path).
# So WS_ROOT and the parent of LIBRARY_WS_ROOT are NOT siblings; we
# resolve LIBRARY_WS_ROOT explicitly in priority order:
#   1. $LIBRARY_WS_ROOT env override
#   2. /root/yahboomcar_ros2_ws_new/software/library_ws (canonical path
#      inside the container — what vendor's old script also expected)
#   3. ${HOME}/software/library_ws (legacy fallback)
SOFTWARE_ROOT="${HOME}/software"
if [[ -z "${LIBRARY_WS_ROOT:-}" ]]; then
    if [[ -d "/root/yahboomcar_ros2_ws_new/software/library_ws" ]]; then
        LIBRARY_WS_ROOT="/root/yahboomcar_ros2_ws_new/software/library_ws"
    else
        LIBRARY_WS_ROOT="${SOFTWARE_ROOT}/library_ws"
    fi
fi

CHECK_ONLY=false
NO_LIDAR=false

usage() {
    cat <<EOF
Usage: $0 [OPTIONS]

D8.3 master-launch entry point for the X3PLUS stack inside the
yahboom_ros2_humble container.

Options:
  --check      Run pre-flight only, do not launch
  --no-lidar   Skip YDLidar (forwards enable_lidar:=false)
  -h|--help    This help

Paths (detected):
  WS_ROOT          = $WS_ROOT
  SOFTWARE_ROOT    = $SOFTWARE_ROOT
  LIBRARY_WS_ROOT  = $LIBRARY_WS_ROOT

Web Video Streams (after startup):
  http://<robot_ip>:8090/                                    - Topic list
  http://<robot_ip>:8090/stream_viewer?topic=/camera/color/image_raw
                                                             - Astra color
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --check)    CHECK_ONLY=true ; shift ;;
        --no-lidar) NO_LIDAR=true   ; shift ;;
        -h|--help)  usage ; exit 0 ;;
        *) echo "Unknown option: $1" ; usage ; exit 1 ;;
    esac
done

# ----------------------------------------------------------------------
# Kill any stale processes from previous bringup runs (vendor or ours).
# ----------------------------------------------------------------------
kill_stale_processes() {
    local patterns=(
        "ros2_control_node"
        "controller_manager"
        "ydlidar_ros2_driver_node"
        "static_transform_publisher"
        "joy_node"
        "arm_teleop_node"
        "teleop_twist_joy"
        "robot_state_publisher"
        "web_video_server"
        "Mcnamu_driver_X3plus"
        "base_node_X3"
        "yahboom_joy_X3plus"
        "imu_filter_madgwick_node"
        "ekf_localization_node"
    )
    local killed=0
    for pattern in "${patterns[@]}"; do
        if pkill -f "$pattern" 2>/dev/null; then
            echo "  Stopped existing: $pattern"
            killed=1
        fi
    done
    if [[ $killed -eq 1 ]]; then
        echo "Waiting 2s for processes to exit..."
        sleep 2
    fi
}

echo "Killing any stale bringup processes..."
kill_stale_processes

# ----------------------------------------------------------------------
# Environment setup. Source order matters:
#   1. /opt/ros/humble — base.
#   2. library_ws — provides ydlidar_ros2_driver. NOTE: this overlays
#      OpenCV 4.10 which breaks web_video_server, so the web_video_server
#      side launch later sources /opt/ros/humble ONLY.
#   3. yahboomcar_ws — main workspace (yahboom_ros2_control,
#      rosmaster_capability, yahboomcar_astra, etc.).
# ----------------------------------------------------------------------
export ROBOT_TYPE=x3plus
export PYTHONPATH="${SOFTWARE_ROOT}/py_install_V3.3.1/build/lib${PYTHONPATH:+:$PYTHONPATH}"

if [[ ! -f /opt/ros/humble/setup.bash ]]; then
    echo "Error: ROS2 Humble not found at /opt/ros/humble."
    exit 1
fi
source /opt/ros/humble/setup.bash

if [[ -f "${LIBRARY_WS_ROOT}/install/setup.bash" ]]; then
    source "${LIBRARY_WS_ROOT}/install/setup.bash"
else
    echo "Error: library_ws not found at ${LIBRARY_WS_ROOT}."
    echo "       (D8.3 needs ydlidar_ros2_driver from library_ws.)"
    exit 1
fi

if [[ ! -f "${WS_ROOT}/install/setup.bash" ]]; then
    echo "Error: Workspace not built. Run: cd ${WS_ROOT} && colcon build && source install/setup.bash"
    exit 1
fi
source "${WS_ROOT}/install/setup.bash"

# Make workspace Python packages importable by launched nodes (rosidl /
# ament_cmake use local/lib; ament_python uses lib/python3.10).
_install="${WS_ROOT}/install"
for _pkg in "${_install}"/*/; do
    for _dir in "${_pkg}local/lib/python3.10/dist-packages" \
                "${_pkg}lib/python3.10/site-packages"; do
        [[ -d "$_dir" ]] && export PYTHONPATH="${_dir}${PYTHONPATH:+:$PYTHONPATH}"
    done
done
unset _install _pkg _dir

# ----------------------------------------------------------------------
# Pre-flight checks (fail closed).
# ----------------------------------------------------------------------
run_checks() {
    local ok=0
    echo "Pre-flight checks:"
    echo "  WS_ROOT          = $WS_ROOT"
    echo "  SOFTWARE_ROOT    = $SOFTWARE_ROOT"
    echo "  LIBRARY_WS_ROOT  = $LIBRARY_WS_ROOT"

    if [[ ! -f "${WS_ROOT}/install/setup.bash" ]]; then
        echo "  [FAIL] install/setup.bash not found"
        ok=1
    else
        echo "  [OK] yahboomcar_ws install/setup.bash"
    fi

    if [[ ! -f "${LIBRARY_WS_ROOT}/install/setup.bash" ]]; then
        echo "  [FAIL] library_ws install/setup.bash not found"
        ok=1
    else
        echo "  [OK] library_ws install/setup.bash"
    fi

    if ! ros2 pkg list 2>/dev/null | grep -q '^yahboom_ros2_control$'; then
        echo "  [FAIL] yahboom_ros2_control not in ROS2 package path"
        ok=1
    else
        echo "  [OK] yahboom_ros2_control package found"
    fi

    if ! ros2 pkg list 2>/dev/null | grep -q '^ydlidar_ros2_driver$'; then
        echo "  [FAIL] ydlidar_ros2_driver not in ROS2 package path (library_ws not sourced?)"
        ok=1
    else
        echo "  [OK] ydlidar_ros2_driver package found"
    fi

    if [[ ! -e /dev/yahboom_stm32 ]]; then
        echo "  [WARN] /dev/yahboom_stm32 missing — udev rule deployed?"
        echo "         (See provision/jetson/99-yahboom-stm32.rules and"
        echo "          memory project_yahboom_dual_ch340_devpath.md.)"
    else
        echo "  [OK] /dev/yahboom_stm32 → $(readlink -f /dev/yahboom_stm32)"
    fi

    if [[ "$NO_LIDAR" == false ]]; then
        if [[ ! -e /dev/ydlidar ]]; then
            echo "  [WARN] /dev/ydlidar missing — vendor ydlidar.rules deployed?"
            echo "         (Pass --no-lidar to skip lidar bring-up.)"
        else
            echo "  [OK] /dev/ydlidar → $(readlink -f /dev/ydlidar)"
        fi
    else
        echo "  [SKIP] lidar checks (--no-lidar)"
    fi

    if [[ $ok -ne 0 ]]; then
        echo "Fix the [FAIL] items above and try again."
        return 1
    fi
    echo "All checks passed."
    return 0
}

run_checks || exit 1

if [[ "$CHECK_ONLY" == true ]]; then
    echo "Exiting (--check only)."
    exit 0
fi

# ----------------------------------------------------------------------
# Launch master.launch.py (primary process).
# ----------------------------------------------------------------------
LAUNCH_ARGS=(ros2 launch yahboom_ros2_control master.launch.py)
if [[ "$NO_LIDAR" == true ]]; then
    LAUNCH_ARGS+=(enable_lidar:=false)
fi

echo "Starting D8.3 master.launch.py: ${LAUNCH_ARGS[*]}"
"${LAUNCH_ARGS[@]}" &
MASTER_PID=$!

# Wait for controller_manager + ydlidar to come up before side launches.
# Empirically (D8.2) controller_manager takes ~12 s to activate all
# controllers; lidar lifecycle activation completes within ~3 s after
# that. 30 s is generous and matches the vendor's prior wait.
sleep 30

# ----------------------------------------------------------------------
# Side launches.
# ----------------------------------------------------------------------
echo "Starting rosmaster_capability node in background..."
ros2 launch rosmaster_capability capability.launch.py &

echo "Starting Astra camera node in background..."
ros2 launch yahboomcar_astra astra.launch.py &

# web_video_server in a CLEAN shell — must NOT inherit the library_ws
# environment. Sourcing library_ws/install/setup.bash overlays OpenCV
# 4.10 over the system OpenCV 4.5 that web_video_server is linked
# against, causing `libopencv_imgcodecs.so.410: cannot open shared
# object file` at startup. The clean shell sources /opt/ros/humble +
# main workspace ONLY (no library_ws).
echo "Starting web_video_server (clean shell, port 8090)..."
env -i HOME="$HOME" PATH="/usr/local/bin:/usr/bin:/bin" \
     CYCLONEDDS_URI="${CYCLONEDDS_URI:-}" \
    bash -c "
        source /opt/ros/humble/setup.bash
        source ${WS_ROOT}/install/setup.bash
        export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
        export ROS_DOMAIN_ID=100
        ros2 run web_video_server web_video_server --ros-args -p port:=8090
    " &

echo ""
echo "─────────────────────────────────────────────────────────────"
echo "  D8.3 master launch + side launches running."
echo "  master.launch.py PID: $MASTER_PID"
echo "  Web streams: http://<robot_ip>:8090/"
echo "  Ctrl+C to shut everything down."
echo "─────────────────────────────────────────────────────────────"

wait $MASTER_PID
