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

