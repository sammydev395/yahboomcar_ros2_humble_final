# Yahboom ROS2 container setup
_Exported on 1/1/2026 at 12:46:57 PST from Cursor (2.2.44)_

---

**User**

we have a docker container on this yahboom rosmaster x3plus 6-dof robot that has all ros1 code and which is run by the ~/run_docker.sh bash l s-l ros-imagesscript, In this cursor ide workspace we have ros2 humble code at /home/jetson/yahboomcar_ros2_ws_new whichis provided by the vendor but to which we have added some missing migrations  te chat history for which is at /home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws/docs/cursor_yahboom_car_ros2_migration_statu.md. Everything is built but now we want to setup a container to run our new ros2 code much like the existing youthful_bartik ros1 container that is stopped right now

You will need to search and see if the vendor along withthe new ros2 code has provided the necessary startup scripts 

---

**Cursor**


Reviewing the migration status, the existing Docker run script, and the ROS2 workspace for startup scripts.



Searching for Docker-related files and startup scripts in the ROS2 workspace:



Checking available Docker images and searching for ROS2 Docker setup documentation:



Checking the ROS1 container setup and searching for ROS2 Docker documentation:



Checking the actual ROS1 container image and its base:



Checking if ROS2 Humble is installed and reviewing the workspace structure:



ROS2 Humble is installed. Creating a Docker setup for ROS2 similar to the ROS1 container. Checking for an official ROS2 Humble Docker image and creating the necessary files:



Creating a ROS2 Docker setup similar to the ROS1 container. Checking for a vendor-provided Docker setup, then creating the necessary files:


---

**User**

i am downloading the 12+ month old rosmaster x3plus system file image form yahboom gdrive and i know it has a ros2 foxy ubuntu 20.04 docker container with all the necessary overlay files and ros2 foxy packages. We will fire up that container and from there 
Option 1: Inspect & Rebuild for Humble
1. Find the container/image ID from these overlay files
2. Run the container to explore startup scripts, dependencies, configurations
3. Extract or copy the Dockerfile and custom code
4. Create a new Dockerfile based on ROS2 Humble base image

What to extract from the old container:
1. Dockerfile (if available) - shows system dependencies, environment setup
2. Entrypoint/startup scripts - initialization sequence, service startup order
3. Environment variables - ROS_DOMAIN_ID, network config, device paths
4. System dependencies - apt packages, Python libraries, udev rules
5. Launch configurations - which nodes start automatically, parameters
6. Hardware configurations - serial port settings, camera configs, sensor calibrations

We will then setup this new new humble ubuntu 22.04 container in which:
1. map our  host /home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws into this new humble ubuntu 22.04 container 
2. Migrate al the extracted stuff from the old foxy container for this new humble based container


---

**Cursor**


Preparing extraction and migration scripts. Creating a structured plan and helper scripts to extract from the Foxy container and build the Humble container.


```shellscript
#!/bin/bash
# Script to extract information from the old ROS2 Foxy container
# Run this after loading the old container image

set -e

EXTRACT_DIR="./foxy_container_extract"
mkdir -p "$EXTRACT_DIR"

echo "=== Extracting information from ROS2 Foxy container ==="
echo ""

# Check if container/image exists
if [ -z "$1" ]; then
    echo "Usage: $0 <container_name_or_id>"
    echo "Example: $0 yahboom_foxy_container"
    echo ""
    echo "Available containers/images:"
    docker ps -a
    docker images | head -10
    exit 1
fi

CONTAINER_NAME="$1"

# Check if it's a running/stopped container or an image
if docker ps -a --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
    echo "Found container: $CONTAINER_NAME"
    CONTAINER_ID=$(docker ps -a --filter "name=${CONTAINER_NAME}" --format "{{.ID}}")
    IS_CONTAINER=true
elif docker images --format '{{.Repository}}:{{.Tag}}' | grep -q "${CONTAINER_NAME}"; then
    echo "Found image: $CONTAINER_NAME"
    IMAGE_NAME="$CONTAINER_NAME"
    IS_CONTAINER=false
else
    echo "Error: Container or image '$CONTAINER_NAME' not found"
    exit 1
fi

echo "Creating temporary container for extraction..."
if [ "$IS_CONTAINER" = true ]; then
    # Create a temporary container from the existing one
    TEMP_CONTAINER="foxy_extract_temp_$$"
    docker commit "$CONTAINER_ID" "temp_foxy_extract:latest"
    docker run -d --name "$TEMP_CONTAINER" "temp_foxy_extract:latest" tail -f /dev/null
    EXTRACT_SOURCE="$TEMP_CONTAINER"
else
    TEMP_CONTAINER="foxy_extract_temp_$$"
    docker run -d --name "$TEMP_CONTAINER" "$IMAGE_NAME" tail -f /dev/null
    EXTRACT_SOURCE="$TEMP_CONTAINER"
fi

echo "Extracting information..."

# 1. Extract Dockerfile (if available) or inspect image
echo "[1/6] Extracting Dockerfile and image information..."
docker inspect "$EXTRACT_SOURCE" > "$EXTRACT_DIR/docker_inspect.json"
docker history "$EXTRACT_SOURCE" > "$EXTRACT_DIR/docker_history.txt" 2>&1 || true

# Try to find Dockerfile in common locations
docker exec "$EXTRACT_SOURCE" find / -name "Dockerfile" -o -name "*.dockerfile" 2>/dev/null | head -5 > "$EXTRACT_DIR/dockerfile_locations.txt" || true

# 2. Extract entrypoint/startup scripts
echo "[2/6] Extracting entrypoint and startup scripts..."
docker exec "$EXTRACT_SOURCE" find / -type f -name "*entrypoint*" -o -name "*startup*" -o -name "*init*" 2>/dev/null | grep -E "(entrypoint|startup|init|\.sh)" | head -20 > "$EXTRACT_DIR/startup_scripts_list.txt" || true

mkdir -p "$EXTRACT_DIR/scripts"
for script in $(cat "$EXTRACT_DIR/startup_scripts_list.txt" 2>/dev/null | head -10); do
    if [ -n "$script" ]; then
        script_name=$(basename "$script")
        docker cp "$EXTRACT_SOURCE:$script" "$EXTRACT_DIR/scripts/${script_name}" 2>/dev/null || true
    fi
done

# Check common startup locations
for path in /ros_entrypoint.sh /entrypoint.sh /startup.sh /etc/init.d /usr/local/bin; do
    docker exec "$EXTRACT_SOURCE" test -f "$path" 2>/dev/null && docker cp "$EXTRACT_SOURCE:$path" "$EXTRACT_DIR/scripts/" 2>/dev/null || true
    docker exec "$EXTRACT_SOURCE" test -d "$path" 2>/dev/null && docker cp "$EXTRACT_SOURCE:$path" "$EXTRACT_DIR/scripts/$(basename $path)" 2>/dev/null || true
done

# 3. Extract environment variables
echo "[3/6] Extracting environment variables..."
docker exec "$EXTRACT_SOURCE" env | sort > "$EXTRACT_DIR/environment_variables.txt"
docker exec "$EXTRACT_SOURCE" cat /etc/environment 2>/dev/null > "$EXTRACT_DIR/etc_environment.txt" || true
docker exec "$EXTRACT_SOURCE" cat ~/.bashrc 2>/dev/null > "$EXTRACT_DIR/bashrc.txt" || true
docker exec "$EXTRACT_SOURCE" cat ~/.profile 2>/dev/null > "$EXTRACT_DIR/profile.txt" || true

# 4. Extract system dependencies
echo "[4/6] Extracting system dependencies..."
docker exec "$EXTRACT_SOURCE" dpkg -l > "$EXTRACT_DIR/installed_packages.txt"
docker exec "$EXTRACT_SOURCE" pip3 list 2>/dev/null > "$EXTRACT_DIR/pip_packages.txt" || true
docker exec "$EXTRACT_SOURCE" pip list 2>/dev/null > "$EXTRACT_DIR/pip2_packages.txt" || true
docker exec "$EXTRACT_SOURCE" cat /etc/apt/sources.list 2>/dev/null > "$EXTRACT_DIR/apt_sources.txt" || true
docker exec "$EXTRACT_SOURCE" ls -la /etc/apt/sources.list.d/ 2>/dev/null > "$EXTRACT_DIR/apt_sources_list_d.txt" || true

# Extract udev rules
echo "    Extracting udev rules..."
docker exec "$EXTRACT_SOURCE" find /etc/udev/rules.d -type f 2>/dev/null > "$EXTRACT_DIR/udev_rules_list.txt" || true
mkdir -p "$EXTRACT_DIR/udev_rules"
for rule in $(cat "$EXTRACT_DIR/udev_rules_list.txt" 2>/dev/null); do
    if [ -n "$rule" ]; then
        rule_name=$(basename "$rule")
        docker cp "$EXTRACT_SOURCE:$rule" "$EXTRACT_DIR/udev_rules/${rule_name}" 2>/dev/null || true
    fi
done

# 5. Extract ROS2 configurations
echo "[5/6] Extracting ROS2 configurations..."
docker exec "$EXTRACT_SOURCE" find /opt/ros -type f -name "*.yaml" -o -name "*.yml" -o -name "*.launch.py" 2>/dev/null | head -50 > "$EXTRACT_DIR/ros_configs_list.txt" || true
docker exec "$EXTRACT_SOURCE" find ~ -type f -name "*.yaml" -o -name "*.yml" -o -name "*.launch.py" 2>/dev/null | grep -E "(launch|config|param)" | head -50 >> "$EXTRACT_DIR/ros_configs_list.txt" || true

mkdir -p "$EXTRACT_DIR/ros_configs"
for config in $(cat "$EXTRACT_DIR/ros_configs_list.txt" 2>/dev/null | head -30); do
    if [ -n "$config" ]; then
        config_dir=$(dirname "$config" | sed 's|^/||' | tr '/' '_')
        config_name=$(basename "$config")
        mkdir -p "$EXTRACT_DIR/ros_configs/${config_dir}"
        docker cp "$EXTRACT_SOURCE:$config" "$EXTRACT_DIR/ros_configs/${config_dir}/${config_name}" 2>/dev/null || true
    fi
done

# Extract workspace information
docker exec "$EXTRACT_SOURCE" find / -type d -name "*workspace*" -o -name "*ws" -o -name "*ros2*" 2>/dev/null | grep -E "(workspace|ws|ros2)" | head -20 > "$EXTRACT_DIR/workspace_locations.txt" || true

# 6. Extract hardware configurations
echo "[6/6] Extracting hardware configurations..."
docker exec "$EXTRACT_SOURCE" find /etc -type f \( -name "*serial*" -o -name "*camera*" -o -name "*sensor*" -o -name "*device*" \) 2>/dev/null | head -20 > "$EXTRACT_DIR/hardware_configs_list.txt" || true
docker exec "$EXTRACT_SOURCE" ls -la /dev/ 2>/dev/null > "$EXTRACT_DIR/devices_list.txt" || true
docker exec "$EXTRACT_SOURCE" cat /etc/modprobe.d/* 2>/dev/null > "$EXTRACT_DIR/modprobe_configs.txt" || true

mkdir -p "$EXTRACT_DIR/hardware_configs"
for config in $(cat "$EXTRACT_DIR/hardware_configs_list.txt" 2>/dev/null); do
    if [ -n "$config" ]; then
        config_name=$(basename "$config")
        docker cp "$EXTRACT_SOURCE:$config" "$EXTRACT_DIR/hardware_configs/${config_name}" 2>/dev/null || true
    fi
done

# Extract ROS2 workspace structure (if exists in container)
echo "    Extracting ROS2 workspace structure..."
docker exec "$EXTRACT_SOURCE" find / -type d -path "*/install" -o -path "*/src" 2>/dev/null | grep -E "(workspace|ws|ros2)" | head -10 > "$EXTRACT_DIR/workspace_structure.txt" || true

# Create summary report
echo ""
echo "=== Creating extraction summary ==="
cat > "$EXTRACT_DIR/EXTRACTION_SUMMARY.md" << EOF
# ROS2 Foxy Container Extraction Summary

Extracted on: $(date)

## Container/Image Information
- Source: $CONTAINER_NAME
- Type: $([ "$IS_CONTAINER" = true ] && echo "Container" || echo "Image")

## Extracted Components

### 1. Docker Information
- Docker inspect: \`docker_inspect.json\`
- Docker history: \`docker_history.txt\`
- Dockerfile locations: \`dockerfile_locations.txt\`

### 2. Startup Scripts
- Script list: \`startup_scripts_list.txt\`
- Extracted scripts: \`scripts/\`

### 3. Environment Variables
- Environment: \`environment_variables.txt\`
- /etc/environment: \`etc_environment.txt\`
- ~/.bashrc: \`bashrc.txt\`
- ~/.profile: \`profile.txt\`

### 4. System Dependencies
- Installed packages: \`installed_packages.txt\`
- Python packages (pip3): \`pip_packages.txt\`
- Python packages (pip2): \`pip2_packages.txt\`
- APT sources: \`apt_sources.txt\`
- Udev rules: \`udev_rules/\`

### 5. ROS2 Configurations
- Config list: \`ros_configs_list.txt\`
- Extracted configs: \`ros_configs/\`
- Workspace locations: \`workspace_locations.txt\`

### 6. Hardware Configurations
- Hardware config list: \`hardware_configs_list.txt\`
- Device list: \`devices_list.txt\`
- Modprobe configs: \`modprobe_configs.txt\`
- Extracted configs: \`hardware_configs/\`

## Next Steps

1. Review the extracted files
2. Identify which configurations need to be migrated
3. Create new Dockerfile based on ROS2 Humble
4. Migrate configurations to new container
EOF

# Cleanup temporary container
echo ""
echo "Cleaning up temporary container..."
docker stop "$TEMP_CONTAINER" 2>/dev/null || true
docker rm "$TEMP_CONTAINER" 2>/dev/null || true
docker rmi "temp_foxy_extract:latest" 2>/dev/null || true

echo ""
echo "=== Extraction Complete ==="
echo "All extracted files are in: $EXTRACT_DIR"
echo "Review the summary: $EXTRACT_DIR/EXTRACTION_SUMMARY.md"
echo ""
ls -lh "$EXTRACT_DIR"

```

