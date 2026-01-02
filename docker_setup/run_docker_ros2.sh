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
    -v /home/jetson/yahboomcar_ros2_ws/software/library_ws:/root/library_ws:rw \
    -v /home/jetson/yahboomcar_ros2_ws_new/software:/root/software:rw \
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
                     ln -sf /dev/ttyUSB2 /dev/myspeech 2>/dev/null || true && \
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

