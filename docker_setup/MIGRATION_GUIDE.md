# ROS2 Foxy to Humble Container Migration Guide

This guide documents the process of migrating from the old ROS2 Foxy (Ubuntu 20.04) container to a new ROS2 Humble (Ubuntu 22.04) container.

## Step 1: Extract Information from Foxy Container

After downloading and loading the old Foxy container image:

```bash
# Load the container image (if it's a tar file)
docker load -i <foxy_container_image.tar>

# List available containers/images
docker images
docker ps -a

# Run the extraction script
cd /home/jetson/yahboomcar_ros2_ws_new
chmod +x extract_foxy_container_info.sh
./extract_foxy_container_info.sh <container_name_or_image>
```

This will create a `foxy_container_extract/` directory with all extracted information.

## Step 2: Review Extracted Information

Review the extraction summary:
```bash
cat foxy_container_extract/EXTRACTION_SUMMARY.md
```

Key files to review:
- `docker_inspect.json` - Container configuration
- `environment_variables.txt` - Environment setup
- `installed_packages.txt` - System packages
- `pip_packages.txt` - Python packages
- `startup_scripts_list.txt` - Startup scripts
- `ros_configs/` - ROS2 configurations
- `hardware_configs/` - Hardware configurations

## Step 3: Update Dockerfile

Based on the extracted information, update `Dockerfile.ros2.humble`:

1. **Add missing system packages** from `installed_packages.txt`
2. **Add Python dependencies** from `pip_packages.txt`
3. **Copy udev rules** from `udev_rules/`
4. **Add custom startup logic** from extracted scripts
5. **Copy hardware configurations** from `hardware_configs/`

## Step 4: Update Entrypoint Script

Update `docker_entrypoint.sh` based on extracted startup scripts:
- Add any initialization commands
- Set environment variables from `environment_variables.txt`
- Configure hardware devices
- Start any required services

## Step 5: Build the New Container

```bash
cd /home/jetson/yahboomcar_ros2_ws_new
docker build -f Dockerfile.ros2.humble -t yahboom_ros2_humble:latest .
```

## Step 6: Run the New Container

```bash
chmod +x run_docker_ros2.sh
./run_docker_ros2.sh
```

Or manually:
```bash
docker run -it --rm \
    --name yahboom_ros2_humble \
    --privileged \
    --gpus all \
    --runtime=nvidia \
    --net=host \
    -v /home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws:/root/yahboomcar_ros2_ws \
    yahboom_ros2_humble:latest
```

## Step 7: Verify and Test

Inside the container:
```bash
# Check ROS2 environment
source /opt/ros/humble/setup.bash
ros2 --version

# Source workspace
source /root/yahboomcar_ros2_ws/install/setup.bash

# Test nodes
ros2 node list
ros2 topic list
```

## Migration Checklist

- [ ] Extract Foxy container information
- [ ] Review extracted files
- [ ] Update Dockerfile with dependencies
- [ ] Update entrypoint script
- [ ] Copy udev rules
- [ ] Copy hardware configurations
- [ ] Copy ROS2 launch/config files (if needed)
- [ ] Build new Humble container
- [ ] Test container startup
- [ ] Verify device access (USB, serial, cameras)
- [ ] Test ROS2 nodes
- [ ] Verify workspace mounting

## Common Issues and Solutions

### Issue: Missing packages
**Solution**: Add to Dockerfile based on `installed_packages.txt`

### Issue: Device permissions
**Solution**: Check udev rules and add to container

### Issue: Environment variables not set
**Solution**: Add to `docker_entrypoint.sh` or Dockerfile ENV

### Issue: Workspace not found
**Solution**: Verify mount path in `run_docker_ros2.sh`

### Issue: ROS2 nodes not starting
**Solution**: Check launch files and parameters, verify workspace is sourced

## Notes

- The workspace is mounted from host, so changes persist
- Device mappings match the ROS1 container setup
- X11 forwarding is enabled for GUI applications
- Network is set to host mode for ROS2 communication