```dockerfile
# Dockerfile for ROS2 Humble on Ubuntu 22.04
# Based on extraction from ROS2 Foxy container
# This will be updated after extracting information from the old Foxy container

FROM ubuntu:22.04

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Set locale
ENV LANG=en_US.UTF-8
ENV LC_ALL=en_US.UTF-8
RUN apt-get update && apt-get install -y \
    locales \
    && locale-gen en_US.UTF-8 \
    && update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8

# Install basic utilities
RUN apt-get update && apt-get install -y \
    curl \
    gnupg2 \
    lsb-release \
    wget \
    git \
    vim \
    nano \
    build-essential \
    cmake \
    python3-pip \
    python3-colcon-common-extensions \
    python3-rosdep \
    python3-vcstool \
    sudo \
    net-tools \
    iputils-ping \
    && rm -rf /var/lib/apt/lists/*

# Setup ROS2 Humble repository
RUN curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.asc | apt-key add - \
    && echo "deb [arch=$(dpkg --print-architecture)] http://packages.ros.org/ros2/ubuntu $(lsb_release -cs) main" > /etc/apt/sources.list.d/ros2-latest.list

# Install ROS2 Humble
RUN apt-get update && apt-get install -y \
    ros-humble-desktop \
    ros-humble-ros-base \
    && rm -rf /var/lib/apt/lists/*

# Install additional ROS2 packages (will be updated based on Foxy extraction)
# Common packages for robotics
RUN apt-get update && apt-get install -y \
    ros-humble-cv-bridge \
    ros-humble-image-transport \
    ros-humble-image-transport-plugins \
    ros-humble-camera-info-manager \
    ros-humble-compressed-image-transport \
    ros-humble-image-geometry \
    ros-humble-pcl-ros \
    ros-humble-pcl-conversions \
    ros-humble-laser-filters \
    ros-humble-robot-localization \
    ros-humble-robot-state-publisher \
    ros-humble-joint-state-publisher \
    ros-humble-xacro \
    ros-humble-urdf \
    ros-humble-tf2-ros \
    ros-humble-tf2-tools \
    ros-humble-nav2-bringup \
    ros-humble-nav2-common \
    ros-humble-slam-toolbox \
    ros-humble-cartographer-ros \
    ros-humble-cartographer \
    && rm -rf /var/lib/apt/lists/*

# Install system dependencies (will be updated based on Foxy extraction)
RUN apt-get update && apt-get install -y \
    libopencv-dev \
    libeigen3-dev \
    libboost-all-dev \
    libusb-1.0-0-dev \
    udev \
    usbutils \
    && rm -rf /var/lib/apt/lists/*

# Initialize rosdep (will be updated with custom rules from Foxy if needed)
RUN rosdep init || true \
    && rosdep update || true

# Create workspace directory (will be mounted from host)
RUN mkdir -p /root/yahboomcar_ros2_ws

# Setup environment
RUN echo "source /opt/ros/humble/setup.bash" >> /root/.bashrc

# Copy entrypoint script (will be created/updated based on Foxy extraction)
COPY docker_entrypoint.sh /ros_entrypoint.sh
RUN chmod +x /ros_entrypoint.sh

# Set working directory
WORKDIR /root

# Default command
ENTRYPOINT ["/ros_entrypoint.sh"]
CMD ["bash"]

```

```shellscript
#!/bin/bash
# ROS2 Humble Container Entrypoint
# This will be updated based on extracted Foxy container startup scripts

set -e

# Source ROS2 Humble
source /opt/ros/humble/setup.bash

# Source workspace if it exists and is built
if [ -f /root/yahboomcar_ros2_ws/install/setup.bash ]; then
    echo "Sourcing ROS2 workspace..."
    source /root/yahboomcar_ros2_ws/install/setup.bash
fi

# Setup device symlinks (similar to ROS1 container)
if [ -e /dev/ttyUSB1 ]; then
    ln -sf /dev/ttyUSB1 /dev/myserial 2>/dev/null || true
fi

if [ -e /dev/ttyUSB0 ]; then
    ln -sf /dev/ttyUSB0 /dev/ydlidar 2>/dev/null || true
fi

# Execute the command passed to the container
exec "$@"

```

