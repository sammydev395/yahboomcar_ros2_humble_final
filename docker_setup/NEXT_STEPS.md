# Next Steps - ROS2 Humble Container Setup

## ✅ Completed

1. ✅ Extracted Foxy container information
2. ✅ Analyzed extracted data and created recommendations
3. ✅ Updated Dockerfile with all build dependencies:
   - MoveIt2 packages (for ARM packages)
   - PyTorch & Ultralytics (for YOLOv11)
   - MediaPipe (for arm_mediapipe)
   - All Python dependencies
4. ✅ Created comprehensive README documentation
5. ✅ All scripts are ready (load, extract, analyze, build, run)

## 🔄 Next Steps (In Order)

### Step 1: Copy Udev Rules (Optional but Recommended)

The Foxy container has a `usb.rules` file. Check if it's needed:

```bash
cd /home/jetson/yahboomcar_ros2_ws_new/docker_setup
cat foxy_container_extract/udev_rules/usb.rules
```

If it contains important USB device rules, the Dockerfile is already configured to copy it. If not, you can skip this.

### Step 2: Build the Container

```bash
cd /home/jetson/yahboomcar_ros2_ws_new/docker_setup
./build_ros2_container.sh
```

This will:
- Build the Docker image with all dependencies
- Install MoveIt2, PyTorch, YOLOv11, MediaPipe, etc.
- Set up the ROS2 Humble environment
- Take 15-30 minutes depending on your system

### Step 3: Run the Container

```bash
./run_docker_ros2.sh
```

This will:
- Start the container with all device mappings
- Mount the workspace from host
- Enter the container automatically

### Step 4: Verify Container Setup

Inside the container:

```bash
# Check ROS2 is working
ros2 --version

# Check workspace is mounted
ls -la /root/yahboomcar_ros2_ws/src

# Check all packages are visible
ls /root/yahboomcar_ros2_ws/src/

# Verify Python dependencies
python3 -c "import torch; print('PyTorch:', torch.__version__)"
python3 -c "import ultralytics; print('Ultralytics OK')"
python3 -c "import mediapipe; print('MediaPipe OK')"

# Verify LiDAR, Astra, ORB-SLAM2 dependencies
ls -d /root/library_ws/src/ydlidar_ros2_driver && echo "✅ LiDAR drivers mounted"
ls -d /root/software/orbslam2/ORB_SLAM2-master && echo "✅ ORB-SLAM2 available"
ls -d /root/software/orbslam2/Pangolin-0.6 && echo "✅ Pangolin available"
dpkg -l | grep -E "libopenni|libusb" && echo "✅ OpenNI/USB libraries installed"
echo "ORB_SLAM2_ROOT_DIR: $ORB_SLAM2_ROOT_DIR" && echo "✅ ORB-SLAM2 env vars set"
```

### Step 5: Build the Workspace

```bash
cd /root/yahboomcar_ros2_ws

# Source ROS2
source /opt/ros/humble/setup.bash

# Build all packages
colcon build

# Or build specific packages first
colcon build --packages-select yahboomcar_msgs
colcon build --packages-select arm_autopilot
colcon build --packages-select garbage_identify_yolov11
colcon build --packages-select x3plus_moveit_config

# Source workspace
source install/setup.bash
```

### Step 6: Run Test Plan

Follow the comprehensive test plan:

```bash
# View test plan
cat /root/yahboomcar_ros2_ws/docs/TEST_PLAN.md

# Start with Phase 1 (safe tests - no movement)
# Test LiDAR, camera, IMU, etc.
```

## Quick Reference Commands

### Build Container
```bash
cd /home/jetson/yahboomcar_ros2_ws_new/docker_setup
./build_ros2_container.sh
```

### Run Container
```bash
./run_docker_ros2.sh
```

### Enter Running Container
```bash
docker exec -it yahboom_ros2_humble bash
```

### Check Container Status
```bash
docker ps -a | grep yahboom_ros2_humble
```

### View Container Logs
```bash
docker logs yahboom_ros2_humble
```

## Troubleshooting

If build fails:
- Check disk space: `df -h`
- Check Docker is running: `docker ps`
- Review build output for specific errors

If container won't start:
- Check image exists: `docker images | grep yahboom_ros2_humble`
- Verify workspace path exists on host
- Check device permissions

If packages won't build:
- Verify dependencies are installed
- Check for ROS2 compatibility issues
- Review build errors in `log/` directory

## Summary

**Ready to build!** All dependencies are configured. The next step is to build the container and test it.

