#!/bin/bash
# Extract information directly from mounted overlay filesystem
# Run this after mounting the .img file

set -e

OVERLAY_DIR="/mnt/foxy_image/var/lib/docker/overlay2/0f11ca337f9f88f0914940ad0a25e76796117b10ae6dab740c5841580ea53d3d/diff"
EXTRACT_DIR="./foxy_container_extract"
mkdir -p "$EXTRACT_DIR"

echo "=== Extracting information from mounted ROS2 Foxy overlay ==="
echo ""

if ! sudo test -d "$OVERLAY_DIR"; then
    echo "Error: Overlay directory not found: $OVERLAY_DIR"
    echo "Make sure the image is mounted at /mnt/foxy_image"
    exit 1
fi

echo "Extracting from: $OVERLAY_DIR"
echo ""

# 1. Extract Dockerfile
echo "[1/6] Extracting Dockerfile..."
DOCKERFILE_PATH=$(sudo find "$OVERLAY_DIR" -name "Dockerfile" -type f 2>/dev/null | head -1)
if [ -n "$DOCKERFILE_PATH" ]; then
    sudo cat "$DOCKERFILE_PATH" > "$EXTRACT_DIR/Dockerfile"
    echo "    ✓ Dockerfile extracted from: $DOCKERFILE_PATH"
else
    echo "    ✗ Dockerfile not found"
fi

# 2. Extract entrypoint/startup scripts
echo "[2/6] Extracting entrypoint and startup scripts..."
mkdir -p "$EXTRACT_DIR/scripts"

if [ -f "$OVERLAY_DIR/ros_entrypoint.sh" ]; then
    sudo cp "$OVERLAY_DIR/ros_entrypoint.sh" "$EXTRACT_DIR/scripts/ros_entrypoint.sh" 2>/dev/null || \
    sudo cat "$OVERLAY_DIR/ros_entrypoint.sh" > "$EXTRACT_DIR/scripts/ros_entrypoint.sh"
    echo "    ✓ ros_entrypoint.sh extracted"
fi

# Find other startup scripts
sudo find "$OVERLAY_DIR" -type f -name "*entrypoint*" -o -name "*startup*" -o -name "*init*" 2>/dev/null | \
    grep -E "(entrypoint|startup|init|\.sh)" | while read script; do
    script_name=$(basename "$script")
    sudo cp "$script" "$EXTRACT_DIR/scripts/${script_name}" 2>/dev/null || true
done

# Check common locations
for path in /ros_entrypoint.sh /entrypoint.sh /startup.sh /etc/init.d /usr/local/bin; do
    if [ -f "$OVERLAY_DIR$path" ] || [ -d "$OVERLAY_DIR$path" ]; then
        sudo cp -r "$OVERLAY_DIR$path" "$EXTRACT_DIR/scripts/$(basename $path)" 2>/dev/null || true
    fi
done

echo "    Found $(ls -1 "$EXTRACT_DIR/scripts/" 2>/dev/null | wc -l) scripts"

# 3. Extract environment variables
echo "[3/6] Extracting environment variables..."
sudo cat "$OVERLAY_DIR/etc/environment" 2>/dev/null > "$EXTRACT_DIR/etc_environment.txt" || true
sudo cat "$OVERLAY_DIR/root/.bashrc" 2>/dev/null > "$EXTRACT_DIR/bashrc.txt" || true
sudo cat "$OVERLAY_DIR/root/.profile" 2>/dev/null > "$EXTRACT_DIR/profile.txt" || true

# Extract from .bashrc
if [ -f "$EXTRACT_DIR/bashrc.txt" ]; then
    grep -E "^(export|ENV)" "$EXTRACT_DIR/bashrc.txt" > "$EXTRACT_DIR/environment_variables.txt" 2>/dev/null || true
    echo "    ✓ Environment files extracted"
fi

# 4. Extract system dependencies
echo "[4/6] Extracting system dependencies..."
sudo dpkg -l > "$EXTRACT_DIR/installed_packages.txt" 2>/dev/null || \
    sudo cat "$OVERLAY_DIR/var/lib/dpkg/status" 2>/dev/null | grep "^Package:" | sed 's/^Package: //' > "$EXTRACT_DIR/installed_packages.txt" || true