```shellscript
#!/bin/bash
# Script to run ROS2 Humble Docker container
# Similar to ~/run_docker.sh for ROS1

# Enable X11 forwarding
xhost +local:docker

# Container name
CONTAINER_NAME="yahboom_ros2_humble"

# Check if container already exists
if docker ps -a --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
    echo "Container $CONTAINER_NAME already exists."
    read -p "Do you want to remove it and create a new one? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo "Stopping and removing existing container..."
        docker stop "$CONTAINER_NAME" 2>/dev/null || true
        docker rm "$CONTAINER_NAME" 2>/dev/null || true
    else
        echo "Starting existing container..."
        docker start "$CONTAINER_NAME"
        docker exec -it "$CONTAINER_NAME" bash
        exit 0
    fi
fi

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORKSPACE_DIR="$SCRIPT_DIR/yahboomcar_ws"

# Check if workspace exists
if [ ! -d "$WORKSPACE_DIR" ]; then
    echo "Error: Workspace directory not found: $WORKSPACE_DIR"
    exit 1
fi

echo "Starting ROS2 Humble container..."
echo "Workspace: $WORKSPACE_DIR"
echo "Container name: $CONTAINER_NAME"

# Run the container
docker run -d \
    --name "$CONTAINER_NAME" \
    --privileged \
    --gpus all \
    --runtime=nvidia \
    --net=host \
    --env="DISPLAY=:0" \
    --env="QT_X11_NO_MITSHM=1" \
    --env="NO_AT_BRIDGE=1" \
    --env="ROS_DOMAIN_ID=${ROS_DOMAIN_ID:-0}" \
    -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
    -v $HOME/.Xauthority:/root/.Xauthority:rw \
    -v "$WORKSPACE_DIR:/root/yahboomcar_ros2_ws:rw" \
    -v /home/jetson/temp:/root/temp:rw \
    -v /dev/bus/usb:/dev/bus/usb:rw \
    --device=/dev/ttyUSB0:/dev/ttyUSB0 \
    --device=/dev/ttyUSB1:/dev/ttyUSB1 \
    --device=/dev/ttyUSB2:/dev/ttyUSB2 \
    --device=/dev/myserial \
    --device=/dev/ydlidar \
    --device=/dev/astro_pro_plus \
    --device=/dev/astro_pro_plus_rgb \
    --device=/dev/input \
    --device=/dev/video0:/dev/video0 \
    --device=/dev/video1:/dev/video1 \
    --device=/dev/video2:/dev/video2 \
    --device=/dev/video3:/dev/video3 \
    -p 9090:9090 \
    -p 8888:8888 \
    -p 6000:6000 \
    yahboom_ros2_humble:latest \
    bash -c "ln -sf /dev/ttyUSB1 /dev/myserial 2>/dev/null || true && \
             ln -sf /dev/ttyUSB0 /dev/ydlidar 2>/dev/null || true && \
             tail -f /dev/null"

if [ $? -eq 0 ]; then
    echo ""
    echo "Container started successfully!"
    echo "To enter the container, run:"
    echo "  docker exec -it $CONTAINER_NAME bash"
    echo ""
    echo "Or use this script with: ./run_docker_ros2.sh"
    echo ""
    # Automatically exec into the container
    sleep 1
    docker exec -it "$CONTAINER_NAME" bash
else
    echo "Failed to start container. Make sure the Docker image 'yahboom_ros2_humble:latest' is built."
    echo "Build it with: docker build -f Dockerfile.ros2.humble -t yahboom_ros2_humble:latest ."
    exit 1
fi

```

```markdown
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

```


Making the scripts executable and creating a helper script to analyze extracted data and update the Dockerfile:


```shellscript
#!/bin/bash
# Script to analyze extracted Foxy container data and update Dockerfile
# This helps automate the migration process

set -e

EXTRACT_DIR="./foxy_container_extract"
DOCKERFILE="Dockerfile.ros2.humble"
ENTRYPOINT="docker_entrypoint.sh"

if [ ! -d "$EXTRACT_DIR" ]; then
    echo "Error: Extraction directory not found: $EXTRACT_DIR"
    echo "Please run extract_foxy_container_info.sh first"
    exit 1
fi

echo "=== Analyzing extracted Foxy container data ==="
echo ""

# Create backup of current Dockerfile
if [ -f "$DOCKERFILE" ]; then
    cp "$DOCKERFILE" "${DOCKERFILE}.backup.$(date +%Y%m%d_%H%M%S)"
    echo "Backed up existing Dockerfile"
fi

# Analyze installed packages and create package list
echo "[1/5] Analyzing installed packages..."
if [ -f "$EXTRACT_DIR/installed_packages.txt" ]; then
    # Extract package names (remove version info for apt install)
    grep "^ii" "$EXTRACT_DIR/installed_packages.txt" | awk '{print $2}' | cut -d: -f1 > /tmp/foxy_packages.txt
    
    # Filter out packages that are likely already in Humble or not needed
    # Focus on development tools, libraries, and ROS packages
    echo "    Found $(wc -l < /tmp/foxy_packages.txt) installed packages"
    
    # Create a list of packages to potentially add
    cat > "$EXTRACT_DIR/recommended_packages.txt" << EOF
# Packages from Foxy container that may need to be added to Humble
# Review and add to Dockerfile as needed

EOF
    grep -E "(lib|dev|tool|ros)" /tmp/foxy_packages.txt | head -50 >> "$EXTRACT_DIR/recommended_packages.txt"
    echo "    Created recommended_packages.txt"
fi

# Analyze Python packages
echo "[2/5] Analyzing Python packages..."
if [ -f "$EXTRACT_DIR/pip_packages.txt" ]; then
    # Extract package names
    grep -v "^Package" "$EXTRACT_DIR/pip_packages.txt" | awk '{print $1}' > /tmp/foxy_pip_packages.txt
    echo "    Found $(wc -l < /tmp/foxy_pip_packages.txt) Python packages"
    
    cat > "$EXTRACT_DIR/recommended_pip_packages.txt" << EOF
# Python packages from Foxy container
# Add to Dockerfile with: RUN pip3 install <package>

EOF
    cat /tmp/foxy_pip_packages.txt >> "$EXTRACT_DIR/recommended_pip_packages.txt"
    echo "    Created recommended_pip_packages.txt"
fi

# Analyze environment variables
echo "[3/5] Analyzing environment variables..."
if [ -f "$EXTRACT_DIR/environment_variables.txt" ]; then
    # Extract ROS-related and custom environment variables
    grep -E "(ROS|PATH|LD_LIBRARY|PYTHON)" "$EXTRACT_DIR/environment_variables.txt" > "$EXTRACT_DIR/important_env_vars.txt"
    echo "    Found ROS and important environment variables"
    echo "    Review: $EXTRACT_DIR/important_env_vars.txt"
fi

# Analyze startup scripts
echo "[4/5] Analyzing startup scripts..."
if [ -d "$EXTRACT_DIR/scripts" ] && [ "$(ls -A $EXTRACT_DIR/scripts 2>/dev/null)" ]; then
    echo "    Found startup scripts in: $EXTRACT_DIR/scripts/"
    echo "    Review these scripts to update docker_entrypoint.sh"
    
    # Try to find common initialization patterns
    for script in "$EXTRACT_DIR/scripts"/*; do
        if [ -f "$script" ] && [ -x "$script" ]; then
            echo "    - $(basename $script)"
        fi
    done
else
    echo "    No startup scripts found"
fi

# Analyze udev rules
echo "[5/5] Analyzing udev rules..."
if [ -d "$EXTRACT_DIR/udev_rules" ] && [ "$(ls -A $EXTRACT_DIR/udev_rules 2>/dev/null)" ]; then
    echo "    Found udev rules:"
    ls -1 "$EXTRACT_DIR/udev_rules/" | while read rule; do
        echo "    - $rule"
    done
    echo "    These should be copied to /etc/udev/rules.d/ in the container"
else
    echo "    No udev rules found"
fi

# Create migration recommendations
cat > "$EXTRACT_DIR/MIGRATION_RECOMMENDATIONS.md" << EOF
# Migration Recommendations

Generated on: $(date)

## 1. Dockerfile Updates

### System Packages
Review and add packages from: \`recommended_packages.txt\`

Common packages to check:
- Development tools
- Hardware libraries (USB, serial, camera)
- ROS2 packages (check if Humble equivalents exist)

### Python Packages
Add to Dockerfile:
\`\`\`dockerfile
RUN pip3 install -r /path/to/requirements.txt
\`\`\`

Or individually from: \`recommended_pip_packages.txt\`

### Udev Rules
Add to Dockerfile:
\`\`\`dockerfile
COPY foxy_container_extract/udev_rules/* /etc/udev/rules.d/
\`\`\`

## 2. Entrypoint Script Updates

Review startup scripts in: \`scripts/\`

Common things to add:
- Device symlink creation
- Service startup
- Environment variable exports
- ROS2 workspace sourcing

## 3. Environment Variables

Review: \`important_env_vars.txt\`

Add to Dockerfile ENV or entrypoint script as needed.

## 4. Hardware Configurations

Review: \`hardware_configs/\`

These may need to be:
- Copied into container
- Mounted as volumes
- Configured at runtime

## 5. ROS2 Configurations

Review: \`ros_configs/\`

These are likely already in your workspace, but check if any need to be in the container image.

## Next Steps

1. Review all recommendation files
2. Update Dockerfile.ros2.humble
3. Update docker_entrypoint.sh
4. Build container
5. Test and verify
EOF

echo ""
echo "=== Analysis Complete ==="
echo ""
echo "Review files:"
echo "  - $EXTRACT_DIR/recommended_packages.txt"
echo "  - $EXTRACT_DIR/recommended_pip_packages.txt"
echo "  - $EXTRACT_DIR/important_env_vars.txt"
echo "  - $EXTRACT_DIR/MIGRATION_RECOMMENDATIONS.md"
echo ""
echo "Next: Update Dockerfile.ros2.humble and docker_entrypoint.sh based on these recommendations"

```

