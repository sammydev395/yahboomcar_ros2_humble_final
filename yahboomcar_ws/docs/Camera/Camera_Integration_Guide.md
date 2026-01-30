# Camera Integration Guide - DrDev LumiAI

**Company:** DrDev  
**Product:** LumiAI Intelligent Vision Robotic Arm

Complete guide for camera selection, calibration, integration, and YOLOv8 AI vision across all product tiers.

---

## Table of Contents

1. [Camera Options & Comparison](#camera-options--comparison)
2. [Camera Selection Strategy](#camera-selection-strategy)
3. [ROSmaster X3 Plus: Dual-Camera and Arm Camera Options](#rosmaster-x3-plus-dual-camera-and-arm-camera-options)
4. [OAK-D Integration](#oak-d-integration)
5. [Arducam Integration](#arducam-integration)
6. [Aurora 930 Pro Integration (Ultra Only)](#aurora-930-pro-integration-ultra-only)
7. [Weight Budget Analysis](#weight-budget-analysis)
8. [YOLOv8 Integration](#yolov8-integration)
9. [Product Tier Specifications](#product-tier-specifications)

---

## Camera Options & Comparison

### Available Cameras

**Note:** At any given time, a LumiAI arm will have ONLY ONE camera installed. Product tiers differ by which camera is included.

| Camera | Interface | Resolution | FPS | Depth | AI Chip | Weight | Cost | Status | Link |
|--------|-----------|------------|-----|-------|---------|--------|------|--------|------|
| **Arducam CSI (IMX477 HQ B0240)** | CSI | 12MP (4056×3040) | 30 | ❌ No | ❌ No | ~30g+ | ~$50 | ✅ **In Use (Entry)** | [Arducam B0240](https://www.arducam.com/b0240-arducam-imx477-hq-quality-camera.html) |
| **OAK-D Lite** | USB 3.0 | 1080p/4K | 30 | ✅ Yes | ✅ Myriad X | 60g | $150 | ✅ **In Use (Mid)** | [Waveshare](https://www.waveshare.com/oak-d-lite.htm) |
| **OAK-D S2** | USB 3.0 | 1080p/4K | 30 | ✅ Yes | ✅ Myriad X + IR | 65g | $249 | ✅ **In Use (Premium)** | [Waveshare](https://www.waveshare.com/oak-d-s2.htm) |
| **Aurora 930 Pro** | USB 2.0 | 640x400 | 12 | ✅ Yes | ❌ No | ~65g | $200 | ⚠️ Ultra only (not FPV) | [Hiwonder](https://www.hiwonder.com/products/aurora930-pro) |
| **Arducam PiNSIGHT 12MP** | USB 3.0 | 12.3MP (4000×3000) | 30 | ❌ No | ✅ Luxonis OAK-SoM (Myriad X) | ~60g (module) | $149 | 📦 RPi 5 only (doc'd) | [Product](https://www.arducam.com/presalesarducam-pinsight-12mp-vision-ai-mate-for-raspberry-pi-5.html) · [Docs](https://docs.arducam.com/Raspberry-Pi-Camera/Arducam-PiVistation/Vision-AI-Kit/Arducam-PiNSight/#streaming-example) |

### Detailed Comparison

#### Arducam CSI (IMX477 HQ — B0240)

**Note:** Arducam offers multiple CSI modules. The **B0240 IMX477 HQ** ([product link](https://www.arducam.com/b0240-arducam-imx477-hq-quality-camera.html)) is a high-quality 12MP module; other variants (e.g. IMX519-based) may have different resolution/weight. Always check the specific product SKU.

**Pros:**
- ✅ High resolution (12MP, 4056×3040) — way beyond 720p
- ✅ Low latency (CSI direct to SoC)
- ✅ Good for color-based tracking and detail
- ✅ Direct RPi/Jetson CSI integration

**Cons:**
- ❌ No depth sensing
- ❌ No onboard AI
- ❌ Heavier than minimal USB cameras (~30g+ for HQ module; exact weight see product)
- ❌ Basic night performance

**Use Case:** Entry/mid-tier when high-resolution RGB from CSI is needed; check weight for arm mounting.

---

#### OAK-D Lite
**Pros:**
- ✅ Stereo depth (up to 20 feet!)
- ✅ Onboard Myriad X AI chip (YOLOv8 @ 30 FPS, 5% CPU)
- ✅ 1080p or 4K resolution
- ✅ Python 3.8 compatible
- ✅ Open source drivers (depthai-ros)
- ✅ Built-in IMU

**Cons:**
- ⚠️ Heavier (60g vs 15g)
- ⚠️ Higher cost ($150 vs $50)
- ⚠️ USB bandwidth shared
- ⚠️ Slightly higher latency (~50ms)

**Use Case:** Mid-tier product, outdoor use, depth-aware grasping

---

#### OAK-D S2
**Pros:**
- ✅ Everything from OAK-D Lite, PLUS:
- ✅ Built-in 850nm IR illuminator
- ✅ IR cut filter (auto day/night)
- ✅ Better low-light sensor
- ✅ Professional-grade

**Cons:**
- ⚠️ Highest cost ($249)
- ⚠️ Slightly heavier (65g)

**Use Case:** Premium product, 24/7 operation, professional/industrial

---

#### Aurora 930 Pro (Ultra Only)
**Pros:**
- ✅ Depth sensing
- ✅ Integrated design

**Cons:**
- ❌ Proprietary driver (`.so` file)
- ❌ Python 3.10+ only
- ❌ Lower FPS (12 vs 30)
- ❌ Shorter range (4m vs 20 feet)
- ❌ No onboard AI
- ❌ Not compatible with FPV (Python 3.8)

**Decision:** OAK-D Lite/S2 are BETTER choices for FPV!

---

#### Arducam PiNSIGHT 12MP Vision AI Mate

**Overview:**
- **Sensor:** 12.3MP (RAW10/YUV/NV12/RGB)
- **Resolution:** 4000×3000 @ 30fps
- **AI / SoM:** **Luxonis OAK-SoM (Myriad X)** — same family as OAK-D; runs DepthAI pipelines
- **Interface:** USB 3.0
- **Weight:** ~60g (module; total with cable may be higher)
- **Cost:** $149
- **Product:** [Arducam PiNSIGHT 12MP](https://www.arducam.com/presalesarducam-pinsight-12mp-vision-ai-mate-for-raspberry-pi-5.html)
- **Documentation:** [Arducam PiNSIGHT Quick Start & Streaming](https://docs.arducam.com/Raspberry-Pi-Camera/Arducam-PiVistation/Vision-AI-Kit/Arducam-PiNSight/#streaming-example) — install deps, sample programs, streaming example (DepthAI/depthai_demo, MJPEG streaming).

**Pros:**
- ✅ High resolution (12MP)
- ✅ Onboard Myriad X (DepthAI) for inference
- ✅ USB 3.0 interface
- ✅ **Designed and documented for Raspberry Pi 5 only** (official quick start, dependencies, demos)

**Cons:**
- ❌ **No depth sensing** (RGB only; no stereo on PiNSIGHT)
- ❌ **RPi 5 only in docs** — Arducam does **not** document support for Jetson (e.g. Jetson Orin Nano Super). Integration with Jetson would require a Jetson-compatible DepthAI/Arducam stack; until then, **Jetson support is uncertain**.
- ❌ No built-in IR
- ⚠️ Newer product (less mature ecosystem)

**Jetson Orin Nano Super:** PiNSIGHT is **not** documented for Jetson. The device is USB 3.0 and uses the Luxonis/DepthAI stack; Arducam’s install scripts and examples target Raspberry Pi 5. Using PiNSIGHT on Jetson Orin Nano Super would require either Arducam or community-provided Jetson support — **not guaranteed to work** without that.

**Why NOT Using for LumiAI:**
1. **No depth sensing** - Defeats purpose of premium tier
2. **RPi 5 focused** - LumiAI may use other SBCs
3. **No advantage over OAK-D** for depth + AI in same price range
4. **Overkill resolution** for many robot tasks (1080p often sufficient)

**Potential Future Use:**
- High-resolution inspection on **Raspberry Pi 5** hosts
- Document scanning, quality control
- Stationary/desktop applications (not mobile arm unless RPi 5 is the arm compute)

**Status:** Documented for Raspberry Pi 5 only; not used in current LumiAI product line.

---

## Camera Selection Strategy

### Single Camera Per Unit (Simplified Architecture) ✅

**Key Decision:** Each LumiAI robot ships with **ONLY ONE camera** based on tier:

- **Entry Tier ($599):** Arducam CSI only
- **Mid Tier ($799):** OAK-D Lite only
- **Premium Tier ($999):** OAK-D S2 only

**Why not dual cameras?**
1. ✅ **Simplicity** - Single camera, single driver, single calibration
2. ✅ **Cost** - Save $50-150 per unit
3. ✅ **Weight** - Save 15-65g payload capacity
4. ✅ **Power** - Lower power consumption
5. ✅ **OAK-D has RGB!** - OAK-D includes high-quality RGB camera (no Arducam needed)
6. ✅ **Perfect alignment** - RGB and depth from same camera (no fusion calibration)

**Customer can upgrade camera tier if needed** - modularity maintained!

---

## ROSmaster X3 Plus: Dual-Camera and Arm Camera Options

**Platform:** Yahboom ROSmaster X3 Plus (and similar Yahboom car + arm platforms).

This platform uses a **dual-camera setup** that differs from the single-camera LumiAI strategy:

| Camera | Role | Typical topics | Notes |
|--------|------|----------------|--------|
| **Orbbec Astra Pro** | Stationary depth camera (chassis) | `/camera/color/image_raw`, `/camera/depth/image_raw` | Used by KCF tracker, SLAM, `yahboomcar_visual` Astra viewers |
| **Arm camera** | On arm/gripper (moves with arm) | Often `/image_raw` or `/usb_cam/image_raw` | Used by arm_autopilot, arm_mediapipe, arm_color_transport |

The **stationary Astra** provides depth and RGB for chassis-level perception. The **arm camera** provides a close-up, ego-centric view for manipulation (color tracking, hand/pose, pick-and-place). Using a higher-quality arm camera can improve tracking and detection.

### Existing Arm Camera (Stock)

**What is the existing arm camera?**

| Aspect | Details |
|--------|---------|
| **Hardware** | **USB camera**, typically **Microdia** (vendor:product ID **0c45:6340**), mounted on the arm/gripper. |
| **Device** | Usually appears as **`/dev/video0`**. Optional symlinks: **`/dev/camera_usb`** (used by arm_autopilot, arm_color_transport) and **`/dev/camera_depth`** (used by arm_mediapipe). |
| **Setup** | `ln -sf /dev/video0 /dev/camera_usb` and optionally `ln -sf /dev/video0 /dev/camera_depth` so arm nodes find the device. |
| **How arm packages use it** | **Direct OpenCV `cv.VideoCapture(device)`** — they read from the device, **not** from a ROS image topic. |
| **Packages** | `arm_autopilot` and `arm_color_transport` use `/dev/camera_usb` (fallback `0`); `arm_mediapipe` uses `/dev/camera_depth` (fallback `0`). |
| **Resolution** | Typically 640×480 (set in code). |
| **Interface** | USB UVC (standard webcam protocol). |

Because the arm nodes use **VideoCapture(device)**, any replacement camera that is **not** a UVC device (e.g. OAK-D Lite, which uses the Luxonis/DepthAI USB protocol) requires either: (1) running a ROS node that publishes images from that camera and **changing the arm packages to subscribe to that topic**, or (2) using a camera that appears as a UVC device. The sections below cover replacing with OAK-D Lite and with ArduCam/PiNSIGHT.

---

### Replacing the Existing Arm Camera with OAK-D Lite

**Why OAK-D Lite as the arm camera?**

- Better image quality (1080p/4K capable) and stereo depth from the arm’s viewpoint.
- Onboard Myriad X: run detection/tracking on the camera, offloading host CPU.
- Same DepthAI/depthai-ros ecosystem as other OAK-D use in the guide.
- Trade-off: **~60g** weight and USB 3.0 cable routing on the arm.

**Hardware**

- Mount the **OAK-D Lite** on the arm/gripper in place of the Microdia USB camera.
- Connect via **USB 3.0** to the same host that runs the arm nodes (e.g. Jetson).
- Ensure the cable does not restrict arm motion.

**Software: publish arm camera stream from OAK-D Lite**

1. Install **DepthAI** and **depthai-ros** (see [OAK-D Integration](#oak-d-integration) in this guide).
2. Launch the OAK-D Lite driver so it publishes RGB (and optionally depth) for the **arm** camera. Use a dedicated node/launch (e.g. a second OAK-D Lite instance or a launch that names this camera `arm_cam`) so it does not conflict with any chassis camera.
3. Remap the RGB topic to the topic the arm stack will use, e.g. **`/arm_cam/image_raw`** (and optionally **`/arm_cam/camera_info`**, **`/arm_cam/depth/image_raw`**).

Example remapping idea (adjust to your depthai-ros node/launch):

- Publish: `/oak/rgb/image_raw` → remap to **`/arm_cam/image_raw`**
- Publish: `/oak/rgb/camera_info` → remap to **`/arm_cam/camera_info`**
- Optionally: depth → **`/arm_cam/depth/image_raw`**

**Software: make arm packages use the OAK-D Lite topic**

The current arm packages (**arm_autopilot**, **arm_color_transport**, **arm_mediapipe**) do **not** subscribe to a ROS image topic; they use **`cv.VideoCapture("/dev/camera_usb")`** or **`cv.VideoCapture("/dev/camera_depth")`**. OAK-D Lite is **not** a UVC device, so it does not appear as `/dev/video0`. To use OAK-D Lite as the arm camera you must:

- **Option A (recommended):** Add **optional image-from-topic** support to the arm packages: e.g. a parameter `use_image_topic:=true` and `image_topic:=/arm_cam/image_raw`. When set, the node subscribes to `sensor_msgs/Image` on that topic and uses incoming frames instead of `VideoCapture.read()`. No change to behavior when `use_image_topic` is false (still use `/dev/camera_usb` or `/dev/camera_depth`).
- **Option B:** Run a small **bridge node** that subscribes to `/arm_cam/image_raw` and republishes at a fixed rate, and then modify the arm packages to subscribe to that topic instead of using VideoCapture (same idea as Option A but with a separate node for the subscription).

After Option A (or B), set `use_image_topic:=true` and `image_topic:=/arm_cam/image_raw` when launching arm_autopilot, arm_color_transport, and arm_mediapipe, with the OAK-D Lite driver running and publishing to `/arm_cam/image_raw`.

**Summary**

| Step | Action |
|------|--------|
| 1 | Replace Microdia USB camera with OAK-D Lite on the arm; connect via USB 3.0. |
| 2 | Run depthai-ros (or equivalent) for OAK-D Lite; remap RGB to `/arm_cam/image_raw`. |
| 3 | Add topic subscription support to arm_autopilot, arm_color_transport, arm_mediapipe (e.g. `use_image_topic`, `image_topic`). |
| 4 | Launch arm nodes with `image_topic:=/arm_cam/image_raw` (and `use_image_topic:=true` if you add that). |

For testing the visual pipeline with the new arm camera, see [YAHBOOMCAR_VISUAL_TEST_PLAN.md](YAHBOOMCAR_VISUAL_TEST_PLAN.md).

---

### Using ArduCam CSI or PiInsight as the Arm Camera

**Question:** Can we use **ArduCam CSI** or **Arducam PiInsight (Pinsight)** as the arm camera for better results?

**Short answer:** Yes. Both can be used as the arm camera source. Choose by resolution/weight/cost vs. current USB arm camera (e.g. Microdia).

#### ArduCam CSI as Arm Camera

Arducam CSI modules **vary by product**. The **B0240 IMX477 HQ** ([Arducam B0240](https://www.arducam.com/b0240-arducam-imx477-hq-quality-camera.html)) is **12MP (4056×3040)** and **heavier** than minimal USB cameras — way more than 720p; check product page for exact weight and mechanicals.

| Aspect | ArduCam CSI (e.g. IMX477 HQ B0240) | Typical USB arm camera (e.g. Microdia) |
|--------|-------------------------------------|----------------------------------------|
| **Interface** | CSI | USB |
| **Resolution** | Up to 12MP (4056×3040) @ 30fps | Often 640×480 |
| **Weight** | ~30g+ (HQ module; see product) | Varies |
| **Latency** | Low (CSI direct to SoC) | Slightly higher (USB) |
| **Mounting** | Requires CSI on arm/base; cable routing | USB cable to arm |

**Benefits for arm:**

- High resolution (12MP on B0240) improves color detection, hand/pose detail, and fine features.
- Lower latency (CSI) can help closed-loop control.
- Weight is higher than a tiny USB camera — verify arm payload if mounting on gripper.

**Integration:**

- Run the same `camera_ros` (or Arducam/Jetson CSI driver) for your module, and **remap** its output to the topic the arm stack expects (e.g. `/image_raw` or `/usb_cam/image_raw`). No changes needed in `yahboomcar_visual`, `arm_autopilot`, or `arm_mediapipe` if they subscribe to that topic.
- Mount the CSI camera on the arm/gripper and route the CSI cable so it does not restrict motion.

**When to use:** When you want much better image quality (e.g. 12MP) and lower latency than a basic USB arm camera, and you have CSI on the host (e.g. Jetson Orin Nano Super).

---

#### Arducam PiNSIGHT as Arm Camera

- **Product:** [Arducam PiNSIGHT 12MP](https://www.arducam.com/presalesarducam-pinsight-12mp-vision-ai-mate-for-raspberry-pi-5.html)
- **Docs (Quick Start, streaming):** [Arducam PiNSIGHT](https://docs.arducam.com/Raspberry-Pi-Camera/Arducam-PiVistation/Vision-AI-Kit/Arducam-PiNSight/#streaming-example)

| Aspect | PiNSIGHT 12MP | ArduCam CSI (e.g. B0240) | USB arm camera |
|--------|----------------|---------------------------|----------------|
| **Resolution** | 12.3MP (4000×3000) @ 30fps | Up to 12MP | Often 640×480 |
| **AI** | Luxonis OAK-SoM (Myriad X) | No | No |
| **Interface** | USB 3.0 | CSI | USB |
| **Weight** | ~60g (module) | ~30g+ | Varies |
| **Platform (doc'd)** | **Raspberry Pi 5 only** | RPi / Jetson CSI | Any |

**Benefits for arm:**

- High resolution (12MP) for fine detail (small objects, text, inspection).
- Onboard Myriad X (DepthAI) can run detection/classification on the device, offloading host CPU.

**Trade-offs:**

- **RPi 5 only in documentation.** Arducam does **not** document PiNSIGHT on **Jetson Orin Nano Super** or other Jetson boards. The stack (DepthAI, install scripts, demos) targets Raspberry Pi 5. **It does not seem like PiNSIGHT will integrate with Jetson Orin Nano Super** unless Arducam or the community adds Jetson support; assume **unsupported / uncertain** on Jetson until then.
- **No depth:** RGB only. Depth remains from the **stationary Astra**.
- **Form factor:** Check fit on arm/gripper.

**When to use:** When the **arm host is Raspberry Pi 5**, you want 12MP + onboard AI, and the form factor fits. For **Jetson Orin Nano Super**, prefer Arducam CSI (B0240 or other Jetson-supported CSI) or another USB camera with documented Jetson support.

**Integration (RPi 5):**

- Use Arducam’s PiNSIGHT software (see [PiNSIGHT docs](https://docs.arducam.com/Raspberry-Pi-Camera/Arducam-PiVistation/Vision-AI-Kit/Arducam-PiNSight/#streaming-example)); if a ROS node exists, remap its image topic to `/image_raw`. `yahboomcar_visual` nodes that use `/image_raw` or `/image_raw/compressed` will work with that source.

---

### Recommended Approach for ROSmaster X3 Plus

1. **Keep the stationary Orbbec Astra Pro** for chassis-level depth and RGB (KCF, SLAM, `yahboomcar_visual` Astra viewers).
2. **Arm camera upgrade path:**
   - **Replace with OAK-D Lite:** See [Replacing the existing arm camera with OAK-D Lite](#replacing-the-existing-arm-camera-with-oak-d-lite) — run depthai-ros, remap RGB to e.g. `/arm_cam/image_raw`, and add **topic subscription support** to arm_autopilot, arm_color_transport, and arm_mediapipe (they currently use `VideoCapture(device)` only).
   - **Best balance (quality vs. weight) on Jetson/RPi:** Use **ArduCam CSI** (e.g. **B0240 IMX477 HQ** — 12MP; see [product](https://www.arducam.com/b0240-arducam-imx477-hq-quality-camera.html)). If the CSI driver exposes a UVC device or you add topic support to the arm packages, remap or subscribe to that source.
   - **RPi 5 only — max resolution + onboard AI:** Use **PiNSIGHT** only if the arm host is **Raspberry Pi 5**; not documented for Jetson Orin Nano Super. Requires topic subscription support in arm packages unless PiNSIGHT is exposed as UVC.
3. **Software:** The **existing arm camera** is used via **`/dev/camera_usb`** or **`/dev/camera_depth`** (OpenCV `VideoCapture`). Any replacement that does **not** appear as a UVC device (e.g. OAK-D Lite, PiNSIGHT) requires adding **image-from-topic** support to `arm_autopilot`, `arm_color_transport`, and `arm_mediapipe` so they can subscribe to e.g. `/arm_cam/image_raw`.

For testing the visual stack (including any arm camera source), see [YAHBOOMCAR_VISUAL_TEST_PLAN.md](YAHBOOMCAR_VISUAL_TEST_PLAN.md).

---

## OAK-D Integration

### Hardware Setup

#### OAK-D Lite
- **Interface:** USB 3.0
- **Chipset:** Intel Myriad X (onboard AI acceleration)
- **Capabilities:**
  - RGB camera (4K @ 30fps or 1080p @ 60fps)
  - Stereo depth (up to 20 feet effective range)
  - Onboard neural network inference
  - IMU (Inertial Measurement Unit)
- **Purpose:** Primary camera for mid-tier (depth perception + AI)
- **Weight:** 60g (with cable)
- **Cost:** $150

#### OAK-D S2
- **Everything from OAK-D Lite, PLUS:**
- **Built-in 850nm IR illuminator**
- **IR cut filter (auto day/night)**
- **Better low-light sensor**
- **Weight:** 65g (with cable)
- **Cost:** $249

### OAK-D Calibration

#### Factory Calibration (Already Done) ✅

The OAK-D Lite/S2 comes **pre-calibrated from the factory** with:

- **Intrinsic calibration** - Camera lens distortion, focal length, principal point
- **Stereo calibration** - Left/right camera alignment for depth calculation
- **Calibration data stored onboard** - In the camera's EEPROM

You can verify this:
```bash
# Check factory calibration
python3 -c "
import depthai as dai
with dai.Device() as device:
    calib = device.readCalibration()
    print('Factory calibration found:', calib is not None)
    print('Stereo baseline:', calib.getBaselineDistance(), 'cm')
"
```

**No additional calibration needed for single-camera setup!**

---

### Software Integration

#### Phase 1: Install DepthAI ROS2 Driver

```bash
# Install DepthAI SDK
pip3 install depthai

# Clone depthai-ros repository
cd ~/armpi_fpv/src
git clone https://github.com/luxonis/depthai-ros.git
cd depthai-ros
git checkout iron  # or humble

# Build
cd ~/armpi_fpv
colcon build --packages-select depthai_ros_driver depthai_bridge
```

#### Phase 2: Create Launch File

Create `/home/sammydev295/armpi_fpv/src/armpi_fpv_bringup/launch/oak_d_lite.launch.py`:

```python
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='depthai_ros_driver',
            executable='rgbd_node',
            name='oak_d_lite',
            parameters=[{
                'camera_name': 'oak',
                'rgb_fps': 30,
                'depth_fps': 30,
                'rgb_resolution': '1080p',
                'depth_preset': 'high_accuracy',
                'enable_spatial_detection': True,
                'enable_imu': True,
            }],
            remappings=[
                ('/oak/rgb/image_raw', '/usb_cam/image_raw'),  # Compatible with existing code
                ('/oak/rgb/camera_info', '/usb_cam/camera_info'),
                ('/oak/stereo/image_raw', '/oak_cam/depth/image_raw'),
                ('/oak/stereo/points', '/oak_cam/depth/points'),
            ],
            output='screen'
        ),
        
        # TF transforms for camera frames
        Node(
            package='tf2_ros',
            executable='static_transform_publisher',
            name='oak_d_base_link',
            arguments=[
                '--x', '0',
                '--y', '0',
                '--z', '0',
                '--qx', '0',
                '--qy', '0',
                '--qz', '0',
                '--qw', '1',
                '--frame-id', 'camera_link',
                '--child-frame-id', 'oak_camera_link'
            ]
        ),
    ])
```

**Topics Published:**
- `/usb_cam/image_raw` - RGB image (remapped for compatibility)
- `/usb_cam/camera_info` - Camera calibration (remapped)
- `/oak_cam/depth/image_raw` - Depth image
- `/oak_cam/depth/points` - 3D point cloud
- `/oak/imu/data` - IMU data

#### Phase 3: Menu Integration

Add to `launch_armpi_fpv_tuning.sh`:

```bash
start_oak_d_camera() {
    if is_node_running "oak_d_lite"; then
        echo -e "${YELLOW}📷 OAK-D camera already running - skipping${NC}"
        return
    fi
    
    echo -e "${GREEN}📷 Starting OAK-D Depth Camera...${NC}"
    setup_ros2_env
    nohup ros2 launch armpi_fpv_bringup oak_d_lite.launch.py \
        2>&1 | ts_log >> /tmp/armpi_fpv_oak_d.log &
    echo $! > $PIDS_DIR/oak_d.pid
    echo -e "${GREEN}✅ Started with PID: $!${NC}"
}

# Menu option: "7. 📷 OAK-D Depth Camera"
```

#### Phase 4: Onboard YOLOv8 Inference 🚀

**Key Advantage:** OAK-D can run YOLOv8 **onboard** (on Myriad X VPU), offloading all AI processing from the RPi5!

**Benefits:**
- Zero CPU load on RPi5 for object detection
- 30 FPS object detection with spatial coordinates
- Lower latency (no image transfer overhead)
- Frees up RPi5 for motion planning and control

**Implementation:**
```python
# OAK-D runs YOLOv8 onboard and publishes detections
# Subscribe to /oak/nn/detections instead of processing raw images
# Each detection includes 3D spatial coordinates (x, y, z in meters)
```

**Update YOLOv8 node to use OAK-D spatial detections:**
```python
self.create_subscription(
    Detection2DArray,  # OAK-D's native detection format
    '/oak/nn/detections',
    self.detection_callback,
    10
)
```

### OAK-D Use Cases

#### 1. Depth-Aware Object Tracking
- RGB camera for visual tracking
- Depth sensor for accurate distance measurement
- Example: "Track red ball at 3.5 meters"

#### 2. Gesture Recognition (Up to 20 Feet Range)
- OAK-D's depth + onboard AI can detect hand gestures
- No need for color markers or special lighting
- Works reliably in varied lighting conditions
- MediaPipe hand tracking runs onboard

#### 3. 3D Grasping & Pick-and-Place
- Real-time 3D point cloud generation
- Calculate precise grasp poses in 3D space
- More accurate than 2D image-based inverse kinematics
- Essential for manipulation tasks

#### 4. Obstacle Avoidance
- Real-time depth map at 30 FPS
- Detect obstacles before collision
- Safe autonomous navigation
- Essential for mobile robot platforms

#### 5. Height & Volume Measurement
- Measure object dimensions using point cloud
- Calculate volumes for bin picking
- Inspect part heights for quality control

### OAK-D Testing Plan

#### Test 1: Camera Detection
```bash
# Check if OAK-D is detected via USB
lsusb | grep "03e7"  # Luxonis vendor ID

# Test DepthAI Python SDK
python3 -c "import depthai as dai; print(dai.Device.getAllAvailableDevices())"
```

**Expected Output:**
```
[X_LINK_UNBOOTED] [1.1] 1.2 [03e7:2485]
```

#### Test 2: Basic Launch
```bash
# Launch OAK-D camera node
ros2 launch armpi_fpv_bringup oak_d_lite.launch.py

# In another terminal, verify topics
ros2 topic list | grep -E "usb_cam|oak"
ros2 topic hz /usb_cam/image_raw
ros2 topic hz /oak_cam/depth/image_raw
```

**Expected Topics:**
```
/usb_cam/image_raw       (30 Hz)
/usb_cam/camera_info     (30 Hz)
/oak_cam/depth/image_raw (30 Hz)
/oak_cam/depth/points    (30 Hz)
/oak/imu/data            (200 Hz)
```

#### Test 3: Depth Visualization
```bash
# On remote desktop (RViz)
ros2 run rviz2 rviz2

# In RViz:
# 1. Set Fixed Frame to "oak_camera_link"
# 2. Add PointCloud2 display
# 3. Set Topic to /oak_cam/depth/points
# 4. Adjust Color Transformer to "AxisColor" or "Intensity"
```

#### Test 4: Onboard YOLOv8 Detection
```bash
# Deploy YOLOv8n model to OAK-D
# (Model conversion done offline using DepthAI tools)

# Launch with onboard detection enabled
ros2 launch armpi_fpv_bringup oak_d_yolo.launch.py

# View detections with spatial coordinates
ros2 topic echo /oak/nn/detections
```

**Expected Output:**
```
detections:
  - class_id: 0 (person)
    confidence: 0.87
    spatial_coordinates:
      x: 1.25  # meters from camera
      y: 0.15
      z: 2.34
```

#### Test 5: Gesture Detection (20 Feet Range)
```bash
# Use OAK-D's hand tracking model (MediaPipe)
# Stand up to 20 feet (6 meters) away and wave

ros2 topic echo /oak/nn/spatial_detections

# Move hand left/right, up/down
# Verify detection up to 20 feet distance
```

### OAK-D Advantages Over Aurora 930 Pro

| Feature | Aurora 930 Pro (Ultra) | OAK-D Lite (FPV) | Winner |
|---------|------------------------|------------------|--------|
| **Driver** | Proprietary `.so` | Open source (DepthAI) | 🏆 OAK-D |
| **Python** | 3.10+ only | 3.7+ (works with 3.8!) | 🏆 OAK-D |
| **AI Chip** | None (CPU inference) | Myriad X VPU (onboard) | 🏆 OAK-D |
| **FPS** | 12 fps | 30 fps | 🏆 OAK-D |
| **Depth Range** | 0.15-4m | 0.2-20m (65 feet) | 🏆 OAK-D |
| **IMU** | No | Yes (6-axis) | 🏆 OAK-D |
| **Cost** | ~$200 | ~$150 | 🏆 OAK-D |
| **ROS2 Support** | Custom driver | Official depthai-ros | 🏆 OAK-D |
| **YOLOv8** | RPi5 CPU (slow) | Onboard VPU (fast) | 🏆 OAK-D |
| **Max Resolution** | 640x400 | 4K (3840×2160) | 🏆 OAK-D |

**Verdict:** OAK-D Lite is superior in every category! ✅

### OAK-D Resources

- **OAK-D Lite Hardware Docs:** https://docs.luxonis.com/projects/hardware/en/latest/pages/DM9095/
- **DepthAI ROS2 Wrapper:** https://github.com/luxonis/depthai-ros
- **YOLOv8 on OAK-D:** https://docs.luxonis.com/projects/api/en/latest/samples/Yolo/tiny_yolo/
- **Spatial Detection Examples:** https://docs.luxonis.com/projects/api/en/latest/samples/SpatialDetection/spatial_tiny_yolo/
- **Hand Tracking (MediaPipe):** https://docs.luxonis.com/projects/api/en/latest/samples/mixed/mono_camera_hand_tracking/
- **DepthAI Python API:** https://docs.luxonis.com/projects/api/en/latest/

---

## Arducam Integration

### Hardware Setup

- **Interface:** CSI (Camera Serial Interface)
- **Sensor / module:** Depends on product — e.g. **IMX477 (B0240)** 12MP ([Arducam B0240](https://www.arducam.com/b0240-arducam-imx477-hq-quality-camera.html)) or IMX519-based modules
- **Resolution:** Depends on module — B0240: **12MP (4056×3040)** @ 30fps; other modules may be 720p or different
- **Purpose:** Entry-tier camera (RGB only, no depth)
- **Weight:** Depends on module — B0240 HQ is **heavier than 15g** (~30g+); lighter CSI modules exist (~15g). Check product page.
- **Cost:** ~$50 (varies by SKU)

### Software Integration

**Current Implementation:** Already integrated! 

**Launch:**
```bash
ros2 run camera_ros camera_node --ros-args \
    -p width:=640 \
    -p height:=480 \
    -p format:=BGR888 \
    -p FrameDurationLimits:="[33333,33333]" \
    --remap /camera/image_raw:=/usb_cam/image_raw
```

**Topics Published:**
- `/usb_cam/image_raw` - RGB image
- `/usb_cam/camera_info` - Camera calibration

---

## Aurora 930 Pro Integration (Ultra Only)

### Hardware Overview

**Hiwonder Aurora 930 Pro 3D Structured Light Depth Camera**

| Specification | Value |
|--------------|-------|
| **Interface** | USB 2.0 (Wafer connector) |
| **Resolution (RGB)** | 640x400 @ 12fps |
| **Resolution (Depth)** | 640x400 @ 12fps |
| **Resolution (IR)** | 640x400 @ 12fps |
| **Depth Range** | 150-3000mm (0.15-3m) |
| **Baseline** | 40mm |
| **FOV** | 74°×51° |
| **Depth Accuracy** | ±8mm @ 1m |
| **Power** | 5V, 1.5A (avg <1.6W) |
| **Operating Temp** | -10°C to 55°C |
| **Weight** | ~80g |
| **Cost** | ~$200 |

### Why NOT Compatible with FPV?

❌ **Proprietary Driver:** Uses `.so` binary (no source code)  
❌ **Python 3.10+ Only:** FPV requires Python 3.8 compatibility  
❌ **Lower Performance:** 12 FPS vs OAK-D's 30 FPS  
❌ **Shorter Range:** 0.15-3m vs OAK-D's 0.2-20m  
❌ **No Onboard AI:** Requires CPU inference (vs OAK-D's Myriad X)  
❌ **USB 2.0:** Slower than OAK-D's USB 3.0  

### Comparison: Aurora 930 Pro vs OAK-D Lite

| Feature | Aurora 930 Pro (Ultra) | OAK-D Lite (FPV) |
|---------|------------------------|------------------|
| **Driver** | Proprietary `.so` | Open source (DepthAI) |
| **Python** | 3.10+ only | 3.7+ (works with 3.8!) |
| **AI Chip** | None (CPU inference) | Myriad X (onboard) |
| **FPS** | 12 | 30 |
| **Range** | 0.15-3m | 0.2-20m (20 feet!) |
| **IMU** | No | Yes |
| **Cost** | ~$200 | ~$150 |
| **ROS2 Support** | Custom driver | Official depthai-ros |

**Decision:** OAK-D Lite is superior for FPV in every metric!


---

## Weight Budget Analysis

### Total Servo Capacity
- **Base servo (HTS-25L):** 25kg·cm @ 7.4V
- **Total arm payload:** ~500g (arm + gripper + camera + spotlight)

### Component Weights

#### Camera Options:
```
Arducam CSI:                 varies (e.g. B0240 IMX477 ~30g+; lighter modules ~15g)
OAK-D Lite:                  ~60g (with cable)
OAK-D S2:                    ~65g (with cable)
```

#### Gripper Assembly:
```
Gripper mechanism:           ~80g
Servo (gripper):             ~25g
Mounting bracket:            ~20g
Total:                       ~125g
```

#### Spotlight (Mini LED):
```
5W LED module:               ~20g
3D printed mount:            ~10g
Wiring:                      ~5g
Total:                       ~35g
```

### Weight Budget by Product Tier

#### Entry Tier: Arducam + Mini LED
```
Gripper assembly:            125g
Arducam CSI:                 15–30g+ (depends on module; B0240 ~30g+)
Mini LED spotlight:          35g
─────────────────────────────────
Total:                       175–190g+ ✅ (35–38% capacity)
Margin:                      310g+
Status:                      Excellent!
```

#### Mid Tier: OAK-D Lite + Mini LED
```
Gripper assembly:            125g
OAK-D Lite:                  60g
Mini LED spotlight:          35g
─────────────────────────────────
Total:                       220g ✅ (44% capacity)
Margin:                      280g
Status:                      Perfect balance!
```

#### Premium Tier: OAK-D S2 + Mini LED
```
Gripper assembly:            125g
OAK-D S2:                    65g
Mini LED spotlight:          35g
─────────────────────────────────
Total:                       225g ✅ (45% capacity)
Margin:                      275g
Status:                      Ideal premium setup!
```

**Conclusion:** All three tiers are viable with mini LED spotlights!

---

## Camera Integration Plans

### Entry Tier: Arducam CSI Integration

**Hardware:**
- Arducam CSI camera (e.g. **B0240 IMX477 HQ** 12MP or IMX519-based module — see [B0240](https://www.arducam.com/b0240-arducam-imx477-hq-quality-camera.html))
- CSI ribbon cable
- 3D printed camera mount

**Software:**
```bash
# Launch Arducam
ros2 run camera_ros camera_node --ros-args \
    -p width:=640 \
    -p height:=480 \
    -p format:=BGR888 \
    -p FrameDurationLimits:="[33333,33333]" \
    --remap /camera/image_raw:=/usb_cam/image_raw
```

**Topics Published:**
- `/usb_cam/image_raw` - RGB image
- `/usb_cam/camera_info` - Camera calibration

---

### Mid/Premium Tier: OAK-D Integration

**Hardware:**
- OAK-D Lite or S2
- USB 3.0 cable
- 3D printed camera mount

**Software Installation:**
```bash
# Install DepthAI SDK
pip3 install depthai

# Clone depthai-ros repository
cd ~/armpi_fpv/src
git clone https://github.com/luxonis/depthai-ros.git
cd depthai-ros
git checkout iron  # or humble

# Build
cd ~/armpi_fpv
colcon build --packages-select depthai_ros_driver depthai_bridge
```

**Launch File:** `/home/sammydev295/armpi_fpv/src/armpi_fpv_bringup/launch/oak_d_lite.launch.py`

```python
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='depthai_ros_driver',
            executable='rgbd_node',
            name='oak_d_lite',
            parameters=[{
                'camera_name': 'oak',
                'rgb_fps': 30,
                'depth_fps': 30,
                'rgb_resolution': '1080p',
                'depth_preset': 'high_accuracy',
                'enable_spatial_detection': True,
                'enable_imu': True,
            }],
            remappings=[
                ('/oak/rgb/image_raw', '/usb_cam/image_raw'),  # Compatible with existing code
                ('/oak/rgb/camera_info', '/usb_cam/camera_info'),
                ('/oak/stereo/image_raw', '/oak_cam/depth/image_raw'),
                ('/oak/stereo/points', '/oak_cam/depth/points'),
            ],
            output='screen'
        ),
        
        # TF transforms for camera frames
        Node(
            package='tf2_ros',
            executable='static_transform_publisher',
            name='oak_d_base_link',
            arguments=[
                '--x', '0',
                '--y', '0',
                '--z', '0',
                '--qx', '0',
                '--qy', '0',
                '--qz', '0',
                '--qw', '1',
                '--frame-id', 'camera_link',
                '--child-frame-id', 'oak_camera_link'
            ]
        ),
    ])
```

**Topics Published:**
- `/usb_cam/image_raw` - RGB image (remapped)
- `/usb_cam/camera_info` - Camera calibration (remapped)
- `/oak_cam/depth/image_raw` - Depth image
- `/oak_cam/depth/points` - 3D point cloud
- `/oak/imu/data` - IMU data

---

## YOLOv8 Integration

### YOLOv8 Image Topic Subscription

#### Current Implementation:
```python
# yolov8_detect_demo.py (line 84, 118)
image_topic = self.get_parameter('image_topic').get_parameter_value().string_value
self.image_sub = self.create_subscription(Image, image_topic, self.image_callback, 1)
```

**It's configurable via ROS2 parameter!** This means it works with **any camera** that publishes `sensor_msgs/Image`.

### Topic Mapping by Camera:

| Camera | RGB Topic | YOLOv8 Works? | Performance |
|--------|-----------|---------------|-------------|
| **Arducam CSI** | `/usb_cam/image_raw` | ✅ Yes | CPU-based inference |
| **OAK-D Lite** | `/oak/rgb/image_raw` | ✅ Yes | CPU or onboard inference |
| **OAK-D S2** | `/oak/rgb/image_raw` | ✅ Yes | CPU or onboard inference |

---

### YOLOv8 Performance by Camera Tier

#### Entry Tier: Arducam CSI + RPi5 CPU Inference

**Setup:**
```bash
ros2 run yolov8_detect yolov8_detect_demo.py \
    --ros-args -p image_topic:=/usb_cam/image_raw
```

**Performance:**
- **Resolution:** 640x480 (optimal for YOLOv8n)
- **FPS:** ~8-12 FPS (YOLOv8n on RPi5)
- **CPU Load:** ~40-50% (all 4 cores)
- **Latency:** ~80-120ms per frame
- **Accuracy:** Good for common objects (80 COCO classes)

**Pros:**
- ✅ Lowest cost ($50 camera)
- ✅ Works out of the box
- ✅ Good for indoor, controlled lighting

**Cons:**
- ⚠️ CPU intensive (affects other tasks)
- ⚠️ Lower FPS (not real-time)
- ⚠️ No depth data

---

#### Mid Tier: OAK-D Lite + Onboard Inference 🚀

**Setup Option 1: CPU Inference (same as Arducam)**
```bash
ros2 run yolov8_detect yolov8_detect_demo.py \
    --ros-args -p image_topic:=/oak/rgb/image_raw
```
- Same performance as Arducam tier

**Setup Option 2: Onboard Inference (MUCH BETTER!)** ⭐
```python
# Use OAK-D's built-in YOLOv8 pipeline
Node(
    package='depthai_ros_driver',
    parameters=[{
        'nn_type': 'yolo',
        'nn_model': 'yolov8n',
        'spatial_detection': True,  # Include depth!
    }]
)

# Subscribe to onboard detections
self.detection_sub = self.create_subscription(
    Detection2DArray, 
    '/oak/nn/detections', 
    self.detection_callback, 
    1
)
```

**Performance (Onboard):**
- **Resolution:** 1080p (OAK-D can handle it!)
- **FPS:** ~25-30 FPS (Myriad X chip)
- **CPU Load:** ~5% (almost zero!)
- **Latency:** ~30-40ms per frame
- **Accuracy:** Same as CPU inference
- **BONUS:** Each detection includes 3D spatial coordinates!

**Pros:**
- ✅ Real-time performance (30 FPS)
- ✅ Frees up RPi5 CPU (can run more tasks)
- ✅ Depth data for each detection
- ✅ Night vision with external IR

**Cons:**
- ⚠️ Slightly higher cost (+$100 vs Arducam)
- ⚠️ External IR needs GPIO control

---

#### Premium Tier: OAK-D S2 + Built-in IR + Onboard Inference 🌟

**Same as Mid Tier, but with:**
- ✅ Automatic night vision (no GPIO needed)
- ✅ Better low-light performance
- ✅ Professional-grade solution

**Performance:** Identical to OAK-D Lite (same Myriad X chip)

---

### Real-World Performance Comparison

#### Test Scenario: Detect person at 10 feet

| Tier | Camera | FPS | CPU | Latency | Night Vision | Cost |
|------|--------|-----|-----|---------|--------------|------|
| **Entry** | Arducam | 10 | 45% | 100ms | ❌ No | $50 |
| **Mid** | OAK-D Lite (CPU) | 10 | 45% | 100ms | 🟡 External | $150 |
| **Mid** | OAK-D Lite (Onboard) | 30 | 5% | 35ms | 🟡 External | $150 |
| **Premium** | OAK-D S2 (Onboard) | 30 | 5% | 35ms | ✅ Built-in | $249 |

---

## Product Tier Specifications

### Entry Tier ($499-599)

**Camera System:**
- **Camera:** Arducam CSI (e.g. **B0240 IMX477 HQ** 12MP — [product](https://www.arducam.com/b0240-arducam-imx477-hq-quality-camera.html); or other CSI modules)
- **Resolution:** Up to 12MP (4056×3040) or 720p depending on module
- **Depth:** ❌ No
- **AI:** YOLOv8 on host CPU (~10 FPS)
- **Night Vision:** Mini LED spotlight (relay-controlled)
- **Weight:** 175–190g+ total (35–38% capacity; CSI weight depends on module)

**Use Cases:**
- Indoor use, daytime only
- Non-critical applications
- Hobbyists, education
- "AI-powered tracking" (10 FPS is fine)

**BOM:**
- Arducam CSI: $50
- Mini LED + mount: $6.50-9.50
- **Total camera system: $56.50-59.50**

---

### Mid Tier ($699-799)

**Camera System:**
- **Camera:** OAK-D Lite
- **Resolution:** 1080p @ 30fps
- **Depth:** ✅ Yes (up to 20 feet)
- **AI:** YOLOv8 onboard (30 FPS, 5% CPU)
- **Night Vision:** Mini LED spotlight (relay-controlled)
- **Weight:** 220g total (44% capacity)

**Use Cases:**
- Outdoor use with external IR
- Real-time AI (30 FPS onboard)
- Depth-aware grasping
- "Professional AI with depth sensing"

**BOM:**
- OAK-D Lite: $150
- Mini LED + mount: $8.50-13.50
- **Total camera system: $158.50-163.50**

---

### Premium Tier ($899-999)

**Camera System:**
- **Camera:** OAK-D S2
- **Resolution:** 1080p @ 30fps
- **Depth:** ✅ Yes (up to 20 feet)
- **AI:** YOLOv8 onboard (30 FPS, 5% CPU)
- **Night Vision:** Mini LED spotlight + Built-in IR (dual-mode)
- **Weight:** 225g total (45% capacity)

**Use Cases:**
- 24/7 operation (day/night)
- Autonomous systems
- Research, industrial
- "Premium AI vision system"

**BOM:**
- OAK-D S2: $249
- Mini LED + mount: $12.50-18.50
- **Total camera system: $261.50-267.50**

---

## Summary

### Key Decisions:

**Camera Strategy:**
- ✅ **Single OAK-D camera** is the right call for production
- ✅ **Start with OAK-D Lite** ($150), validate everything works
- ✅ **Offer OAK-D S2** ($249) as premium tier option
- ✅ **All existing code works** with simple topic remapping

**Calibration:**
- ✅ **OAK-D comes pre-calibrated** (intrinsic + stereo)
- ✅ **Arducam uses LAB color calibration** (already implemented)
- ✅ **No multi-camera calibration needed** (one camera per unit)

**YOLOv8 Performance:**
- ✅ **Entry tier:** 10 FPS CPU inference (acceptable)
- ✅ **Mid tier:** 30 FPS onboard inference (real-time!)
- ✅ **Premium tier:** 30 FPS + dual night vision (professional)

**Weight Budget:**
- ✅ **All three tiers viable** with mini LED spotlights
- ✅ **Excellent safety margins** (35-45% capacity used)
- ✅ **Room for future upgrades**

**Bottom Line:** Each LumiAI ships with ONE camera based on tier:
- **Entry:** Arducam CSI only (~$50) — Resolution/weight depend on module (e.g. B0240 12MP, ~30g+); color tracking, LAB calibration
- **Mid:** OAK-D Lite only ($150) - Depth + onboard AI + 30 FPS YOLOv8
- **Premium:** OAK-D S2 only ($249) - Everything + active IR for 24/7 operation

**No multi-camera setups!** Simple, cost-effective, reliable. 🚀

### Why OAK-D for Mid/Premium Tiers?

**Technical Advantages:**
- ✅ **Onboard AI chip** (Myriad X VPU) - Zero RPi5 CPU load for YOLOv8
- ✅ **30 FPS depth sensing** - 2.5x faster than Aurora 930 Pro (12 FPS)
- ✅ **20m range** - 5x better than Aurora (4m max)
- ✅ **4K RGB camera** - 10x resolution of Aurora (640x400)
- ✅ **Python 3.8 compatible** - Works with existing Hiwonder drivers
- ✅ **Open source driver** - No proprietary `.so` dependencies
- ✅ **6-axis IMU** - Bonus sensor for motion tracking
- ✅ **$99 cheaper** than Aurora 930 Pro ($150 vs $249)

**Business Advantages:**
- 💰 **Lower cost** - Save $99 vs Ultra's Aurora camera
- 🚀 **Better performance** - 30 FPS vs 12 FPS
- 🔧 **Easier integration** - Official ROS2 support
- 📦 **Better supply chain** - Multiple retailers (Waveshare, Luxonis)
- 🛠️ **Future-proof** - Active development, large community

**Developer Experience:**
- 🎯 **Comprehensive docs** - Luxonis has excellent documentation
- 🧪 **Example code** - Many ROS2 examples available
- 🤝 **Community support** - Active Discord and GitHub
- 🔄 **Regular updates** - DepthAI SDK actively maintained

---

*Document Version: 2.0 - Feb 2025*  
*Company: DrDev*  
*Product: LumiAI Intelligent Vision Robotic Arm*

