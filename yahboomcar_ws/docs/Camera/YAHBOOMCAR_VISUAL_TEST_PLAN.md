# yahboomcar_visual Package — Test Plan

**Package:** `yahboomcar_visual`  
**Path:** `yahboomcar_ws/src/yahboomcar_visual`  
**Purpose:** Camera image viewing, relay, flip, laser-to-image, AR overlay, and RGB/depth display for the Yahboom ROSmaster X3 Plus platform.

This plan covers all ROS2 nodes and visual features in the package. **Not all tests use the same camera:**

- **Tests 1–3 (Astra viewers)** apply **only to the stationary Orbbec Astra Pro** depth camera (chassis). They subscribe to `/camera/color/image_raw` and `/camera/depth/image_raw`. If you use a different chassis depth camera (e.g. OAK-D Lite), you would need to remap its topics to that namespace to run these viewers, or treat Tests 1–3 as Astra-specific.
- **Tests 4–5 (pub_image, astra_image_flip)** and **Test 7 (simple_AR)** use **both** the **OAK-D Lite** and the **stationary (Orbbec Astra) camera**: run each test **first with OAK-D Lite** as the source, **then again with the stationary camera** as the source. See the individual test sections for how to switch the source (topic remapping for Tests 4–5; device or topic for Test 7).
- **Test 6 (laser_to_image)** does not use a camera; it uses LaserScan (`/scan`).

The platform typically has a **stationary Orbbec Astra Pro** (chassis) and an **arm camera** (e.g. Microdia USB, or replacement such as OAK-D Lite / ArduCam / PiNSIGHT). See [Camera_Integration_Guide.md](Camera_Integration_Guide.md) for arm camera options and replacing with OAK-D Lite.