```shellscript
#!/bin/bash
# Script to build the ROS2 Humble Docker container

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

DOCKERFILE="Dockerfile.ros2.humble"
IMAGE_NAME="yahboom_ros2_humble"
IMAGE_TAG="latest"

echo "=== Building ROS2 Humble Docker Container ==="
echo ""

# Check if Dockerfile exists
if [ ! -f "$DOCKERFILE" ]; then
    echo "Error: Dockerfile not found: $DOCKERFILE"
    exit 1
fi

# Check if extraction was done (optional warning)
if [ ! -d "foxy_container_extract" ]; then
    echo "Warning: foxy_container_extract/ directory not found."
    echo "You may want to run extract_foxy_container_info.sh first to customize the build."
    read -p "Continue anyway? (y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

echo "Building Docker image: ${IMAGE_NAME}:${IMAGE_TAG}"
echo "This may take a while..."
echo ""

# Build the image
docker build \
    -f "$DOCKERFILE" \
    -t "${IMAGE_NAME}:${IMAGE_TAG}" \
    .

if [ $? -eq 0 ]; then
    echo ""
    echo "=== Build Successful ==="
    echo "Image: ${IMAGE_NAME}:${IMAGE_TAG}"
    echo ""
    echo "Image size:"
    docker images "${IMAGE_NAME}:${IMAGE_TAG}" --format "table {{.Repository}}\t{{.Tag}}\t{{.Size}}"
    echo ""
    echo "To run the container:"
    echo "  ./run_docker_ros2.sh"
    echo ""
    echo "Or manually:"
    echo "  docker run -it --rm --name ${IMAGE_NAME} ${IMAGE_NAME}:${IMAGE_TAG}"
else
    echo ""
    echo "=== Build Failed ==="
    exit 1
fi

```


Creating a quick-start README and fixing a small issue in the run script:


```shellscript
#!/bin/bash
# Script to run ROS2 Humble Docker container
# Similar to ~/run_docker.sh for ROS1

# Enable X11 forwarding
xhost +local:docker

# Container name
CONTAINER_NAME="yahboom_ros2_humble"

# Check if container already exists
if docker ps -a --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
    echo "Container $CONTAINER_NAME already exists."
    read -p "Do you want to remove it and create a new one? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo "Stopping and removing existing container..."
        docker stop "$CONTAINER_NAME" 2>/dev/null || true
        docker rm "$CONTAINER_NAME" 2>/dev/null || true
    else
        echo "Starting existing container..."
        docker start "$CONTAINER_NAME"
        docker exec -it "$CONTAINER_NAME" bash
        exit 0
    fi
fi

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORKSPACE_DIR="$SCRIPT_DIR/yahboomcar_ws"

# Check if workspace exists
if [ ! -d "$WORKSPACE_DIR" ]; then
    echo "Error: Workspace directory not found: $WORKSPACE_DIR"
    exit 1
fi

echo "Starting ROS2 Humble container..."
echo "Workspace: $WORKSPACE_DIR"
echo "Container name: $CONTAINER_NAME"

# Run the container
docker run -d \
    --name "$CONTAINER_NAME" \
    --privileged \
    --gpus all \
    --runtime=nvidia \
    --net=host \
    --env="DISPLAY=:0" \
    --env="QT_X11_NO_MITSHM=1" \
    --env="NO_AT_BRIDGE=1" \
    --env="ROS_DOMAIN_ID=${ROS_DOMAIN_ID:-0}" \
    -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
    -v $HOME/.Xauthority:/root/.Xauthority:rw \
    -v "$WORKSPACE_DIR:/root/yahboomcar_ros2_ws:rw" \
    -v /home/jetson/temp:/root/temp:rw \
    -v /dev/bus/usb:/dev/bus/usb:rw \
    --device=/dev/ttyUSB0:/dev/ttyUSB0 \
    --device=/dev/ttyUSB1:/dev/ttyUSB1 \
    --device=/dev/ttyUSB2:/dev/ttyUSB2 \
    --device=/dev/myserial \
    --device=/dev/ydlidar \
    --device=/dev/astro_pro_plus \
    --device=/dev/astro_pro_plus_rgb \
    --device=/dev/input \
    --device=/dev/video0:/dev/video0 \
    --device=/dev/video1:/dev/video1 \
    --device=/dev/video2:/dev/video2 \
    --device=/dev/video3:/dev/video3 \
    -p 9090:9090 \
    -p 8888:8888 \
    -p 6000:6000 \
    yahboom_ros2_humble:latest \
    bash -c "ln -sf /dev/ttyUSB1 /dev/myserial 2>/dev/null || true && \
             ln -sf /dev/ttyUSB0 /dev/ydlidar 2>/dev/null || true && \
             tail -f /dev/null" || \
    docker run -d \
        --name "$CONTAINER_NAME" \
        --privileged \
        --gpus all \
        --runtime=nvidia \
        --net=host \
        --env="DISPLAY=:0" \
        --env="QT_X11_NO_MITSHM=1" \
        --env="NO_AT_BRIDGE=1" \
        --env="ROS_DOMAIN_ID=${ROS_DOMAIN_ID:-0}" \
        -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
        -v $HOME/.Xauthority:/root/.Xauthority:rw \
        -v "$WORKSPACE_DIR:/root/yahboomcar_ros2_ws:rw" \
        -v /home/jetson/temp:/root/temp:rw \
        -v /dev/bus/usb:/dev/bus/usb:rw \
        --device=/dev/ttyUSB0:/dev/ttyUSB0 \
        --device=/dev/ttyUSB1:/dev/ttyUSB1 \
        --device=/dev/ttyUSB2:/dev/ttyUSB2 \
        --device=/dev/myserial \
        --device=/dev/ydlidar \
        --device=/dev/astro_pro_plus \
        --device=/dev/astro_pro_plus_rgb \
        --device=/dev/input \
        --device=/dev/video0:/dev/video0 \
        --device=/dev/video1:/dev/video1 \
        --device=/dev/video2:/dev/video2 \
        --device=/dev/video3:/dev/video3 \
        -p 9090:9090 \
        -p 8888:8888 \
        -p 6000:6000 \
        yahboom_ros2_humble:latest \
        bash -c "ln -sf /dev/ttyUSB1 /dev/myserial 2>/dev/null || true && \
                 ln -sf /dev/ttyUSB0 /dev/ydlidar 2>/dev/null || true && \
                 tail -f /dev/null"

if [ $? -eq 0 ]; then
    echo ""
    echo "Container started successfully!"
    echo "To enter the container, run:"
    echo "  docker exec -it $CONTAINER_NAME bash"
    echo ""
    echo "Or use this script with: ./run_docker_ros2.sh"
    echo ""
    # Automatically exec into the container
    sleep 1
    docker exec -it "$CONTAINER_NAME" bash
else
    echo "Failed to start container. Make sure the Docker image 'yahboom_ros2_humble:latest' is built."
    echo "Build it with: docker build -f Dockerfile.ros2.humble -t yahboom_ros2_humble:latest ."
    exit 1
fi


```


Fixing the duplicate docker run command:


```shellscript
#!/bin/bash
# Script to run ROS2 Humble Docker container
# Similar to ~/run_docker.sh for ROS1

# Enable X11 forwarding
xhost +local:docker

# Container name
CONTAINER_NAME="yahboom_ros2_humble"

# Check if container already exists
if docker ps -a --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
    echo "Container $CONTAINER_NAME already exists."
    read -p "Do you want to remove it and create a new one? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo "Stopping and removing existing container..."
        docker stop "$CONTAINER_NAME" 2>/dev/null || true
        docker rm "$CONTAINER_NAME" 2>/dev/null || true
    else
        echo "Starting existing container..."
        docker start "$CONTAINER_NAME"
        docker exec -it "$CONTAINER_NAME" bash
        exit 0
    fi
fi

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORKSPACE_DIR="$SCRIPT_DIR/yahboomcar_ws"

# Check if workspace exists
if [ ! -d "$WORKSPACE_DIR" ]; then
    echo "Error: Workspace directory not found: $WORKSPACE_DIR"
    exit 1
fi

echo "Starting ROS2 Humble container..."
echo "Workspace: $WORKSPACE_DIR"
echo "Container name: $CONTAINER_NAME"

# Run the container
docker run -d \
    --name "$CONTAINER_NAME" \
    --privileged \
    --gpus all \
    --runtime=nvidia \
    --net=host \
    --env="DISPLAY=:0" \
    --env="QT_X11_NO_MITSHM=1" \
    --env="NO_AT_BRIDGE=1" \
    --env="ROS_DOMAIN_ID=${ROS_DOMAIN_ID:-0}" \
    -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
    -v $HOME/.Xauthority:/root/.Xauthority:rw \
    -v "$WORKSPACE_DIR:/root/yahboomcar_ros2_ws:rw" \
    -v /home/jetson/temp:/root/temp:rw \
    -v /dev/bus/usb:/dev/bus/usb:rw \
    --device=/dev/ttyUSB0:/dev/ttyUSB0 \
    --device=/dev/ttyUSB1:/dev/ttyUSB1 \
    --device=/dev/ttyUSB2:/dev/ttyUSB2 \
    --device=/dev/myserial \
    --device=/dev/ydlidar \
    --device=/dev/astro_pro_plus \
    --device=/dev/astro_pro_plus_rgb \
    --device=/dev/input \
    --device=/dev/video0:/dev/video0 \
    --device=/dev/video1:/dev/video1 \
    --device=/dev/video2:/dev/video2 \
    --device=/dev/video3:/dev/video3 \
    -p 9090:9090 \
    -p 8888:8888 \
    -p 6000:6000 \
    yahboom_ros2_humble:latest \
    bash -c "ln -sf /dev/ttyUSB1 /dev/myserial 2>/dev/null || true && \
             ln -sf /dev/ttyUSB0 /dev/ydlidar 2>/dev/null || true && \
             tail -f /dev/null"

if [ $? -eq 0 ]; then
    echo ""
    echo "Container started successfully!"
    echo "To enter the container, run:"
    echo "  docker exec -it $CONTAINER_NAME bash"
    echo ""
    echo "Or use this script with: ./run_docker_ros2.sh"
    echo ""
    # Automatically exec into the container
    sleep 1
    docker exec -it "$CONTAINER_NAME" bash
else
    echo "Failed to start container. Make sure the Docker image 'yahboom_ros2_humble:latest' is built."
    echo "Build it with: docker build -f Dockerfile.ros2.humble -t yahboom_ros2_humble:latest ."
    exit 1
fi


```

