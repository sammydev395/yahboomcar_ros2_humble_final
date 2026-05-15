#!/bin/bash
# Original entrypoint tasks
ln -sf /dev/ttyUSB1 /dev/myserial 2>/dev/null || true
ln -sf /dev/ttyUSB0 /dev/ydlidar 2>/dev/null || true

# Source ROS2
source /opt/ros/humble/setup.bash
export ROS_DOMAIN_ID=100
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
source /root/yahboomcar_ros2_ws/install/setup.bash

# Launch Astra camera
ros2 launch yahboomcar_astra astra.launch.py > /tmp/astra.log 2>&1 &

# Launch USB arm camera
ros2 run usb_cam usb_cam_node_exe --ros-args   -p video_device:=/dev/video0   -p image_width:=640   -p image_height:=480   -p framerate:=15.0   -r image_raw:=/rosmaster/arm_cam/image_raw > /tmp/usb_cam.log 2>&1 &

# Wait for topics then start web video server
sleep 15
source /root/yahboomcar_ros2_ws_new/software/library_ws/install/setup.bash
/root/yahboomcar_ros2_ws_new/software/library_ws/install/web_video_server/lib/web_video_server/web_video_server   --ros-args -p port:=8090 > /tmp/web_video.log 2>&1 &

echo All ROS2 camera nodes started

# Keep container alive
tail -f /dev/null
