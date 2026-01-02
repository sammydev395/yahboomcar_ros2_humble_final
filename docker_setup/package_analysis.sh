#!/bin/bash
echo "=== Foxy Container Packages ==="
sudo find /mnt/foxy_image/var/lib/docker/overlay2/0f11ca337f9f88f0914940ad0a25e76796117b10ae6dab740c5841580ea53d3d/diff/root/yahboomcar_ros2_ws/src -maxdepth 1 -type d 2>/dev/null | sed 's|.*/||' | sort

echo ""
echo "=== Humble Workspace Packages ==="
ls -1 /home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws/src/ | sort

echo ""
echo "=== ARM packages in Humble (NOT in Foxy) ==="
for pkg in arm_autopilot arm_color_transport arm_mediapipe arm_moveit_demo; do
    if ! sudo test -d /mnt/foxy_image/var/lib/docker/overlay2/0f11ca337f9f88f0914940ad0a25e76796117b10ae6dab740c5841580ea53d3d/diff/root/yahboomcar_ros2_ws/src/$pkg 2>/dev/null; then
        echo "  ❌ $pkg - NOT in Foxy"
    else
        echo "  ✅ $pkg - Found in Foxy"
    fi
done

echo ""
echo "=== Checking specific packages ==="
for pkg in garbage_identify_yolov11 x3plus_moveit_config; do
    if [ -d "/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws/src/$pkg" ]; then
        echo "  ✅ $pkg - EXISTS in Humble workspace"
        if sudo test -d /mnt/foxy_image/var/lib/docker/overlay2/0f11ca337f9f88f0914940ad0a25e76796117b10ae6dab740c5841580ea53d3d/diff/root/yahboomcar_ros2_ws/src/$pkg 2>/dev/null; then
            echo "     ✅ Also in Foxy container"
        else
            echo "     ❌ NOT in Foxy container"
        fi
    else
        echo "  ❌ $pkg - NOT in Humble workspace"
    fi
done