```markdown
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
cd /home/jetson/yahboomcar_ros2_ws_new
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

```


Created a Docker setup for migrating from ROS2 Foxy to Humble. Summary:

## Created files

### Extraction and analysis scripts
1. **`extract_foxy_container_info.sh`** — Extracts from the old Foxy container:
   - Dockerfile/image info
   - Startup/entrypoint scripts
   - Environment variables
   - System dependencies (apt packages, pip packages)
   - Udev rules
   - ROS2 configurations
   - Hardware configurations

2. **`analyze_and_update_dockerfile.sh`** — Analyzes extracted data and generates recommendations for updating the Dockerfile

### Docker files
3. **`Dockerfile.ros2.humble`** — Base Dockerfile for ROS2 Humble (Ubuntu 22.04)
   - Includes ROS2 Humble desktop
   - Common robotics packages
   - Ready to update based on Foxy extraction

4. **`docker_entrypoint.sh`** — Container entrypoint script
   - Sources ROS2 Humble
   - Sources workspace if available
   - Creates device symlinks (like ROS1 container)

### Build and run scripts
5. **`build_ros2_container.sh`** — Builds the Docker image
6. **`run_docker_ros2.sh`** — Runs the container with:
   - Workspace mount: `/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws` → `/root/yahboomcar_ros2_ws`
   - All device mappings (USB, serial, cameras, input)
   - GPU support, X11 forwarding, host networking

### Documentation
7. **`MIGRATION_GUIDE.md`** — Step-by-step migration guide
8. **`README_DOCKER_SETUP.md`** — Quick reference

## Next steps

1. Download and load the Foxy container image
2. Extract information:
   ```bash
   cd /home/jetson/yahboomcar_ros2_ws_new
   ./extract_foxy_container_info.sh <foxy_container_name>
   ```
3. Analyze the extracted data:
   ```bash
   ./analyze_and_update_dockerfile.sh
   ```
4. Review recommendations and update:
   - `Dockerfile.ros2.humble` — Add missing packages/dependencies
   - `docker_entrypoint.sh` — Add startup logic from Foxy
5. Build the container:
   ```bash
   ./build_ros2_container.sh
   ```
6. Run the container:
   ```bash
   ./run_docker_ros2.sh
   ```

The workspace is mounted from the host, so your ROS2 code at `/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws` will be accessible in the container at `/root/yahboomcar_ros2_ws`.

All scripts are executable and ready to use. After you download the Foxy container, run the extraction script to begin the migration.

```shellscript
   cd /home/jetson/yahboomcar_ros2_ws_new
   ./extract_foxy_container_info.sh <foxy_container_name>
```

```shellscript
   ./analyze_and_update_dockerfile.sh
```

```shellscript
   ./build_ros2_container.sh
```

```shellscript
   ./run_docker_ros2.sh
```

---

**User**

ypu wil need to keep the port and other mappings needed in mind from this ~/run_docker.sh and also we will be mapping out humble src from the host folder /home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws to this new container so we don't have 2 copies of the code>
Also download is in progress and is a large 82 GB zip (original size is 160+ GB) , so it will tak a while 2 plus hours more. it is downloading to the downloads folder. weill have to mount this iimage once it finsihes downloading

---

**Cursor**


Verifying port and device mappings match the ROS1 script, confirming the workspace mount, and creating a script to load the downloaded image. Checking the ROS1 script for any missing mappings:



Updating the run script to match the ROS1 USB bus mappings and adding a script to load the downloaded image:


```shellscript
#!/bin/bash
# Script to run ROS2 Humble Docker container
# Similar to ~/run_docker.sh for ROS1

# Enable X11 forwarding
xhost +local:docker

# Container name
CONTAINER_NAME="yahboom_ros2_humble"

# Check if container already exists
if docker ps -a --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
    echo "Container $CONTAINER_NAME already exists."
    read -p "Do you want to remove it and create a new one? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo "Stopping and removing existing container..."
        docker stop "$CONTAINER_NAME" 2>/dev/null || true
        docker rm "$CONTAINER_NAME" 2>/dev/null || true
    else
        echo "Starting existing container..."
        docker start "$CONTAINER_NAME"
        docker exec -it "$CONTAINER_NAME" bash
        exit 0
    fi
fi

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORKSPACE_DIR="$SCRIPT_DIR/yahboomcar_ws"

# Check if workspace exists
if [ ! -d "$WORKSPACE_DIR" ]; then
    echo "Error: Workspace directory not found: $WORKSPACE_DIR"
    exit 1
fi

echo "Starting ROS2 Humble container..."
echo "Workspace: $WORKSPACE_DIR"
echo "Container name: $CONTAINER_NAME"

# Run the container
docker run -d \
    --name "$CONTAINER_NAME" \
    --privileged \
    --gpus all \
    --runtime=nvidia \
    --net=host \
    --env="DISPLAY=:0" \
    --env="QT_X11_NO_MITSHM=1" \
    --env="NO_AT_BRIDGE=1" \
    --env="ROS_DOMAIN_ID=${ROS_DOMAIN_ID:-0}" \
    -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
    -v $HOME/.Xauthority:/root/.Xauthority:rw \
    -v "$WORKSPACE_DIR:/root/yahboomcar_ros2_ws:rw" \
    -v /home/jetson/temp:/root/temp:rw \
    -v /dev/bus/usb:/dev/bus/usb:rw \
    -v /dev/bus/usb/001/009:/dev/bus/usb/001/009 \
    -v /dev/bus/usb/001/012:/dev/bus/usb/001/012 \
    -v /dev/bus/usb/001/007:/dev/bus/usb/001/007 \
    --device=/dev/ttyUSB0:/dev/ttyUSB0 \
    --device=/dev/ttyUSB1:/dev/ttyUSB1 \
    --device=/dev/ttyUSB2:/dev/ttyUSB2 \
    --device=/dev/myserial \
    --device=/dev/ydlidar \
    --device=/dev/astro_pro_plus \
    --device=/dev/astro_pro_plus_rgb \
    --device=/dev/input \
    --device=/dev/video0:/dev/video0 \
    --device=/dev/video1:/dev/video1 \
    --device=/dev/video2:/dev/video2 \
    --device=/dev/video3:/dev/video3 \
    -p 9090:9090 \
    -p 8888:8888 \
    -p 6000:6000 \
    yahboom_ros2_humble:latest \
    bash -c "ln -sf /dev/ttyUSB1 /dev/myserial 2>/dev/null || true && \
             ln -sf /dev/ttyUSB0 /dev/ydlidar 2>/dev/null || true && \
             tail -f /dev/null"

if [ $? -eq 0 ]; then
    echo ""
    echo "Container started successfully!"
    echo "To enter the container, run:"
    echo "  docker exec -it $CONTAINER_NAME bash"
    echo ""
    echo "Or use this script with: ./run_docker_ros2.sh"
    echo ""
    # Automatically exec into the container
    sleep 1
    docker exec -it "$CONTAINER_NAME" bash
else
    echo "Failed to start container. Make sure the Docker image 'yahboom_ros2_humble:latest' is built."
    echo "Build it with: docker build -f Dockerfile.ros2.humble -t yahboom_ros2_humble:latest ."
    exit 1
fi


```

