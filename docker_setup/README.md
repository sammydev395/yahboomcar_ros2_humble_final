# ROS2 Humble Docker Setup

This directory contains all scripts and files needed to set up a ROS2 Humble Docker container for the Yahboom Rosmaster X3 Plus 6-DOF robot.

## Files Overview

### Scripts
- **`load_foxy_image.sh`** - Load the downloaded ROS2 Foxy container image
- **`extract_foxy_container_info.sh`** - Extract information from Foxy container
- **`analyze_and_update_dockerfile.sh`** - Analyze extracted data and create recommendations
- **`build_ros2_container.sh`** - Build the ROS2 Humble Docker image
- **`run_docker_ros2.sh`** - Run the container with proper mounts and device access

### Docker Files
- **`Dockerfile.ros2.humble`** - Dockerfile for ROS2 Humble container
- **`docker_entrypoint.sh`** - Container entrypoint script

### Documentation
- **`QUICK_START.md`** - Quick start guide
- **`MIGRATION_GUIDE.md`** - Detailed migration guide from Foxy to Humble
- **`README_DOCKER_SETUP.md`** - Complete setup documentation

## Quick Start

1. **Load Foxy image** (after download completes):
   ```bash
   cd /home/jetson/yahboomcar_ros2_ws_new/docker_setup
   ./load_foxy_image.sh
   ```

2. **Extract Foxy container info**:
   ```bash
   ./extract_foxy_container_info.sh <foxy_image_name>
   ```

3. **Analyze and update**:
   ```bash
   ./analyze_and_update_dockerfile.sh
   ```

4. **Build container**:
   ```bash
   ./build_ros2_container.sh
   ```

5. **Run container**:
   ```bash
   ./run_docker_ros2.sh
   ```

## Workspace Mapping

The workspace is mounted from:
- **Host**: `/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws`
- **Container**: `/root/yahboomcar_ros2_ws`

All scripts automatically detect the correct paths.

## See Also

- `QUICK_START.md` - Step-by-step quick start
- `MIGRATION_GUIDE.md` - Detailed migration process
- `README_DOCKER_SETUP.md` - Complete documentation