sudo pip3 list 2>/dev/null > "$EXTRACT_DIR/pip_packages.txt" || \
    sudo cat "$OVERLAY_DIR/usr/local/lib/python3.8/dist-packages"/*/METADATA 2>/dev/null | grep "^Name:" | sed 's/^Name: //' > "$EXTRACT_DIR/pip_packages.txt" || true

sudo cat "$OVERLAY_DIR/etc/apt/sources.list" 2>/dev/null > "$EXTRACT_DIR/apt_sources.txt" || true
sudo ls -la "$OVERLAY_DIR/etc/apt/sources.list.d/" 2>/dev/null > "$EXTRACT_DIR/apt_sources_list_d.txt" || true

# Extract udev rules
echo "    Extracting udev rules..."
sudo find "$OVERLAY_DIR/etc/udev/rules.d" -type f 2>/dev/null > "$EXTRACT_DIR/udev_rules_list.txt" || true
mkdir -p "$EXTRACT_DIR/udev_rules"
for rule in $(cat "$EXTRACT_DIR/udev_rules_list.txt" 2>/dev/null); do
    if [ -n "$rule" ]; then
        rule_name=$(basename "$rule")
        sudo cp "$rule" "$EXTRACT_DIR/udev_rules/${rule_name}" 2>/dev/null || true
    fi
done

# 5. Extract ROS2 configurations
echo "[5/6] Extracting ROS2 configurations..."
sudo find "$OVERLAY_DIR/opt/ros" -type f \( -name "*.yaml" -o -name "*.yml" -o -name "*.launch.py" \) 2>/dev/null | head -50 > "$EXTRACT_DIR/ros_configs_list.txt" || true
sudo find "$OVERLAY_DIR/root" -type f \( -name "*.yaml" -o -name "*.yml" -o -name "*.launch.py" \) 2>/dev/null | grep -E "(launch|config|param)" | head -50 >> "$EXTRACT_DIR/ros_configs_list.txt" || true

mkdir -p "$EXTRACT_DIR/ros_configs"
for config in $(cat "$EXTRACT_DIR/ros_configs_list.txt" 2>/dev/null | head -30); do
    if [ -n "$config" ]; then
        config_dir=$(dirname "$config" | sed "s|^$OVERLAY_DIR||" | sed 's|^/||' | tr '/' '_')
        config_name=$(basename "$config")
        mkdir -p "$EXTRACT_DIR/ros_configs/${config_dir}"
        sudo cp "$config" "$EXTRACT_DIR/ros_configs/${config_dir}/${config_name}" 2>/dev/null || true
    fi
done

# Extract workspace information
sudo find "$OVERLAY_DIR" -type d -name "*workspace*" -o -name "*ws" -o -name "*ros2*" 2>/dev/null | grep -E "(workspace|ws|ros2)" | head -20 > "$EXTRACT_DIR/workspace_locations.txt" || true

# 6. Extract hardware configurations
echo "[6/6] Extracting hardware configurations..."
sudo find "$OVERLAY_DIR/etc" -type f \( -name "*serial*" -o -name "*camera*" -o -name "*sensor*" -o -name "*device*" \) 2>/dev/null | head -20 > "$EXTRACT_DIR/hardware_configs_list.txt" || true
sudo ls -la "$OVERLAY_DIR/dev/" 2>/dev/null > "$EXTRACT_DIR/devices_list.txt" || true
sudo cat "$OVERLAY_DIR/etc/modprobe.d/"* 2>/dev/null > "$EXTRACT_DIR/modprobe_configs.txt" || true

mkdir -p "$EXTRACT_DIR/hardware_configs"
for config in $(cat "$EXTRACT_DIR/hardware_configs_list.txt" 2>/dev/null); do
    if [ -n "$config" ]; then
        config_name=$(basename "$config")
        sudo cp "$config" "$EXTRACT_DIR/hardware_configs/${config_name}" 2>/dev/null || true
    fi
done

# Create summary report
echo ""
echo "=== Creating extraction summary ==="
cat > "$EXTRACT_DIR/EXTRACTION_SUMMARY.md" << EOF
# ROS2 Foxy Container Extraction Summary

Extracted on: $(date)

## Source Information
- Image: yahboomtechnology/ros-foxy-orbslam2:1.0.0
- Overlay: 0f11ca337f9f88f0914940ad0a25e76796117b10ae6dab740c5841580ea53d3d
- Source: Mounted filesystem at /mnt/foxy_image

## Extracted Components

### 1. Docker Information
- Dockerfile: \`Dockerfile\`

### 2. Startup Scripts
- Scripts: \`scripts/\`
- Entrypoint: \`scripts/ros_entrypoint.sh\`

### 3. Environment Variables
- /etc/environment: \`etc_environment.txt\`
- ~/.bashrc: \`bashrc.txt\`
- ~/.profile: \`profile.txt\`
- Environment variables: \`environment_variables.txt\`

### 4. System Dependencies
- Installed packages: \`installed_packages.txt\`
- Python packages: \`pip_packages.txt\`
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
2. Run: \`./analyze_and_update_dockerfile.sh\`
3. Update Dockerfile.ros2.humble based on recommendations
4. Build and test the new Humble container
EOF

echo ""
echo "=== Extraction Complete ==="
echo "All extracted files are in: $EXTRACT_DIR"
echo "Review the summary: $EXTRACT_DIR/EXTRACTION_SUMMARY.md"
echo ""
ls -lh "$EXTRACT_DIR"