```shellscript
#!/bin/bash
# Script to run ROS2 Humble Docker container
# Similar to ~/run_docker.sh for ROS1

# Enable X11 forwarding (matching ROS1 container script)
xhost +

# Container name
CONTAINER_NAME="yahboom_ros2_humble"

# Check if container already exists
if docker ps -a --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
    echo "Container $CONTAINER_NAME already exists."
    read -p "Do you want to remove it and create a new one? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo "Stopping and removing existing container..."
        docker stop "$CONTAINER_NAME" 2>/dev/null || true
        docker rm "$CONTAINER_NAME" 2>/dev/null || true
    else
        echo "Starting existing container..."
        docker start "$CONTAINER_NAME"
        docker exec -it "$CONTAINER_NAME" bash
        exit 0
    fi
fi

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORKSPACE_DIR="$SCRIPT_DIR/yahboomcar_ws"

# Check if workspace exists
if [ ! -d "$WORKSPACE_DIR" ]; then
    echo "Error: Workspace directory not found: $WORKSPACE_DIR"
    exit 1
fi

echo "Starting ROS2 Humble container..."
echo "Workspace: $WORKSPACE_DIR"
echo "Container name: $CONTAINER_NAME"

# Run the container
docker run -d \
    --name "$CONTAINER_NAME" \
    --privileged \
    --gpus all \
    --runtime=nvidia \
    --net=host \
    --env="DISPLAY=:0" \
    --env="QT_X11_NO_MITSHM=1" \
    --env="NO_AT_BRIDGE=1" \
    --env="ROS_DOMAIN_ID=${ROS_DOMAIN_ID:-0}" \
    -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
    -v $HOME/.Xauthority:/root/.Xauthority:rw \
    -v "$WORKSPACE_DIR:/root/yahboomcar_ros2_ws:rw" \
    -v /home/jetson/temp:/root/temp:rw \
    -v /dev/bus/usb:/dev/bus/usb:rw \
    -v /dev/bus/usb/001/009:/dev/bus/usb/001/009 \
    -v /dev/bus/usb/001/012:/dev/bus/usb/001/012 \
    -v /dev/bus/usb/001/007:/dev/bus/usb/001/007 \
    --device=/dev/ttyUSB0:/dev/ttyUSB0 \
    --device=/dev/ttyUSB1:/dev/ttyUSB1 \
    --device=/dev/ttyUSB2:/dev/ttyUSB2 \
    --device=/dev/myserial \
    --device=/dev/ydlidar \
    --device=/dev/astro_pro_plus \
    --device=/dev/astro_pro_plus_rgb \
    --device=/dev/input \
    --device=/dev/video0:/dev/video0 \
    --device=/dev/video1:/dev/video1 \
    --device=/dev/video2:/dev/video2 \
    --device=/dev/video3:/dev/video3 \
    -p 9090:9090 \
    -p 8888:8888 \
    -p 6000:6000 \
    yahboom_ros2_humble:latest \
    bash -c "ln -sf /dev/ttyUSB1 /dev/myserial 2>/dev/null || true && \
             ln -sf /dev/ttyUSB0 /dev/ydlidar 2>/dev/null || true && \
             tail -f /dev/null"

if [ $? -eq 0 ]; then
    echo ""
    echo "Container started successfully!"
    echo "To enter the container, run:"
    echo "  docker exec -it $CONTAINER_NAME bash"
    echo ""
    echo "Or use this script with: ./run_docker_ros2.sh"
    echo ""
    # Automatically exec into the container
    sleep 1
    docker exec -it "$CONTAINER_NAME" bash
else
    echo "Failed to start container. Make sure the Docker image 'yahboom_ros2_humble:latest' is built."
    echo "Build it with: docker build -f Dockerfile.ros2.humble -t yahboom_ros2_humble:latest ."
    exit 1
fi


```

```shellscript
#!/bin/bash
# Script to load the downloaded ROS2 Foxy container image
# Handles both .tar and .tar.gz formats, and extracts from zip if needed

set -e

DOWNLOADS_DIR="$HOME/Downloads"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "=== ROS2 Foxy Container Image Loader ==="
echo ""

# Function to find the downloaded file
find_downloaded_file() {
    # Look for common patterns
    local patterns=(
        "*yahboom*"
        "*rosmaster*"
        "*x3plus*"
        "*foxy*"
        "*ros2*"
        "*docker*"
        "*.tar"
        "*.tar.gz"
        "*.zip"
    )
    
    for pattern in "${patterns[@]}"; do
        local file=$(find "$DOWNLOADS_DIR" -maxdepth 1 -type f -iname "$pattern" 2>/dev/null | head -1)
        if [ -n "$file" ] && [ ! -f "$file.crdownload" ]; then
            echo "$file"
            return 0
        fi
    done
    
    return 1
}

# Check if download is still in progress
check_download_status() {
    local crdownload_files=$(find "$DOWNLOADS_DIR" -maxdepth 1 -name "*.crdownload" 2>/dev/null)
    if [ -n "$crdownload_files" ]; then
        echo "Warning: Download appears to be in progress (.crdownload files found)"
        echo "Files:"
        echo "$crdownload_files"
        echo ""
        read -p "Wait for download to complete? (y/n) " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            echo "Waiting for download to complete..."
            while find "$DOWNLOADS_DIR" -maxdepth 1 -name "*.crdownload" | grep -q .; do
                sleep 30
                echo "Still downloading... ($(date))"
            done
            echo "Download appears to be complete!"
        else
            echo "Exiting. Please run this script again after download completes."
            exit 1
        fi
    fi
}

# Handle zip file extraction
extract_zip_if_needed() {
    local file="$1"
    
    if [[ "$file" == *.zip ]]; then
        echo "Detected ZIP file. Extracting..."
        echo "This may take a while for large files..."
        
        local extract_dir="$SCRIPT_DIR/foxy_image_extract"
        mkdir -p "$extract_dir"
        
        # Check available disk space
        local available_space=$(df "$extract_dir" | tail -1 | awk '{print $4}')
        local file_size=$(stat -f%z "$file" 2>/dev/null || stat -c%s "$file" 2>/dev/null)
        
        echo "File size: $(numfmt --to=iec-i --suffix=B $file_size 2>/dev/null || echo "$file_size bytes")"
        echo "Available space: $(numfmt --to=iec-i --suffix=B $available_space 2>/dev/null || echo "$available_space bytes")"
        
        read -p "Proceed with extraction? (y/n) " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            echo "Extraction cancelled."
            exit 1
        fi
        
        echo "Extracting to: $extract_dir"
        unzip -q "$file" -d "$extract_dir" || {
            echo "Error: Extraction failed. Trying with verbose output..."
            unzip "$file" -d "$extract_dir"
        }
        
        # Find the actual image file in extracted contents
        local image_file=$(find "$extract_dir" -type f \( -name "*.tar" -o -name "*.tar.gz" -o -name "*.img" \) | head -1)
        
        if [ -n "$image_file" ]; then
            echo "Found image file in extracted contents: $image_file"
            echo "$image_file"
            return 0
        else
            echo "Warning: No .tar or .tar.gz file found in extracted contents"
            echo "Contents:"
            ls -lh "$extract_dir" | head -20
            return 1
        fi
    else
        echo "$file"
        return 0
    fi
}

# Main execution
check_download_status

# Find the downloaded file
echo "Searching for downloaded image file in: $DOWNLOADS_DIR"
IMAGE_FILE=$(find_downloaded_file)

if [ -z "$IMAGE_FILE" ]; then
    echo "Error: Could not find downloaded image file."
    echo ""
    echo "Please specify the file path manually:"
    read -p "Enter full path to image file: " IMAGE_FILE
    
    if [ ! -f "$IMAGE_FILE" ]; then
        echo "Error: File not found: $IMAGE_FILE"
        exit 1
    fi
else
    echo "Found: $IMAGE_FILE"
fi

# Check file size
FILE_SIZE=$(stat -f%z "$IMAGE_FILE" 2>/dev/null || stat -c%s "$IMAGE_FILE" 2>/dev/null)
echo "File size: $(numfmt --to=iec-i --suffix=B $FILE_SIZE 2>/dev/null || echo "$FILE_SIZE bytes")"
echo ""

# Handle zip extraction if needed
if [[ "$IMAGE_FILE" == *.zip ]]; then
    IMAGE_FILE=$(extract_zip_if_needed "$IMAGE_FILE")
    if [ -z "$IMAGE_FILE" ] || [ ! -f "$IMAGE_FILE" ]; then
        echo "Error: Could not extract or find image file"
        exit 1
    fi
fi

# Determine file type and load accordingly
echo "Loading Docker image..."
echo ""

if [[ "$IMAGE_FILE" == *.tar.gz ]] || [[ "$IMAGE_FILE" == *.tgz ]]; then
    echo "Loading compressed tar archive..."
    docker load -i "$IMAGE_FILE"
elif [[ "$IMAGE_FILE" == *.tar ]]; then
    echo "Loading tar archive..."
    docker load -i "$IMAGE_FILE"
else
    echo "Warning: Unknown file type. Attempting to load as Docker image..."
    docker load -i "$IMAGE_FILE" || {
        echo "Error: Failed to load image. File may not be a valid Docker image."
        echo "Supported formats: .tar, .tar.gz"
        exit 1
    }
fi

if [ $? -eq 0 ]; then
    echo ""
    echo "=== Image Loaded Successfully ==="
    echo ""
    echo "Available Docker images:"
    docker images | head -10
    echo ""
    echo "Next steps:"
    echo "1. Identify the loaded image name/tag"
    echo "2. Run extraction script:"
    echo "   ./extract_foxy_container_info.sh <image_name_or_container_name>"
    echo ""
    echo "To find the image name, look for the 'Loaded image:' message above,"
    echo "or run: docker images"
else
    echo ""
    echo "=== Failed to Load Image ==="
    exit 1
fi

```


Verifying the workspace mapping and creating a quick reference:


```markdown
# Quick Start Guide - ROS2 Humble Docker Setup

## Current Status

✅ All scripts created and ready  
⏳ Waiting for Foxy container image download (82GB zip, ~160GB uncompressed)

## Once Download Completes

### Step 1: Load the Foxy Image

```bash
cd /home/jetson/yahboomcar_ros2_ws_new
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

