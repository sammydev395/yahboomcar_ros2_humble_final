#!/bin/bash
# ROS2 Humble Container Entrypoint
# This will be updated based on extracted Foxy container startup scripts

set -e

# Source ROS2 Humble
source /opt/ros/humble/setup.bash

# Source workspace if it exists and is built
if [ -f /root/yahboomcar_ros2_ws/install/setup.bash ]; then
    echo "Sourcing ROS2 workspace..."
    source /root/yahboomcar_ros2_ws/install/setup.bash
fi

# Setup device symlinks (similar to ROS1 container)
if [ -e /dev/ttyUSB1 ]; then
    ln -sf /dev/ttyUSB1 /dev/myserial 2>/dev/null || true
fi

if [ -e /dev/ttyUSB0 ]; then
    ln -sf /dev/ttyUSB0 /dev/ydlidar 2>/dev/null || true
fi

if [ -e /dev/ttyUSB2 ]; then
    ln -sf /dev/ttyUSB2 /dev/myspeech 2>/dev/null || true
fi

# Execute the command passed to the container
exec "$@"

