# Migration Recommendations

Generated on: Thu Jan  1 03:54:14 PM PST 2026

## 1. Dockerfile Updates

### System Packages
Review and add packages from: `recommended_packages.txt`

Common packages to check:
- Development tools
- Hardware libraries (USB, serial, camera)
- ROS2 packages (check if Humble equivalents exist)

### Python Packages
Add to Dockerfile:
```dockerfile
RUN pip3 install -r /path/to/requirements.txt
```

Or individually from: `recommended_pip_packages.txt`

### Udev Rules
Add to Dockerfile:
```dockerfile
COPY foxy_container_extract/udev_rules/* /etc/udev/rules.d/
```

## 2. Entrypoint Script Updates

Review startup scripts in: `scripts/`

Common things to add:
- Device symlink creation
- Service startup
- Environment variable exports
- ROS2 workspace sourcing

## 3. Environment Variables

Review: `important_env_vars.txt`

Add to Dockerfile ENV or entrypoint script as needed.

## 4. Hardware Configurations

Review: `hardware_configs/`

These may need to be:
- Copied into container
- Mounted as volumes
- Configured at runtime

## 5. ROS2 Configurations

Review: `ros_configs/`

These are likely already in your workspace, but check if any need to be in the container image.

## Next Steps

1. Review all recommendation files
2. Update Dockerfile.ros2.humble
3. Update docker_entrypoint.sh
4. Build container
5. Test and verify