```


## Updates made

### 1. Port and device mappings
Updated `run_docker_ros2.sh` to match the ROS1 container:
- Added specific USB bus mappings: `/dev/bus/usb/001/009`, `/dev/bus/usb/001/012`, `/dev/bus/usb/001/007`
- Kept all port mappings: `9090:9090`, `8888:8888`, `6000:6000`
- All device mappings match ROS1 exactly

### 2. Workspace mapping
The workspace is mounted (not copied) from:
- Host: `/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws`
- Container: `/root/yahboomcar_ros2_ws`
- Type: Read-write mount — changes persist on host, no duplication

### 3. Image loader script
Created `load_foxy_image.sh` that:
- Detects the downloaded file in `~/Downloads`
- Handles zip extraction (for large files)
- Checks if download is still in progress (`.crdownload` files)
- Loads the Docker image
- Shows next steps

## Next steps (after download completes)

1. Load the image:
   ```bash
   cd /home/jetson/yahboomcar_ros2_ws_new
   ./load_foxy_image.sh
   ```

2. Extract Foxy container info:
   ```bash
   ./extract_foxy_container_info.sh <image_name>
   ```

3. Continue with the migration process as outlined in the guides.

The workspace mapping ensures your ROS2 Humble code at `/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws` is accessible in the container without duplication. All port and device mappings match your ROS1 container setup.

```shellscript
   cd /home/jetson/yahboomcar_ros2_ws_new
   ./load_foxy_image.sh
```

```shellscript
   ./extract_foxy_container_info.sh <image_name>
```

---

**User**

plz move all these files  to their own folder some where 

---

**Cursor**


Creating a folder for the Docker setup files and moving them there. Updating script paths as needed.



Updating script paths to account for the new location:


```shellscript
#!/bin/bash
# Script to run ROS2 Humble Docker container
# Similar to ~/run_docker.sh for ROS1

# Enable X11 forwarding (matching ROS1 container script)
xhost +

# Container name
CONTAINER_NAME="yahboom_ros2_humble"

# Check if container already exists
if docker ps -a --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
    echo "Container $CONTAINER_NAME already exists."
    read -p "Do you want to remove it and create a new one? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo "Stopping and removing existing container..."
        docker stop "$CONTAINER_NAME" 2>/dev/null || true
        docker rm "$CONTAINER_NAME" 2>/dev/null || true
    else
        echo "Starting existing container..."
        docker start "$CONTAINER_NAME"
        docker exec -it "$CONTAINER_NAME" bash
        exit 0
    fi
fi

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# Workspace is in parent directory
WORKSPACE_DIR="$(dirname "$SCRIPT_DIR")/yahboomcar_ws"

# Check if workspace exists
if [ ! -d "$WORKSPACE_DIR" ]; then
    echo "Error: Workspace directory not found: $WORKSPACE_DIR"
    exit 1
fi

echo "Starting ROS2 Humble container..."
echo "Workspace: $WORKSPACE_DIR"
echo "Container name: $CONTAINER_NAME"

# Run the container
docker run -d \
    --name "$CONTAINER_NAME" \
    --privileged \
    --gpus all \
    --runtime=nvidia \
    --net=host \
    --env="DISPLAY=:0" \
    --env="QT_X11_NO_MITSHM=1" \
    --env="NO_AT_BRIDGE=1" \
    --env="ROS_DOMAIN_ID=${ROS_DOMAIN_ID:-0}" \
    -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
    -v $HOME/.Xauthority:/root/.Xauthority:rw \
    -v "$WORKSPACE_DIR:/root/yahboomcar_ros2_ws:rw" \
    -v /home/jetson/temp:/root/temp:rw \
    -v /dev/bus/usb:/dev/bus/usb:rw \
    -v /dev/bus/usb/001/009:/dev/bus/usb/001/009 \
    -v /dev/bus/usb/001/012:/dev/bus/usb/001/012 \
    -v /dev/bus/usb/001/007:/dev/bus/usb/001/007 \
    --device=/dev/ttyUSB0:/dev/ttyUSB0 \
    --device=/dev/ttyUSB1:/dev/ttyUSB1 \
    --device=/dev/ttyUSB2:/dev/ttyUSB2 \
    --device=/dev/myserial \
    --device=/dev/ydlidar \
    --device=/dev/astro_pro_plus \
    --device=/dev/astro_pro_plus_rgb \
    --device=/dev/input \
    --device=/dev/video0:/dev/video0 \
    --device=/dev/video1:/dev/video1 \
    --device=/dev/video2:/dev/video2 \
    --device=/dev/video3:/dev/video3 \
    -p 9090:9090 \
    -p 8888:8888 \
    -p 6000:6000 \
    yahboom_ros2_humble:latest \
    bash -c "ln -sf /dev/ttyUSB1 /dev/myserial 2>/dev/null || true && \
             ln -sf /dev/ttyUSB0 /dev/ydlidar 2>/dev/null || true && \
             tail -f /dev/null"

if [ $? -eq 0 ]; then
    echo ""
    echo "Container started successfully!"
    echo "To enter the container, run:"
    echo "  docker exec -it $CONTAINER_NAME bash"
    echo ""
    echo "Or use this script with: ./run_docker_ros2.sh"
    echo ""
    # Automatically exec into the container
    sleep 1
    docker exec -it "$CONTAINER_NAME" bash
else
    echo "Failed to start container. Make sure the Docker image 'yahboom_ros2_humble:latest' is built."
    echo "Build it with: docker build -f Dockerfile.ros2.humble -t yahboom_ros2_humble:latest ."
    exit 1
fi


```

```shellscript
#!/bin/bash
# Script to load the downloaded ROS2 Foxy container image
# Handles both .tar and .tar.gz formats, and extracts from zip if needed

set -e

DOWNLOADS_DIR="$HOME/Downloads"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# Extract directory should be in the docker_setup folder
EXTRACT_BASE_DIR="$SCRIPT_DIR"

echo "=== ROS2 Foxy Container Image Loader ==="
echo ""

# Function to find the downloaded file
find_downloaded_file() {
    # Look for common patterns
    local patterns=(
        "*yahboom*"
        "*rosmaster*"
        "*x3plus*"
        "*foxy*"
        "*ros2*"
        "*docker*"
        "*.tar"
        "*.tar.gz"
        "*.zip"
    )
    
    for pattern in "${patterns[@]}"; do
        local file=$(find "$DOWNLOADS_DIR" -maxdepth 1 -type f -iname "$pattern" 2>/dev/null | head -1)
        if [ -n "$file" ] && [ ! -f "$file.crdownload" ]; then
            echo "$file"
            return 0
        fi
    done
    
    return 1
}

# Check if download is still in progress
check_download_status() {
    local crdownload_files=$(find "$DOWNLOADS_DIR" -maxdepth 1 -name "*.crdownload" 2>/dev/null)
    if [ -n "$crdownload_files" ]; then
        echo "Warning: Download appears to be in progress (.crdownload files found)"
        echo "Files:"
        echo "$crdownload_files"
        echo ""
        read -p "Wait for download to complete? (y/n) " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            echo "Waiting for download to complete..."
            while find "$DOWNLOADS_DIR" -maxdepth 1 -name "*.crdownload" | grep -q .; do
                sleep 30
                echo "Still downloading... ($(date))"
            done
            echo "Download appears to be complete!"
        else
            echo "Exiting. Please run this script again after download completes."
            exit 1
        fi
    fi
}

# Handle zip file extraction
extract_zip_if_needed() {
    local file="$1"
    
    if [[ "$file" == *.zip ]]; then
        echo "Detected ZIP file. Extracting..."
        echo "This may take a while for large files..."
        
        local extract_dir="$SCRIPT_DIR/foxy_image_extract"
        mkdir -p "$extract_dir"
        
        # Check available disk space
        local available_space=$(df "$extract_dir" | tail -1 | awk '{print $4}')
        local file_size=$(stat -f%z "$file" 2>/dev/null || stat -c%s "$file" 2>/dev/null)
        
        echo "File size: $(numfmt --to=iec-i --suffix=B $file_size 2>/dev/null || echo "$file_size bytes")"
        echo "Available space: $(numfmt --to=iec-i --suffix=B $available_space 2>/dev/null || echo "$available_space bytes")"
        
        read -p "Proceed with extraction? (y/n) " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            echo "Extraction cancelled."
            exit 1
        fi
        
        echo "Extracting to: $extract_dir"
        unzip -q "$file" -d "$extract_dir" || {
            echo "Error: Extraction failed. Trying with verbose output..."
            unzip "$file" -d "$extract_dir"
        }
        
        # Find the actual image file in extracted contents
        local image_file=$(find "$extract_dir" -type f \( -name "*.tar" -o -name "*.tar.gz" -o -name "*.img" \) | head -1)
        
        if [ -n "$image_file" ]; then
            echo "Found image file in extracted contents: $image_file"
            echo "$image_file"
            return 0
        else
            echo "Warning: No .tar or .tar.gz file found in extracted contents"
            echo "Contents:"
            ls -lh "$extract_dir" | head -20
            return 1
        fi
    else
        echo "$file"
        return 0
    fi
}

# Main execution
check_download_status

# Find the downloaded file
echo "Searching for downloaded image file in: $DOWNLOADS_DIR"
IMAGE_FILE=$(find_downloaded_file)

if [ -z "$IMAGE_FILE" ]; then
    echo "Error: Could not find downloaded image file."
    echo ""
    echo "Please specify the file path manually:"
    read -p "Enter full path to image file: " IMAGE_FILE
    
    if [ ! -f "$IMAGE_FILE" ]; then
        echo "Error: File not found: $IMAGE_FILE"
        exit 1
    fi
else
    echo "Found: $IMAGE_FILE"
fi

# Check file size
FILE_SIZE=$(stat -f%z "$IMAGE_FILE" 2>/dev/null || stat -c%s "$IMAGE_FILE" 2>/dev/null)
echo "File size: $(numfmt --to=iec-i --suffix=B $FILE_SIZE 2>/dev/null || echo "$FILE_SIZE bytes")"
echo ""

