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

