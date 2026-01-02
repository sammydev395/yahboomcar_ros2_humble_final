# ROS2 Humble Docker Container Setup

Complete guide for setting up a ROS2 Humble (Ubuntu 22.04) Docker container for the Yahboom Rosmaster X3 Plus 6-DOF robot, migrating from ROS2 Foxy.

## Table of Contents

1. [Quick Start](#quick-start)
2. [Files Overview](#files-overview)
3. [Migration Process](#migration-process)
4. [Package Analysis](#package-analysis)
5. [Container Configuration](#container-configuration)
6. [Testing](#testing)
7. [Troubleshooting](#troubleshooting)

---

## Quick Start

### Step 1: Load the Foxy Image

After downloading the ROS2 Foxy container image:

```bash
cd /home/jetson/yahboomcar_ros2_ws_new/docker_setup
./load_foxy_image.sh
```

This script will:
- Detect the downloaded file in `~/Downloads`
- Handle zip extraction if needed (large file, will take time)
- Load the Docker image
- Show you the image name/tag

### Step 2: Extract Information from Foxy Container

Since the image is a disk image, mount it first, then extract:

```bash
# Mount the image (if not already mounted)
sudo losetup -fP /home/jetson/yahboomcar_ros2_ws_new/docker_setup/foxy_image_extract/ROSMASTER_OrinNano_ROS2_20250822.img
LOOP_DEV=$(sudo losetup -j /home/jetson/yahboomcar_ros2_ws_new/docker_setup/foxy_image_extract/ROSMASTER_OrinNano_ROS2_20250822.img | cut -d: -f1)
sudo kpartx -av "$LOOP_DEV"
sudo mount /dev/mapper/loop0p1 /mnt/foxy_image

# Extract from mounted overlay
./extract_from_mounted_overlay.sh
```

This creates `foxy_container_extract/` with all extracted information.

### Step 3: Analyze and Update

```bash
./analyze_and_update_dockerfile.sh
```

Review the recommendations in `foxy_container_extract/MIGRATION_RECOMMENDATIONS.md`

### Step 4: Update Dockerfile and Entrypoint

Edit these files based on the recommendations:
- `Dockerfile.ros2.humble` - Add missing packages, dependencies
- `docker_entrypoint.sh` - Add startup scripts, environment setup

### Step 5: Build ROS2 Humble Container

```bash
./build_ros2_container.sh
```

### Step 6: Run the Container

```bash
./run_docker_ros2.sh
```

---

## Files Overview

### Scripts

- **`load_foxy_image.sh`** - Load the downloaded ROS2 Foxy container image
- **`extract_foxy_container_info.sh`** - Extract information from Foxy container (for loaded Docker images)
- **`extract_from_mounted_overlay.sh`** - Extract from mounted disk image overlay
- **`analyze_and_update_dockerfile.sh`** - Analyze extracted data and create recommendations
- **`build_ros2_container.sh`** - Build the ROS2 Humble Docker image
- **`run_docker_ros2.sh`** - Run the container with proper mounts and device access

### Docker Files

- **`Dockerfile.ros2.humble`** - Dockerfile for ROS2 Humble container (update based on Foxy extraction)
- **`docker_entrypoint.sh`** - Container entrypoint script (update based on Foxy extraction)

---

## Migration Process

### Step 1: Extract Information from Foxy Container

The Foxy container (`yahboomtechnology/ros-foxy-orbslam2:1.0.0`) contains:
- Dockerfile with ROS2 Foxy + ORB_SLAM2 setup
- Entrypoint script (`ros_entrypoint.sh`)
- System dependencies (3,479 packages)
- Python packages (375 packages)
- Environment variables and configurations
- Udev rules for USB devices
- ROS2 configurations

### Step 2: Review Extracted Information

Key files to review:
- `Dockerfile` - Original Foxy container setup
- `scripts/ros_entrypoint.sh` - Startup script
- `environment_variables.txt` - Environment setup
- `installed_packages.txt` - System packages
- `pip_packages.txt` - Python packages
- `udev_rules/` - USB device rules

### Step 3: Update Dockerfile

Based on the extracted information, update `Dockerfile.ros2.humble`:

1. **Add missing system packages** from `installed_packages.txt`
2. **Add Python dependencies** from `pip_packages.txt`
3. **Copy udev rules** from `udev_rules/`
4. **Add custom startup logic** from extracted scripts
5. **Copy hardware configurations** from `hardware_configs/`

### Step 4: Update Entrypoint Script

Update `docker_entrypoint.sh` based on extracted startup scripts:
- Add any initialization commands
- Set environment variables from `environment_variables.txt`
- Configure hardware devices
- Start any required services

### Step 5: Build the New Container

```bash
./build_ros2_container.sh
```

Or manually:
```bash
docker build -f Dockerfile.ros2.humble -t yahboom_ros2_humble:latest .
```

### Step 6: Run the New Container

```bash
./run_docker_ros2.sh
```

### Step 7: Verify and Test

Inside the container:
```bash
# Check ROS2 environment
source /opt/ros/humble/setup.bash
ros2 --version

# Source workspace (if built)
source /root/yahboomcar_ros2_ws/install/setup.bash

# Test nodes
ros2 node list
ros2 topic list
```

---

## Package Analysis

### Foxy Container Structure

The Foxy container (`yahboomtechnology/ros-foxy-orbslam2:1.0.0`) has:
- Workspace location: `/root/yahboomcar_ros2_ws/`
- Focused on ORB_SLAM2 and ROS2 Foxy base setup
- **NO ARM packages** in the container
- **NO `garbage_identify_yolov11`** in the container
- **NO `x3plus_moveit_config`** in the container

### ARM Packages Status

**ARM packages in Humble workspace:**
- ✅ `arm_autopilot`
- ✅ `arm_color_transport`
- ✅ `arm_mediapipe`
- ✅ `arm_moveit_demo`

**Status in Foxy container:**
- ❌ **NOT FOUND** - None of the ARM packages exist in the Foxy container

### Specific Packages Check

#### garbage_identify_yolov11
- **Humble workspace**: ✅ EXISTS at `/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws/src/garbage_identify_yolov11`
- **Foxy container**: ❌ NOT FOUND

#### x3plus_moveit_config
- **Humble workspace**: ✅ EXISTS at `/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws/src/x3plus_moveit_config`
- **Foxy container**: ❌ NOT FOUND

### Important: Workspace Mounting

**GOOD NEWS**: Since the workspace is **MOUNTED** (not copied) from host to container:
- **Host**: `/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws` → **Container**: `/root/yahboomcar_ros2_ws`

**All packages in the Humble workspace are automatically available in the container**, including:
- ✅ All ARM packages (`arm_autopilot`, `arm_color_transport`, `arm_mediapipe`, `arm_moveit_demo`)
- ✅ `garbage_identify_yolov11`
- ✅ `x3plus_moveit_config`
- ✅ All other packages in the workspace

### What Needs to Be Done

#### 1. No Action Needed for Package Files
Since the workspace is mounted, all packages are automatically accessible. **No need to copy or add packages to the container image.**

#### 2. Build Dependencies Required

The container needs to have build dependencies installed for these packages. **These are already added to `Dockerfile.ros2.humble`:**

**For ARM packages:**
- ✅ MoveIt2 packages: `ros-humble-moveit`, `ros-humble-moveit-core`, `ros-humble-moveit-ros-planning-interface`, `ros-humble-moveit-ros-move-group`, `ros-humble-moveit-kinematics`, `ros-humble-moveit-planners`, `ros-humble-moveit-ros-visualization`, `ros-humble-moveit-simple-controller-manager`, `ros-humble-moveit-msgs`
- ✅ MediaPipe: `mediapipe` (Python package)
- ✅ Python packages: `python3-numpy`, `python3-yaml`, `python3-opencv`
- ✅ ROS2 Control: `ros-humble-ros2-control`, `ros-humble-ros2-controllers`, `ros-humble-controller-manager`

**For garbage_identify_yolov11:**
- ✅ YOLOv11: `ultralytics` (Python package)
- ✅ PyTorch: `torch`, `torchvision` (Python packages)
- ✅ OpenCV: `libopencv-dev`, `python3-opencv`
- ✅ ROS2 vision packages: `ros-humble-cv-bridge`, `ros-humble-image-transport`

**For x3plus_moveit_config:**
- ✅ MoveIt2 packages: All MoveIt2 packages listed above
- ✅ Robot description: `ros-humble-xacro`, `ros-humble-urdf`, `ros-humble-robot-state-publisher`
- ✅ Visualization: `ros-humble-rviz2`, `ros-humble-joint-state-publisher-gui`
- ✅ ROS2 Control: `ros-humble-ros2-control`, `ros-humble-ros2-controllers`

**All dependencies are configured in the Dockerfile and will be installed when building the container.**

**For LiDAR, Astra Camera, and ORB-SLAM2:**

**LiDAR Support:**
- ✅ Basic ROS2 packages: `laser_filters`, `laser_geometry`, `depthimage_to_laserscan`
- ✅ LiDAR drivers: Mounted from `/home/jetson/yahboomcar_ros2_ws/software/library_ws` → `/root/library_ws` (contains `ydlidar_ros2_driver`, `sllidar_ros2`)
- ✅ YDLidar-SDK: Available in mounted `/root/software/YDLidar-SDK`

**Astra Camera Support:**
- ✅ OpenNI libraries: `libopenni-dev`, `libopenni2-dev` (installed)
- ✅ USB support: `libusb-1.0-0-dev` (installed)
- ✅ `yahboomcar_astra` package: Exists in workspace (needs to be built)

**ORB-SLAM2 Support:**
- ✅ Pangolin: Built from source in Dockerfile (or uses mounted version from `/root/software/orbslam2/Pangolin-0.6`)
- ✅ ORB_SLAM2: Built from source in Dockerfile (or uses mounted version from `/root/software/orbslam2/ORB_SLAM2-master`)
- ✅ Environment variables: `ORB_SLAM2_ROOT_DIR`, `LD_LIBRARY_PATH` (configured)
- ✅ Dependencies: `libglew-dev`, `ffmpeg`, `libcanberra-gtk-module` (installed)
- ✅ Additional libraries: OpenCV, Eigen, Boost (installed)

**Note on PyTorch for Jetson**: The Dockerfile installs PyTorch via pip, which should work on ARM64. However, for optimal performance on Jetson, you may want to use NVIDIA's pre-built PyTorch wheels. If you encounter issues, you can install Jetson-optimized PyTorch after container creation:
```bash
# Inside container, if needed:
pip3 install --pre torch torchvision --index-url https://download.pytorch.org/whl/nightly/cu121
```

#### 3. ROS2 Compatibility Check
**IMPORTANT**: These packages may still be ROS1 (catkin-based):
- ARM packages might need ROS2 migration
- Check if they're using `ament_cmake` or `catkin` build system

---

## Container Configuration

### Workspace Mapping

The ROS2 workspace is mounted from the host:
- **Host**: `/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws`
- **Container**: `/root/yahboomcar_ros2_ws`
- **Type**: Read-write mount (changes persist on host)

### Additional Mounts

- **Library workspace**: `/home/jetson/yahboomcar_ros2_ws/software/library_ws` → `/root/library_ws` (read-write)
  - Contains: `ydlidar_ros2_driver`, `sllidar_ros2` (LiDAR drivers)
- **Software directory**: `/home/jetson/yahboomcar_ros2_ws_new/software` → `/root/software` (read-write)
  - Contains: ORB_SLAM2, Pangolin, YDLidar-SDK, and other libraries

### Port Mappings (matching ROS1 container)

- `9090:9090` - Web interface
- `8888:8888` - Jupyter/notebooks
- `6000:6000` - X11

### Device Mappings (matching ROS1 container)

- Serial ports: `/dev/ttyUSB0`, `/dev/ttyUSB1`, `/dev/ttyUSB2`
- Device symlinks: `/dev/myserial`, `/dev/ydlidar`
- Cameras: `/dev/video0`, `/dev/video1`, `/dev/video2`, `/dev/video3`
- USB buses: `/dev/bus/usb/001/009`, `/dev/bus/usb/001/012`, `/dev/bus/usb/001/007`
- Input devices: `/dev/input`
- Astro devices: `/dev/astro_pro_plus`, `/dev/astro_pro_plus_rgb`

### Container Features

- **ROS2 Humble** on Ubuntu 22.04
- **GPU Support**: NVIDIA runtime for Jetson (`--gpus all`)
- **X11 Forwarding**: For GUI applications
- **Network**: Host mode for ROS2 DDS communication
- **Privileged**: Full hardware access

### Usage

#### Enter the Container

```bash
docker exec -it yahboom_ros2_humble bash
```

#### Inside the Container

```bash
# ROS2 environment is already sourced
ros2 --version

# Source workspace (if built)
source /root/yahboomcar_ros2_ws/install/setup.bash

# Run ROS2 nodes
ros2 launch yahboomcar_bringup yahboomcar_bringup_X3plus_launch.py
```

#### Build Workspace in Container

```bash
cd /root/yahboomcar_ros2_ws
colcon build
source install/setup.bash
```

#### Test Package Builds

```bash
cd /root/yahboomcar_ros2_ws
colcon build --packages-select arm_autopilot
colcon build --packages-select garbage_identify_yolov11
colcon build --packages-select x3plus_moveit_config
```

---

## Testing

### Test Plan

Once the container is running and the workspace is built, follow the comprehensive test plan:

**Location**: `/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws/docs/TEST_PLAN.md`

Or inside the container:
```bash
cat /root/yahboomcar_ros2_ws/docs/TEST_PLAN.md
```

### Test Plan Overview

The test plan covers 5 phases:

1. **Phase 1: Non-Movement Sensors & Peripherals** (Safe - robot stationary)
   - LiDAR verification
   - Voice control module
   - Astra depth camera
   - IMU data

2. **Phase 2: Arm Tests** (Stationary robot - only arm moves)
   - Arm joint states
   - Arm movement commands
   - Gripper control
   - Arm camera

3. **Phase 3: Chassis Movement Tests** (⚠️ Robot will move)
   - Buzzer & RGB
   - Chassis movement (cmd_vel)
   - Odometry feedback

4. **Phase 4: Joystick Integration**
   - Joystick connection
   - Full joystick control

5. **Phase 5: Full System Integration**
   - Full bringup with RViz
   - SLAM mapping
   - Arm applications (autopilot, MediaPipe)

### Running Tests in Container

```bash
# Enter container
docker exec -it yahboom_ros2_humble bash

# Source environment
source /opt/ros/humble/setup.bash
source /root/yahboomcar_ros2_ws/install/setup.bash

# Follow test plan
# Start with Phase 1 (safe tests)
```

### All Packages are ROS2 Humble

All packages in `/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws/src` are ROS2 Humble compatible. The test plan verifies:
- Hardware drivers (LiDAR, camera, IMU)
- Arm control packages
- Chassis control
- Navigation and SLAM
- Vision applications
- Integration testing

---

## Troubleshooting

### Container won't start
- Check if image is built: `docker images | grep yahboom_ros2_humble`
- Build it: `./build_ros2_container.sh`

### Devices not accessible
- Check device permissions on host
- Verify device mappings in `run_docker_ros2.sh`
- Container runs with `--privileged` flag

### X11/Display issues
- Run `xhost +` on host (matching ROS1 container)
- Check `DISPLAY` environment variable

### Workspace not found
- Verify workspace path in `run_docker_ros2.sh`
- Check that workspace directory exists on host: `/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws`

### Download still in progress
The `load_foxy_image.sh` script will detect `.crdownload` files and wait if needed.

### Image load fails
- Check file is complete (no `.crdownload` extension)
- Verify it's a valid Docker image format (.tar, .tar.gz, or .img)
- Check available disk space (160GB+ needed for extraction)

### Missing packages
**Solution**: Add to Dockerfile based on `installed_packages.txt` from extraction

### Device permissions
**Solution**: Check udev rules and add to container from `udev_rules/` directory

### Environment variables not set
**Solution**: Add to `docker_entrypoint.sh` or Dockerfile ENV from `environment_variables.txt`

### ROS2 nodes not starting
**Solution**: Check launch files and parameters, verify workspace is sourced

---

## Migration Checklist

- [x] Extract Foxy container information
- [x] Analyze extracted data
- [ ] Update Dockerfile with dependencies
- [ ] Update entrypoint script
- [ ] Copy udev rules (if any)
- [ ] Copy hardware configurations (if any)
- [ ] Build new Humble container
- [ ] Test container startup
- [ ] Verify device access (USB, serial, cameras)
- [ ] Test ROS2 nodes
- [ ] Verify workspace mounting
- [ ] Test ARM package builds
- [ ] Test garbage_identify_yolov11 build
- [ ] Test x3plus_moveit_config build
- [ ] Run comprehensive test plan (TEST_PLAN.md)
- [ ] Complete Phase 1 tests (sensors, no movement)
- [ ] Complete Phase 2 tests (arm, stationary)
- [ ] Complete Phase 3 tests (chassis movement)
- [ ] Complete Phase 4 tests (joystick)
- [ ] Complete Phase 5 tests (full integration)

---

## Notes

- The workspace is **mounted** from host, so builds persist on the host
- The container is based on ROS2 Humble (Ubuntu 22.04)
- All device mappings match the ROS1 container setup
- Network is in host mode for ROS2 DDS communication
- ARM packages and other packages are available via workspace mount - no copying needed
- Build dependencies may need to be added to Dockerfile for ARM packages, YOLOv11, and MoveIt