# Handle zip extraction if needed
if [[ "$IMAGE_FILE" == *.zip ]]; then
    IMAGE_FILE=$(extract_zip_if_needed "$IMAGE_FILE")
    if [ -z "$IMAGE_FILE" ] || [ ! -f "$IMAGE_FILE" ]; then
        echo "Error: Could not extract or find image file"
        exit 1
    fi
fi

# Determine file type and load accordingly
echo "Loading Docker image..."
echo ""

if [[ "$IMAGE_FILE" == *.tar.gz ]] || [[ "$IMAGE_FILE" == *.tgz ]]; then
    echo "Loading compressed tar archive..."
    docker load -i "$IMAGE_FILE"
elif [[ "$IMAGE_FILE" == *.tar ]]; then
    echo "Loading tar archive..."
    docker load -i "$IMAGE_FILE"
else
    echo "Warning: Unknown file type. Attempting to load as Docker image..."
    docker load -i "$IMAGE_FILE" || {
        echo "Error: Failed to load image. File may not be a valid Docker image."
        echo "Supported formats: .tar, .tar.gz"
        exit 1
    }
fi

if [ $? -eq 0 ]; then
    echo ""
    echo "=== Image Loaded Successfully ==="
    echo ""
    echo "Available Docker images:"
    docker images | head -10
    echo ""
    echo "Next steps:"
    echo "1. Identify the loaded image name/tag"
    echo "2. Run extraction script:"
    echo "   ./extract_foxy_container_info.sh <image_name_or_container_name>"
    echo ""
    echo "To find the image name, look for the 'Loaded image:' message above,"
    echo "or run: docker images"
else
    echo ""
    echo "=== Failed to Load Image ==="
    exit 1
fi


```

```shellscript
#!/bin/bash
# Script to load the downloaded ROS2 Foxy container image
# Handles both .tar and .tar.gz formats, and extracts from zip if needed

set -e

DOWNLOADS_DIR="$HOME/Downloads"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# Extract directory should be in the docker_setup folder
EXTRACT_BASE_DIR="$SCRIPT_DIR"

echo "=== ROS2 Foxy Container Image Loader ==="
echo ""

# Function to find the downloaded file
find_downloaded_file() {
    # Look for common patterns
    local patterns=(
        "*yahboom*"
        "*rosmaster*"
        "*x3plus*"
        "*foxy*"
        "*ros2*"
        "*docker*"
        "*.tar"
        "*.tar.gz"
        "*.zip"
    )
    
    for pattern in "${patterns[@]}"; do
        local file=$(find "$DOWNLOADS_DIR" -maxdepth 1 -type f -iname "$pattern" 2>/dev/null | head -1)
        if [ -n "$file" ] && [ ! -f "$file.crdownload" ]; then
            echo "$file"
            return 0
        fi
    done
    
    return 1
}

# Check if download is still in progress
check_download_status() {
    local crdownload_files=$(find "$DOWNLOADS_DIR" -maxdepth 1 -name "*.crdownload" 2>/dev/null)
    if [ -n "$crdownload_files" ]; then
        echo "Warning: Download appears to be in progress (.crdownload files found)"
        echo "Files:"
        echo "$crdownload_files"
        echo ""
        read -p "Wait for download to complete? (y/n) " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            echo "Waiting for download to complete..."
            while find "$DOWNLOADS_DIR" -maxdepth 1 -name "*.crdownload" | grep -q .; do
                sleep 30
                echo "Still downloading... ($(date))"
            done
            echo "Download appears to be complete!"
        else
            echo "Exiting. Please run this script again after download completes."
            exit 1
        fi
    fi
}

# Handle zip file extraction
extract_zip_if_needed() {
    local file="$1"
    
    if [[ "$file" == *.zip ]]; then
        echo "Detected ZIP file. Extracting..."
        echo "This may take a while for large files..."
        
        local extract_dir="$EXTRACT_BASE_DIR/foxy_image_extract"
        mkdir -p "$extract_dir"
        
        # Check available disk space
        local available_space=$(df "$extract_dir" | tail -1 | awk '{print $4}')
        local file_size=$(stat -f%z "$file" 2>/dev/null || stat -c%s "$file" 2>/dev/null)
        
        echo "File size: $(numfmt --to=iec-i --suffix=B $file_size 2>/dev/null || echo "$file_size bytes")"
        echo "Available space: $(numfmt --to=iec-i --suffix=B $available_space 2>/dev/null || echo "$available_space bytes")"
        
        read -p "Proceed with extraction? (y/n) " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            echo "Extraction cancelled."
            exit 1
        fi
        
        echo "Extracting to: $extract_dir"
        unzip -q "$file" -d "$extract_dir" || {
            echo "Error: Extraction failed. Trying with verbose output..."
            unzip "$file" -d "$extract_dir"
        }
        
        # Find the actual image file in extracted contents
        local image_file=$(find "$extract_dir" -type f \( -name "*.tar" -o -name "*.tar.gz" -o -name "*.img" \) | head -1)
        
        if [ -n "$image_file" ]; then
            echo "Found image file in extracted contents: $image_file"
            echo "$image_file"
            return 0
        else
            echo "Warning: No .tar or .tar.gz file found in extracted contents"
            echo "Contents:"
            ls -lh "$extract_dir" | head -20
            return 1
        fi
    else
        echo "$file"
        return 0
    fi
}

# Main execution
check_download_status

# Find the downloaded file
echo "Searching for downloaded image file in: $DOWNLOADS_DIR"
IMAGE_FILE=$(find_downloaded_file)

if [ -z "$IMAGE_FILE" ]; then
    echo "Error: Could not find downloaded image file."
    echo ""
    echo "Please specify the file path manually:"
    read -p "Enter full path to image file: " IMAGE_FILE
    
    if [ ! -f "$IMAGE_FILE" ]; then
        echo "Error: File not found: $IMAGE_FILE"
        exit 1
    fi
else
    echo "Found: $IMAGE_FILE"
fi

# Check file size
FILE_SIZE=$(stat -f%z "$IMAGE_FILE" 2>/dev/null || stat -c%s "$IMAGE_FILE" 2>/dev/null)
echo "File size: $(numfmt --to=iec-i --suffix=B $FILE_SIZE 2>/dev/null || echo "$FILE_SIZE bytes")"
echo ""

# Handle zip extraction if needed
if [[ "$IMAGE_FILE" == *.zip ]]; then
    IMAGE_FILE=$(extract_zip_if_needed "$IMAGE_FILE")
    if [ -z "$IMAGE_FILE" ] || [ ! -f "$IMAGE_FILE" ]; then
        echo "Error: Could not extract or find image file"
        exit 1
    fi
fi

# Determine file type and load accordingly
echo "Loading Docker image..."
echo ""

if [[ "$IMAGE_FILE" == *.tar.gz ]] || [[ "$IMAGE_FILE" == *.tgz ]]; then
    echo "Loading compressed tar archive..."
    docker load -i "$IMAGE_FILE"
elif [[ "$IMAGE_FILE" == *.tar ]]; then
    echo "Loading tar archive..."
    docker load -i "$IMAGE_FILE"
else
    echo "Warning: Unknown file type. Attempting to load as Docker image..."
    docker load -i "$IMAGE_FILE" || {
        echo "Error: Failed to load image. File may not be a valid Docker image."
        echo "Supported formats: .tar, .tar.gz"
        exit 1
    }
fi

if [ $? -eq 0 ]; then
    echo ""
    echo "=== Image Loaded Successfully ==="
    echo ""
    echo "Available Docker images:"
    docker images | head -10
    echo ""
    echo "Next steps:"
    echo "1. Identify the loaded image name/tag"
    echo "2. Run extraction script:"
    echo "   ./extract_foxy_container_info.sh <image_name_or_container_name>"
    echo ""
    echo "To find the image name, look for the 'Loaded image:' message above,"
    echo "or run: docker images"
else
    echo ""
    echo "=== Failed to Load Image ==="
    exit 1
fi


```

```markdown
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

```


Verifying the workspace path update:


```markdown
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


```

```markdown
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


```


All Docker setup files have been moved to `/home/jetson/yahboomcar_ros2_ws_new/docker_setup/`. Summary:

## Files moved to `docker_setup/`

### Scripts (executable)
- `load_foxy_image.sh` - Load downloaded Foxy image
- `extract_foxy_container_info.sh` - Extract Foxy container info
- `analyze_and_update_dockerfile.sh` - Analyze extracted data
- `build_ros2_container.sh` - Build ROS2 Humble container
- `run_docker_ros2.sh` - Run the container

### Docker files
- `Dockerfile.ros2.humble` - Dockerfile for ROS2 Humble
- `docker_entrypoint.sh` - Container entrypoint script

### Documentation
- `README.md` - Main README for the docker_setup folder
- `QUICK_START.md` - Quick start guide
- `MIGRATION_GUIDE.md` - Detailed migration guide
- `README_DOCKER_SETUP.md` - Complete setup documentation

## Paths updated

- `run_docker_ros2.sh` now correctly points to `../yahboomcar_ws` (parent directory)
- Workspace path verified: `/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws`
- All scripts use relative paths that work from the `docker_setup/` directory

## Usage

All scripts should be run from the `docker_setup/` directory:

```bash
cd /home/jetson/yahboomcar_ros2_ws_new/docker_setup
./load_foxy_image.sh
./extract_foxy_container_info.sh <image_name>
# etc.
```

All files are organized and ready to use.

```shellscript
cd /home/jetson/yahboomcar_ros2_ws_new/docker_setup
./load_foxy_image.sh
./extract_foxy_container_info.sh <image_name>
# etc.
```

