#!/bin/bash
# yahboom_ros2_humble container entrypoint
#
# Replaces the old in-image /root/entrypoint_with_cameras.sh which:
#   - hardcoded /dev/ttyUSB1 → /dev/myserial (now devpath-pinned host-side)
#   - launched usb_cam (replaced by OAK-D Lite)
#   - sourced library_ws/install/setup.bash before web_video_server
#     (breaks the binary; see memory/project_web_video_server_clean_shell_required.md)
#
# Responsibilities in priority order:
#   1. Source /opt/ros/humble — base ROS env.
#   2. Source overlay (/opt/ros/humble_overlays/install) — astra_camera +
#      ydlidar driver built from source by the Dockerfile.
#   3. Source workspace (/root/yahboomcar_ros2_ws/install) IF it's built.
#      (When BAKE_WORKSPACE=false at build time, this is the bind-mounted
#       host workspace; first-time users still need to colcon build.)
#   4. Set fleet env (ROS_DOMAIN_ID=100, CycloneDDS).
#   5. Defensive /dev symlinks — only if udev hasn't already created
#      them. We don't fight the host's udev rules.
#   6. exec "$@"  — passes through CMD (default: tail -f /dev/null).
#
# This entrypoint runs ONCE at container start. It does NOT launch
# cameras or master.launch.py — that's the job of the host-side
# start_ros2_nodes.sh + ros2-camera-watcher.service. Keeping launch
# concerns OUT of the entrypoint lets the same image run with the
# user-managed launch (production) or with manual launches (debug)
# without modification.

set -e

# ─── 1. Base ROS ──────────────────────────────────────────────────────
source /opt/ros/humble/setup.bash

# ─── 2. In-image overlay (astra + ydlidar built from source) ─────────
if [ -f /opt/ros/humble_overlays/install/setup.bash ]; then
    source /opt/ros/humble_overlays/install/setup.bash
fi

# ─── 3. User workspace (bind-mounted at /root/yahboomcar_ros2_ws) ────
# Source ONLY if built. Don't fail if it isn't — first-time setup may
# need a manual `colcon build` before this fires for real.
if [ -f /root/yahboomcar_ros2_ws/install/setup.bash ]; then
    source /root/yahboomcar_ros2_ws/install/setup.bash
fi

# ─── 4. Fleet env ────────────────────────────────────────────────────
export ROS_DOMAIN_ID="${ROS_DOMAIN_ID:-100}"
export RMW_IMPLEMENTATION="${RMW_IMPLEMENTATION:-rmw_cyclonedds_cpp}"
if [ -f /etc/cyclonedds/config.xml ]; then
    export CYCLONEDDS_URI="${CYCLONEDDS_URI:-file:///etc/cyclonedds/config.xml}"
fi

# ─── 5. STM32 + lidar /dev symlinks (PROBE-based, not guesswork) ─────
# The STM32 chassis controller AND the CI1302 voice module are BOTH
# CH340 1a86:7523 with no unique serial. The host udev rule pins
# /dev/yahboom_stm32 by devpath (2.4.4) — but that symlink is NOT
# visible inside the container: run_container.sh passes the raw
# `--device=/dev/ttyUSB*` nodes, not the host's symlinks, and the
# container has its own devtmpfs. A blind `ln -sf /dev/ttyUSB1
# /dev/myserial` lands on the voice module whenever enumeration differs
# (every container recreate / USB move) → smoke_serial 0 bytes →
# looks like a dead STM32. (memory project_yahboom_dual_ch340_devpath;
# hit again 2026-05-16 right after the container rebuild.)
#
# Robust fix: probe each ttyUSB with the STM32 AUTO_REPORT-enable frame.
# Only the STM32 streams telemetry back (~hundreds of bytes in ~1 s);
# the voice module and the CP210x lidar do not. Devpath-independent —
# survives recreates AND cable moves.
detect_stm32() {
    local d
    for d in /dev/ttyUSB*; do
        [ -e "$d" ] || continue
        if python3 - "$d" <<'PY' 2>/dev/null
import sys, serial, time
try:
    s = serial.Serial(sys.argv[1], 115200, timeout=0.3)
except Exception:
    sys.exit(1)
time.sleep(0.3)
s.write(bytes([0xFF, 0xFC, 0x05, 0x01, 0x01, 0x00, 0x07]))  # AUTO_REPORT enable
time.sleep(1.2)
n = len(s.read(300))
s.close()
sys.exit(0 if n >= 50 else 1)
PY
        then
            echo "$d"; return 0
        fi
    done
    return 1
}

STM32_DEV="$(detect_stm32 || true)"
if [ -n "${STM32_DEV:-}" ]; then
    ln -sf "$STM32_DEV" /dev/myserial 2>/dev/null || true
    ln -sf "$STM32_DEV" /dev/yahboom_stm32 2>/dev/null || true
    echo "[entrypoint] STM32 detected on $STM32_DEV → /dev/myserial, /dev/yahboom_stm32"
else
    echo "[entrypoint] WARN: no STM32 found by AUTO_REPORT probe among /dev/ttyUSB*." >&2
    echo "[entrypoint]       Leaving /dev/myserial as-is. Check USB / power." >&2
fi

# YDLidar is the lone CP210x (10c4:ea60) — no ambiguity. Prefer the
# host devpath-pinned symlink if it made it through; else the CP210x.
if [ ! -e /dev/ydlidar ]; then
    for d in /dev/ttyUSB*; do
        [ -e "$d" ] || continue
        if [ "$(udevadm info -q property -n "$d" 2>/dev/null | sed -n 's/^ID_VENDOR_ID=//p')" = "10c4" ]; then
            ln -sf "$d" /dev/ydlidar 2>/dev/null || true
            break
        fi
    done
    [ -e /dev/ydlidar ] || ln -sf /dev/ttyUSB0 /dev/ydlidar 2>/dev/null || true
fi

# Gamepad: host udev creates /dev/yahboom_joy → input/jsN, but that
# symlink is NOT visible in the container (own devtmpfs; only the raw
# /dev/input/js* nodes come through via --device=/dev/input). The
# vendor's old container CMD created this symlink; the rewritten
# entrypoint must too or vendor configs / operators expecting
# /dev/yahboom_joy break. (joy_node in our launches opens by index
# device_id:=0 so it works regardless, but keep the symlink for
# parity.) Single Yahboom 2.4 GHz dongle → unambiguous, no probe.
if [ ! -e /dev/yahboom_joy ]; then
    for j in /dev/input/js0 /dev/input/js1 /dev/input/js2; do
        [ -e "$j" ] && { ln -sf "$j" /dev/yahboom_joy 2>/dev/null || true; break; }
    done
fi

# ─── 6. Exec CMD ──────────────────────────────────────────────────────
exec "$@"
