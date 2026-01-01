# Quick Start Guide - ROS2 Humble Docker Setup

## Current Status

✅ All scripts created and ready  
⏳ Waiting for Foxy container image download (82GB zip, ~160GB uncompressed)

## Once Download Completes

### Step 1: Load the Foxy Image

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

```bash
./extract_foxy_container_info.sh <foxy_image_name_or_container>
```

Replace `<foxy_image_name_or_container>` with the actual name from Step 1.

This creates `foxy_container_extract/` with all extracted information.

### Step 3: Analyze and Update

```bash
./analyze_and_update_dockerfile.sh
```

Review the recommendations in `foxy_container_extract/MIGRATION_RECOMMENDATIONS.md`

### Step 4: Update Dockerfile and Entrypoint

Edit these files based on the recommendations:
- `Dockerfile.ros2.humble` - Add missing packages
- `docker_entrypoint.sh` - Add startup logic

### Step 5: Build ROS2 Humble Container

```bash
./build_ros2_container.sh
```

### Step 6: Run the Container

```bash
./run_docker_ros2.sh
```

## Important Mappings

### Workspace Mount
- **Host**: `/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws`
- **Container**: `/root/yahboomcar_ros2_ws`
- **Type**: Read-write (changes persist on host)

### Port Mappings (matching ROS1 container)
- `9090:9090` - Web interface
- `8888:8888` - Jupyter/notebooks
- `6000:6000` - X11

### Device Mappings (matching ROS1 container)
- Serial ports: `/dev/ttyUSB0`, `/dev/ttyUSB1`, `/dev/ttyUSB2`
- Device symlinks: `/dev/myserial`, `/dev/ydlidar`
- Cameras: `/dev/video0-3`
- USB buses: `/dev/bus/usb/001/009`, `/dev/bus/usb/001/012`, `/dev/bus/usb/001/007`
- Input devices: `/dev/input`
- Astro devices: `/dev/astro_pro_plus`, `/dev/astro_pro_plus_rgb`

## Notes

- The workspace is **mounted** from host, not copied - no duplication
- All device mappings match the ROS1 container exactly
- Container runs with `--privileged` and `--gpus all` for full hardware access
- Network is in `host` mode for ROS2 DDS communication

## Troubleshooting

### Download still in progress
The `load_foxy_image.sh` script will detect `.crdownload` files and wait if needed.

### Image load fails
- Check file is complete (no `.crdownload` extension)
- Verify it's a valid Docker image format (.tar, .tar.gz)
- Check available disk space (160GB+ needed for extraction)

### Workspace not found
The script automatically detects the workspace at:
`/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws`

If it's in a different location, edit `run_docker_ros2.sh` line 30.

