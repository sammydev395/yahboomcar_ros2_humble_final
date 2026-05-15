#!/bin/bash
# Auto-start ROS2 camera nodes + web video server in yahboom_ros2_humble container
set -e
CONTAINER=yahboom_ros2_humble
ENV='source /opt/ros/humble/setup.bash && export ROS_DOMAIN_ID=100 && export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp && source /root/yahboomcar_ros2_ws/install/setup.bash'

# Wait for container
for i in {1..30}; do
  docker inspect -f '{{.State.Running}}' $CONTAINER 2>/dev/null | grep -q true && break
  sleep 2
done

# Astra camera
docker exec -d $CONTAINER bash -c "$ENV && ros2 launch yahboomcar_astra astra.launch.py > /tmp/astra.log 2>&1"

# USB arm camera
docker exec -d $CONTAINER bash -c "$ENV && ros2 run usb_cam usb_cam_node_exe --ros-args -p video_device:=/dev/video0 -p image_width:=640 -p image_height:=480 -p framerate:=15.0 -r image_raw:=/rosmaster/arm_cam/image_raw > /tmp/usb_cam.log 2>&1"

# Wait for topics before starting web video server
sleep 15

# Web video server
docker exec -d $CONTAINER bash -c "$ENV && source /root/yahboomcar_ros2_ws_new/software/library_ws/install/setup.bash && /root/yahboomcar_ros2_ws_new/software/library_ws/install/web_video_server/lib/web_video_server/web_video_server --ros-args -p port:=8090 > /tmp/web_video.log 2>&1"

echo 'All ROS2 nodes started'
