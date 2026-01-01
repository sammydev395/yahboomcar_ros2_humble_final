# ROS2 Humble Docker Container Setup

This directory contains scripts and files to set up a ROS2 Humble (Ubuntu 22.04) Docker container for the Yahboom Rosmaster X3 Plus 6-DOF robot, similar to the existing ROS1 container.

## Quick Start

### Step 1: Extract Information from Old Foxy Container

After downloading and loading the old ROS2 Foxy container:

```bash
# Load the container image (if it's a tar file)
docker load -i <foxy_container_image.tar>

# List available containers/images to find the Foxy container
docker images
docker ps -a

# Extract information from the Foxy container
cd /home/jetson/yahboomcar_ros2_ws_new/docker_setup
./extract_foxy_container_info.sh <container_name_or_image>
```

This creates a `foxy_container_extract/` directory with all extracted information.

### Step 2: Analyze Extracted Data

```bash
./analyze_and_update_dockerfile.sh
```

This analyzes the extracted data and creates recommendation files for updating the Dockerfile.

### Step 3: Review and Update Dockerfile

Review the recommendations and update:
- `Dockerfile.ros2.humble` - Add missing packages, dependencies
- `docker_entrypoint.sh` - Add startup scripts, environment setup

### Step 4: Build the Container

```bash
./build_ros2_container.sh
```

Or manually:
```bash
docker build -f Dockerfile.ros2.humble -t yahboom_ros2_humble:latest .
```

### Step 5: Run the Container

```bash
./run_docker_ros2.sh
```

## Files Overview

### Scripts

- **`extract_foxy_container_info.sh`** - Extracts all information from the old Foxy container
- **`analyze_and_update_dockerfile.sh`** - Analyzes extracted data and creates recommendations
- **`build_ros2_container.sh`** - Builds the Docker image
- **`run_docker_ros2.sh`** - Runs the container with proper mounts and device access

### Docker Files

- **`Dockerfile.ros2.humble`** - Dockerfile for ROS2 Humble container (update based on Foxy extraction)
- **`docker_entrypoint.sh`** - Container entrypoint script (update based on Foxy extraction)

### Documentation

- **`MIGRATION_GUIDE.md`** - Detailed migration guide
- **`README_DOCKER_SETUP.md`** - This file

## Container Features

The container is configured with:

- **ROS2 Humble** on Ubuntu 22.04
- **Workspace Mount**: `/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws` → `/root/yahboomcar_ros2_ws`
- **Device Access**: USB devices, serial ports, cameras, input devices
- **GPU Support**: NVIDIA runtime for Jetson
- **X11 Forwarding**: For GUI applications
- **Network**: Host mode for ROS2 communication

## Workspace Mapping

The ROS2 workspace is mounted from the host:
- Host: `/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws`
- Container: `/root/yahboomcar_ros2_ws`

Changes made in the container persist on the host.

## Device Mappings

The container has access to:
- Serial ports: `/dev/ttyUSB0`, `/dev/ttyUSB1`, `/dev/ttyUSB2`
- Device symlinks: `/dev/myserial`, `/dev/ydlidar`
- Cameras: `/dev/video0`, `/dev/video1`, `/dev/video2`, `/dev/video3`
- USB devices: Full USB bus access
- Input devices: `/dev/input`

## Usage

### Enter the Container

```bash
docker exec -it yahboom_ros2_humble bash
```

### Inside the Container

```bash
# ROS2 environment is already sourced
ros2 --version

# Source workspace (if built)
source /root/yahboomcar_ros2_ws/install/setup.bash

# Run ROS2 nodes
ros2 launch yahboomcar_bringup yahboomcar_bringup_X3plus_launch.py
```

### Build Workspace in Container

```bash
cd /root/yahboomcar_ros2_ws
colcon build
source install/setup.bash
```

## Troubleshooting

### Container won't start
- Check if image is built: `docker images | grep yahboom_ros2_humble`
- Build it: `./build_ros2_container.sh`

### Devices not accessible
- Check device permissions on host
- Verify device mappings in `run_docker_ros2.sh`
- Container runs with `--privileged` flag

### X11/Display issues
- Run `xhost +local:docker` on host
- Check `DISPLAY` environment variable

### Workspace not found
- Verify workspace path in `run_docker_ros2.sh`
- Check that workspace directory exists on host

## Migration Checklist

- [ ] Extract Foxy container information
- [ ] Analyze extracted data
- [ ] Update Dockerfile with dependencies
- [ ] Update entrypoint script
- [ ] Copy udev rules (if any)
- [ ] Copy hardware configurations (if any)
- [ ] Build new Humble container
- [ ] Test container startup
- [ ] Verify device access
- [ ] Test ROS2 nodes
- [ ] Verify workspace mounting

## Notes

- The workspace is mounted, so builds persist on the host
- The container is based on ROS2 Humble (Ubuntu 22.04)
- All device mappings match the ROS1 container setup
- Network is in host mode for ROS2 DDS communication

