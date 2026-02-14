#!/usr/bin/env bash
#
# start_node_inside_container.sh
# Start the main X3Plus ROS2 nodes inside the container.
# Based on TEST_PLAN.md and ROS2_Package_Docs (yahboomcar_bringup, yahboomcar_ctrl).
#
# Key nodes started by the bringup launch:
#   - /driver_node   (chassis + 6-DOF arm)
#   - /base_node     (odometry)
#   - /joy_node      (joystick hardware)
#   - /yahboom_joy   (joystick controller → /cmd_vel, /TargetAngle)
#   - /robot_state_publisher, imu_filter, ekf_localization
#

# CycloneDDS RMW configuration
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
export ROS_DOMAIN_ID=100

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# Workspace root: same directory as this script (e.g. .../yahboomcar_ros2_ws)
WS_ROOT="${SCRIPT_DIR}"
# Software deps (Rosmaster_Lib, etc.): prefer $HOME/software, fallback to sibling of workspace
SOFTWARE_ROOT="${HOME}/software"
[[ -d "${SCRIPT_DIR}/../software" && ! -d "$SOFTWARE_ROOT" ]] && SOFTWARE_ROOT="$(cd "${SCRIPT_DIR}/../software" && pwd)"

USE_VOICE=false
USE_RVIZ=false
CHECK_ONLY=false

usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Start main X3Plus bringup (chassis + arm + joystick + state publisher + EKF)."
    echo ""
    echo "Options:"
    echo "  --voice      Use voice-control bringup (driver + voice_control node)"
    echo "  --rviz       Start RViz with the bringup"
    echo "  --check      Only run pre-flight checks, do not launch"
    echo "  -h, --help   Show this help"
    echo ""
    echo "Paths (detected):"
    echo "  WS_ROOT=$WS_ROOT"
    echo "  SOFTWARE_ROOT=$SOFTWARE_ROOT"
    echo ""
    echo "Examples:"
    echo "  $0              # Standard bringup (joystick control)"
    echo "  $0 --check      # Verify environment only"
    echo "  $0 --voice      # Bringup with voice control"
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --voice)
            USE_VOICE=true
            shift
            ;;
        --rviz)
            USE_RVIZ=true
            shift
            ;;
        --check)
            CHECK_ONLY=true
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
done