---

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Topic and Camera Mapping](#topic-and-camera-mapping)
3. [Test 1: Astra RGB Image Viewer](#test-1-astra-rgb-image-viewer)
4. [Test 2: Astra Depth Image Viewer](#test-2-astra-depth-image-viewer)
5. [Test 3: Astra Color + Depth (Synchronized)](#test-3-astra-color--depth-synchronized)
6. [Test 4: pub_image (Image Relay)](#test-4-pub_image-image-relay)
7. [Test 5: astra_image_flip](#test-5-astra_image_flip)
8. [Test 6: laser_to_image](#test-6-laser_to_image)
9. [Test 7: simple_AR](#test-7-simple_ar)
10. [Regression and Integration Checklist](#regression-and-integration-checklist)
11. [Troubleshooting](#troubleshooting)

---

## Prerequisites

- **Workspace built:** `yahboomcar_ros2_ws_new` built with `colcon build --packages-select yahboomcar_visual`.
- **Source workspace:** `source install/setup.bash` (from workspace root).
- **Display:** Nodes that use `cv.imshow()` must run where a display is available (local or X11 forwarding).

**Per-test camera requirements:**

| Tests | Camera / source required |
|-------|---------------------------|
| **1–3** (Astra viewers) | **Stationary Orbbec Astra Pro only.** Launch `yahboomcar_astra` so `/camera/color/image_raw` and `/camera/depth/image_raw` are published. |
| **4–5** (pub_image, astra_image_flip) | Run **twice**: (1) **OAK-D Lite** → remap its RGB to `/image_raw`; (2) **Stationary Astra** → remap `/camera/color/image_raw` to `/image_raw`. |
| **6** (laser_to_image) | **LaserScan** on `/scan` (lidar or laser driver). No camera. |
| **7** (simple_AR) | Run **twice**: (1) **OAK-D Lite** — if simple_AR has topic support, subscribe to OAK-D Lite RGB; else use arm camera device; (2) **Stationary Astra** — use Astra as device (`/dev/videoX`) if exposed, or subscribe to `/camera/color/image_raw` if simple_AR has topic support. |

---

## Topic and Camera Mapping

| Node / feature        | Subscribes to                          | Publishes to                  | Display / output        |
|-----------------------|----------------------------------------|-------------------------------|--------------------------|
| `astra_rgb_image`     | `/camera/color/image_raw` (Astra)      | —                             | OpenCV window "color_image" |
| `astra_depth_image`   | `/camera/depth/image_raw` (Astra)       | —                             | OpenCV window "depth_image" |
| `astra_color_point`   | `/camera/color/image_raw`, `/camera/depth/image_raw` | —        | "frame", "depthFrame"   |
| `pub_image`           | `/image_raw`                           | `/image`                      | None (relay only)        |
| `astra_image_flip`    | `/image_raw/compressed`                | `/image_flip/compressed`      | "flip_image"             |
| `laser_to_image`      | `/scan` (LaserScan)                    | `/laserImage`                 | "img" (bird’s-eye)       |
| `simple_AR`           | `/Graphics_topic` (String)            | `/simpleAR/camera` (Image)    | Uses `VideoCapture(0)` + AR overlay |

**Camera roles on ROSmaster X3 Plus:**

- **Astra (stationary):** `/camera/color/image_raw`, `/camera/depth/image_raw` — used **only** by Tests 1–3 (astra_rgb_image, astra_depth_image, astra_color_point) and KCF tracker. These tests apply **only** to the Orbbec Astra.
- **Arm / generic RGB:** Publishes to `/image_raw` (or a device for simple_AR). Used by **Tests 4–5** (pub_image, astra_image_flip) and **Test 7** (simple_AR). Can be the stock arm camera (Microdia USB), OAK-D Lite (remapped), ArduCam CSI, or any node publishing to `/image_raw`.

**Summary: which tests use which camera?**

| Test | Camera(s) | Run order | Topic or device |
|------|-----------|-----------|------------------|
| 1–3 | **Stationary Orbbec Astra only** | Once | `/camera/color/image_raw`, `/camera/depth/image_raw` |
| 4–5 | **OAK-D Lite, then stationary Astra** | Run test **twice** (1st with OAK-D Lite, 2nd with Astra) | `/image_raw` fed by remap from OAK-D Lite RGB, then from `/camera/color/image_raw` |
| 6 | None (LaserScan) | Once | `/scan` |
| 7 | **OAK-D Lite, then stationary Astra** | Run test **twice** (1st with OAK-D Lite, 2nd with Astra) | Device (e.g. `/dev/camera_usb`, `/dev/videoX`) or topic if simple_AR supports it |

---

## Test 1: Astra RGB Image Viewer (Stationary Astra Only)

**Node:** `astra_rgb_image`  
**Purpose:** Verify RGB stream from the Orbbec Astra Pro and display in an OpenCV window.

**Precondition:** Astra driver running and publishing `/camera/color/image_raw`.

1. Start Astra camera (if not already running):
   ```bash
   ros2 launch yahboomcar_astra astra.launch.py
   ```
2. In another terminal (with display):
   ```bash
   source install/setup.bash
   ros2 run yahboomcar_visual astra_rgb_image
   ```
3. **Expected:**
   - Window titled **"color_image"** shows live 640×480 RGB from the Astra.
   - No errors in the terminal.
4. **Optional:** Check topic:
   ```bash
   ros2 topic hz /camera/color/image_raw
   ```

**Pass criteria:** Window appears, image is live and correctly oriented; no crashes.

---

## Test 2: Astra Depth Image Viewer (Stationary Astra Only)

**Node:** `astra_depth_image`  
**Purpose:** Verify depth stream from the **Orbbec Astra Pro** and display in an OpenCV window.

**Precondition:** Astra driver running and publishing `/camera/depth/image_raw`.

1. Astra running (see Test 1).
2. Run:
   ```bash
   ros2 run yahboomcar_visual astra_depth_image
   ```
3. **Expected:**
   - Window **"depth_image"** shows depth (32FC1, resized to 640×480). May look dark; move objects to see variation.
   - No errors.

**Pass criteria:** Depth window appears and updates; no crashes.

---

## Test 3: Astra Color + Depth (Stationary Astra Only)

**Node:** `astra_color_point`  
**Purpose:** Verify time-synchronized RGB and depth from the **Orbbec Astra Pro** and display both.

**Precondition:** Astra publishing both color and depth.

1. Astra running.
2. Run:
   ```bash
   ros2 run yahboomcar_visual astra_color_point
   ```
3. **Expected:**
   - Two windows: **"frame"** (RGB) and **"depthFrame"** (depth), updating together.
   - Terminal may print "start it".

**Pass criteria:** Both windows appear and stay in sync; no crashes.

---

## Test 4: pub_image (Image Relay) — Run with OAK-D Lite, Then with Stationary Astra

**Node:** `pub_image`  
**Purpose:** Subscribe to `/image_raw`, resize to 640×480, publish on `/image`. No display. **Run this test twice:** first with **OAK-D Lite** as the source for `/image_raw`, then again with the **stationary Orbbec Astra** as the source.

### Test 4a: Source = OAK-D Lite

1. Start **OAK-D Lite** (depthai-ros) so its RGB is published (e.g. `/oak/rgb/image_raw`). Remap that topic to `/image_raw` (e.g. in launch: `--remap /oak/rgb/image_raw:=/image_raw`), or run a relay node that subscribes to the OAK-D RGB topic and publishes to `/image_raw`.
2. Run:
   ```bash
   ros2 run yahboomcar_visual pub_image
   ```
3. Check output:
   ```bash
   ros2 topic hz /image
   ros2 topic echo /image --once
   ```
4. **Pass:** `/image` is published at expected rate; resolution 640×480; no errors. Stop the node when done.

### Test 4b: Source = Stationary (Orbbec Astra)

1. Start the **stationary Astra** so `/camera/color/image_raw` is published:
   ```bash
   ros2 launch yahboomcar_astra astra.launch.py
   ```
2. Remap Astra color to `/image_raw` for this test. In a **separate terminal**, run a one-off relay so `pub_image` sees the Astra as source:
   ```bash
   ros2 run topic_tools relay /camera/color/image_raw /image_raw
   ```
   (If `topic_tools` is not available, use a small relay node or remap in a launch that runs both Astra and pub_image.)
3. Run (in another terminal):
   ```bash
   ros2 run yahboomcar_visual pub_image
   ```
4. Check output:
   ```bash
   ros2 topic hz /image
   ros2 topic echo /image --once
   ```
5. **Pass:** `/image` is published at expected rate; resolution 640×480; no errors.

**Pass criteria (Test 4 overall):** Both 4a (OAK-D Lite) and 4b (stationary Astra) pass.

---

## Test 5: astra_image_flip — Run with OAK-D Lite, Then with Stationary Astra

**Node:** `astra_image_flip`  
**Purpose:** Subscribe to compressed image, flip horizontally, display and publish compressed flipped image. **Run this test twice:** first with **OAK-D Lite** as the source for `/image_raw` (and thus `/image_raw/compressed`), then again with the **stationary Orbbec Astra** as the source.

**Precondition for both runs:** Something must publish **compressed** images on `/image_raw/compressed`. Use `image_transport republish raw compressed` with its input remapped to the current RGB source (OAK-D Lite RGB topic or Astra color topic).

### Test 5a: Source = OAK-D Lite

1. Start **OAK-D Lite** (depthai-ros) with RGB published (e.g. `/oak/rgb/image_raw`).
2. Run a republisher so compressed images appear on `/image_raw/compressed` from the OAK-D Lite RGB stream:
   ```bash
   ros2 run image_transport republish raw compressed --ros-args \
     -r in:=/oak/rgb/image_raw -r out/compressed:=/image_raw/compressed
   ```
3. Run:
   ```bash
   ros2 run yahboomcar_visual astra_image_flip
   ```
4. **Expected:** Window **"flip_image"** shows flipped image from OAK-D Lite; `/image_flip/compressed` is published. Stop the node when done.

### Test 5b: Source = Stationary (Orbbec Astra)

1. Start the **stationary Astra** (e.g. `ros2 launch yahboomcar_astra astra.launch.py`) so `/camera/color/image_raw` is published.
2. Run a republisher so compressed images appear on `/image_raw/compressed` from the Astra:
   ```bash
   ros2 run image_transport republish raw compressed --ros-args \
     -r in:=/camera/color/image_raw -r out/compressed:=/image_raw/compressed
   ```
3. Run:
   ```bash
   ros2 run yahboomcar_visual astra_image_flip
   ```
4. **Expected:** Window **"flip_image"** shows flipped image from the Astra; `/image_flip/compressed` is published.

**Pass criteria (Test 5 overall):** Both 5a (OAK-D Lite) and 5b (stationary Astra) show flipped image and publish `/image_flip/compressed`; no crashes.

---

## Test 6: laser_to_image

**Node:** `laser_to_image`  
**Purpose:** Convert `/scan` (LaserScan) to a bird’s-eye image and publish/display.

**Precondition:** LaserScan on `/scan` (e.g. from lidar or laser driver).

1. Start lidar/scan publisher (e.g. bringup or simulation).
2. Run:
   ```bash
   ros2 run yahboomcar_visual laser_to_image
   ```
3. **Expected:**
   - Window **"img"** shows bird’s-eye view of scan (640×480 resize).
   - Topic `/laserImage` (sensor_msgs/Image) published.

**Pass criteria:** Window and topic active; no crashes when `/scan` is present. If `/scan` is not available, node may sit idle (no failure required).

---

## Test 7: simple_AR — Run with OAK-D Lite, Then with Stationary Astra

**Node:** `simple_AR`  
**Purpose:** Capture from a camera (device or topic), overlay AR graphics based on `/Graphics_topic`, publish to `/simpleAR/camera`. **Run this test twice:** first with **OAK-D Lite** as the source, then again with the **stationary (Orbbec Astra) camera** as the source.

**Note:** The current node uses `cv.VideoCapture(0)` or a device path — it does **not** subscribe to a ROS image topic. So:
- **If simple_AR is unchanged:** Run 7a with whichever **device** is the OAK-D Lite (OAK-D Lite usually does **not** expose UVC, so you may need to run 7a with the **stationary Astra** as device if it appears as `/dev/videoX`, and 7b with the arm camera device, or vice versa depending on which devices you have).
- **If simple_AR is extended to subscribe to an image topic:** Run 7a with OAK-D Lite RGB remapped to that topic; run 7b with `/camera/color/image_raw` remapped to that topic.

Below assumes you have **two usable sources** (e.g. OAK-D Lite via a topic bridge, and Astra as device, or both as topics if simple_AR supports it). Adjust if only one source is available as a device.

### Test 7a: Source = OAK-D Lite

1. **If simple_AR has topic subscription support:** Start OAK-D Lite (depthai-ros) and remap its RGB to the topic simple_AR subscribes to (e.g. `/image_raw` or an `image_topic` parameter). Run simple_AR with that topic. Verify window and `/simpleAR/camera` publish.
2. **If simple_AR uses only VideoCapture(device):** OAK-D Lite typically does not appear as `/dev/videoX`. Either add topic support to simple_AR and use step 1, or **skip 7a with OAK-D Lite** and note "N/A (OAK-D Lite not UVC)" and run 7b with the stationary camera as device.
3. Ensure `astra.yaml` exists at the path used in code.
4. Run:
   ```bash
   ros2 run yahboomcar_visual simple_AR
   ```
5. Optionally: `ros2 topic pub /Graphics_topic std_msgs/String "data: 'Triangle'" --once` to switch overlay.
6. **Pass:** Node runs; `/simpleAR/camera` publishes; graphic overlay works when applicable.

### Test 7b: Source = Stationary (Orbbec Astra)

1. Start the **stationary Astra** (`ros2 launch yahboomcar_astra astra.launch.py`).
2. **If simple_AR has topic subscription support:** Remap `/camera/color/image_raw` to the topic simple_AR subscribes to. Run simple_AR. Verify window and `/simpleAR/camera` publish.
3. **If simple_AR uses only VideoCapture(device):** If the Astra driver exposes a video device (e.g. `/dev/video2`), set that as the camera index or device path and run simple_AR. If the Astra does not expose UVC, use a relay node that subscribes to `/camera/color/image_raw` and republishes to a topic that simple_AR can use only if you add topic support; otherwise note "Astra not as device" and document which device was used for 7b (e.g. arm camera at `/dev/camera_usb`).
4. Run:
   ```bash
   ros2 run yahboomcar_visual simple_AR
   ```
5. **Pass:** Node runs; `/simpleAR/camera` publishes; graphic overlay works when applicable.

**Pass criteria (Test 7 overall):** Both 7a (OAK-D Lite) and 7b (stationary Astra) pass where the hardware/simple_AR implementation allows; otherwise document which run was skipped and why.

---

## Regression and Integration Checklist

Use this for a full pass after changes to the package or drivers. **Tests 4, 5, and 7 are run twice each:** once with OAK-D Lite as source, then again with the stationary (Astra) camera as source.

| # | Test | Source 1 (run first) | Source 2 (run second) | Pass 1 | Pass 2 |
|---|------|---------------------|------------------------|--------|--------|
| 1 | Astra RGB viewer (`astra_rgb_image`) | — | **Stationary Astra only** | — | ☐ |
| 2 | Astra depth viewer (`astra_depth_image`) | — | **Stationary Astra only** | — | ☐ |
| 3 | Astra color+depth sync (`astra_color_point`) | — | **Stationary Astra only** | — | ☐ |
| 4 | Image relay (`pub_image`) | **OAK-D Lite** → `/image_raw` | **Stationary Astra** → `/image_raw` | ☐ | ☐ |
| 5 | Image flip (`astra_image_flip`) | **OAK-D Lite** → `/image_raw/compressed` | **Stationary Astra** → `/image_raw/compressed` | ☐ | ☐ |
| 6 | Laser to image (`laser_to_image`) | — | **LaserScan** `/scan` (no camera) | — | ☐ |
| 7 | simple_AR | **OAK-D Lite** (topic or device if available) | **Stationary Astra** (topic or device if available) | ☐ | ☐ |

**Integration with other packages:**

- **arm_autopilot / arm_mediapipe / arm_color_transport:** Use the **arm camera** (device or topic). Ensure the arm camera driver and, if used, `pub_image` or remaps match what those nodes expect. See [Camera_Integration_Guide.md](Camera_Integration_Guide.md) for replacing with OAK-D Lite.
- **yahboomcar_astra (KCF tracker):** Uses **stationary Astra** color/depth; same Astra launch as Tests 1–3.

---

## Troubleshooting

- **No image in Astra viewers:** Start `yahboomcar_astra` first; check `ros2 topic list` and `ros2 topic hz /camera/color/image_raw` and `/camera/depth/image_raw`.
- **"No module named 'cv_bridge'":** Install `ros-<distro>-cv-bridge` and `ros-<distro>-sensor-msgs`.
- **"No module named 'message_filters'":** Install `ros-<distro>-message-filters`.
- **simple_AR calibration path:** If you see file-not-found for `astra.yaml`, edit `simple_AR.py` and set `yaml_path` to the correct workspace path (e.g. `.../yahboomcar_ros2_ws_new/.../yahboomcar_visual/astra.yaml`).
- **Display (cv.imshow) not available:** Run with `DISPLAY` set (e.g. SSH with X11 forwarding) or on the robot’s local display.
- **Arm camera for better quality / OAK-D Lite:** See [Camera_Integration_Guide.md — ROSmaster X3 Plus and Arm Camera Options](Camera_Integration_Guide.md#rosmaster-x3-plus-yahboom-car-dual-camera-and-arm-camera-options) and [Replacing the existing arm camera with OAK-D Lite](Camera_Integration_Guide.md#replacing-the-existing-arm-camera-with-oak-d-lite).

---

*Document version: 1.2 — Tests 4, 5, 7 run twice: first with OAK-D Lite, then with stationary (Astra) camera*
