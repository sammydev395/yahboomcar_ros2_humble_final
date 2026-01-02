# ROS2 Foxy Container Extraction Summary

Extracted on: Thu Jan  1 03:54:07 PM PST 2026

## Source Information
- Image: yahboomtechnology/ros-foxy-orbslam2:1.0.0
- Overlay: 0f11ca337f9f88f0914940ad0a25e76796117b10ae6dab740c5841580ea53d3d
- Source: Mounted filesystem at /mnt/foxy_image

## Extracted Components

### 1. Docker Information
- Dockerfile: `Dockerfile`

### 2. Startup Scripts
- Scripts: `scripts/`
- Entrypoint: `scripts/ros_entrypoint.sh`

### 3. Environment Variables
- /etc/environment: `etc_environment.txt`
- ~/.bashrc: `bashrc.txt`
- ~/.profile: `profile.txt`
- Environment variables: `environment_variables.txt`

### 4. System Dependencies
- Installed packages: `installed_packages.txt`
- Python packages: `pip_packages.txt`
- APT sources: `apt_sources.txt`
- Udev rules: `udev_rules/`

### 5. ROS2 Configurations
- Config list: `ros_configs_list.txt`
- Extracted configs: `ros_configs/`
- Workspace locations: `workspace_locations.txt`

### 6. Hardware Configurations
- Hardware config list: `hardware_configs_list.txt`
- Device list: `devices_list.txt`
- Modprobe configs: `modprobe_configs.txt`
- Extracted configs: `hardware_configs/`

## Next Steps

1. Review the extracted files
2. Run: `./analyze_and_update_dockerfile.sh`
3. Update Dockerfile.ros2.humble based on recommendations
4. Build and test the new Humble container