# -----------------------------------------------------------------------------
# Kill any existing bringup nodes (by executable name) to avoid port/device conflicts
# Node names from yahboomcar_bringup_X3plus_launch.py and voice launch
# -----------------------------------------------------------------------------
kill_bringup_processes() {
    local patterns=(
        "Mcnamu_driver_X3plus"
        "base_node_X3"
        "joy_node"
        "yahboom_joy_X3plus"
        "robot_state_publisher"
        "imu_filter_madgwick_node"
        "ekf_localization_node"
        "rviz2"
        "Voice_Ctrl_Unified_X3plus"
        "joint_state_publisher"
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

echo "Checking for existing bringup nodes..."
kill_bringup_processes

# -----------------------------------------------------------------------------
# Environment (match TEST_PLAN.md prerequisites)
# -----------------------------------------------------------------------------
export ROBOT_TYPE=x3plus

# Driver node needs Rosmaster_Lib; voice bringup also needs Speech_Lib
if [[ "$USE_VOICE" == true ]]; then
    export PYTHONPATH="${SOFTWARE_ROOT}/py_install_V3.3.1/build/lib:${SOFTWARE_ROOT}/py_install_V0.0.1/py_install/build/lib${PYTHONPATH:+:$PYTHONPATH}"
else
    export PYTHONPATH="${SOFTWARE_ROOT}/py_install_V3.3.1/build/lib${PYTHONPATH:+:$PYTHONPATH}"
fi

# Source ROS2 and workspace (install must exist)
if [[ ! -f /opt/ros/humble/setup.bash ]]; then
    echo "Error: ROS2 Humble not found at /opt/ros/humble. Install or mount it."
    exit 1
fi
source /opt/ros/humble/setup.bash

if [[ ! -f "${WS_ROOT}/install/setup.bash" ]]; then
    echo "Error: Workspace not built. Run from workspace: colcon build && source install/setup.bash"
    exit 1
fi
source "${WS_ROOT}/install/setup.bash"

# Ensure workspace Python packages (e.g. yahboomcar_msgs) are on PYTHONPATH for launched nodes.
# rosidl/ament_cmake use local/lib/python3.10/dist-packages; ament_python use lib/python3.10/site-packages.
_install="${WS_ROOT}/install"
for _pkg in "${_install}"/*/; do
  for _dir in "${_pkg}local/lib/python3.10/dist-packages" "${_pkg}lib/python3.10/site-packages"; do
    if [[ -d "$_dir" ]]; then
      export PYTHONPATH="${_dir}${PYTHONPATH:+:$PYTHONPATH}"
    fi
  done
done
unset _install _pkg _dir

# -----------------------------------------------------------------------------
# Pre-flight checks
# -----------------------------------------------------------------------------
run_checks() {
    local ok=0
    echo "Pre-flight checks:"
    echo "  WS_ROOT=$WS_ROOT"
    echo "  SOFTWARE_ROOT=$SOFTWARE_ROOT"

    if [[ ! -d "$WS_ROOT" ]]; then
        echo "  [FAIL] WS_ROOT is not a directory: $WS_ROOT"
        ok=1
    else
        echo "  [OK] WS_ROOT exists"
    fi

    if [[ ! -f "${WS_ROOT}/install/setup.bash" ]]; then
        echo "  [FAIL] install/setup.bash not found. Run: cd $WS_ROOT && colcon build --symlink-install && source install/setup.bash"
        ok=1
    else
        echo "  [OK] install/setup.bash exists"
    fi

    if [[ ! -d "${SOFTWARE_ROOT}/py_install_V3.3.1/build/lib" ]]; then
        echo "  [WARN] Rosmaster_Lib not found at ${SOFTWARE_ROOT}/py_install_V3.3.1/build/lib (driver may fail)"
    else
        echo "  [OK] Rosmaster_Lib path exists"
    fi

    if ! python3 -c "import yahboomcar_msgs.msg" 2>/dev/null; then
        echo "  [FAIL] yahboomcar_msgs not importable. Run: cd $WS_ROOT && source install/setup.bash && colcon build --packages-select yahboomcar_msgs --symlink-install"
        ok=1
    else
        echo "  [OK] yahboomcar_msgs importable"
    fi

    if ! ros2 pkg list 2>/dev/null | grep -q yahboomcar_bringup; then
        echo "  [FAIL] yahboomcar_bringup not in ROS2 package path. Source workspace: source ${WS_ROOT}/install/setup.bash"
        ok=1
    else
        echo "  [OK] yahboomcar_bringup package found"
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

# -----------------------------------------------------------------------------
# Launch main bringup
# -----------------------------------------------------------------------------
if [[ "$USE_VOICE" == true ]]; then
    echo "Starting X3Plus bringup with VOICE CONTROL..."
    LAUNCH_ARGS=(ros2 launch yahboomcar_bringup yahboomcar_bringup_X3plus_voice_launch.py)
else
    echo "Starting X3Plus bringup (joystick control)..."
    LAUNCH_ARGS=(ros2 launch yahboomcar_bringup yahboomcar_bringup_X3plus_launch.py)
fi

if [[ "$USE_RVIZ" == true ]]; then
    LAUNCH_ARGS+=(use_rviz:=true)
fi

# Launch main bringup in background
"${LAUNCH_ARGS[@]}" &
BRINGUP_PID=$!

# Wait for nodes to start before launching capability facade
sleep 30

# --- Multi-robot capability facade node ---
# Advertises Rosmaster capabilities over DDS for agent coordination
echo "Starting rosmaster_capability node in background..."
ros2 launch rosmaster_capability capability.launch.py &

wait $BRINGUP_PID
