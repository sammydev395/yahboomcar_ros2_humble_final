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

