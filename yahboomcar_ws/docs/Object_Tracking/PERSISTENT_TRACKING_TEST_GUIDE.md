# Persistent Tracking Test Guide
## Yahboom ROSMASTER X3PLUS 6DOF Robot

**Date:** January 2025  
**Platform:** Yahboom ROSMASTER X3PLUS 6DOF Robot  
**ROS Distribution:** ROS2 Humble  
**Camera:** Orbbec Astra Pro Plus Depth Camera  
**Tracking Method:** KCF (Kernelized Correlation Filter) Tracker

---

## Overview

This guide covers **object tracking testing only** for four systems: **KCF** (Astra camera, chassis following), **arm_autopilot** (color-based arm tracking), **MediaPipe** (hand/pose tracking for arm control), and **arm_color_transport** (color-based pick and transport with navigation). It does not include generic arm control verification; see `ARM_JOYSTICK_CONTROL.md` for arm control testing.

### Key Features

- **KCF Tracker:** Persistent object tracking (Astra camera), depth integration, chassis following via `/cmd_vel`.
- **Arm Autopilot:** Color-based object tracking on arm camera (red, green, blue, yellow) → `/TargetAngle` for 6-DOF arm + gripper.
- **MediaPipe:** Hand and pose tracking on arm camera for gesture/pose-based arm control → `/TargetAngle`.
- **Arm Color Transport:** Color detection on arm camera (red, yellow, green, blue) → grip target, then Nav2 navigation to color pose → arm place → return. Uses `/TargetAngle`, `/cmd_vel`, Nav2 `navigate_to_pose`, and `color_end_pose` (MarkerArray).
- **Visual Feedback:** KCF_image; arm_autopilot, MediaPipe, and arm_color_transport OpenCV windows.

---

## Prerequisites

### Hardware Requirements

- ✅ Yahboom ROSMASTER X3PLUS 6DOF robot
- ✅ Orbbec Astra Pro Plus depth camera (fixed mount)
- ✅ USB arm camera (Microdia, ID 0c45:6340) - optional for arm tracking
- ✅ Joystick controller (for safety override)
- ✅ Sufficient workspace (minimum 2m x 2m for movement tests)

### Software Requirements

- ✅ ROS2 Humble installed and sourced
- ✅ Yahboom workspace built and sourced:
  ```bash
  cd /home/jetson/yahboomcar_ws  # or yahboomcar_ros2_ws_new/yahboomcar_ws
  source /opt/ros/humble/setup.bash
  source install/setup.bash
  ```

### Required Packages

- ✅ `yahboomcar_bringup` - Base robot driver
- ✅ `yahboomcar_astra` - Camera and KCF tracker
- ✅ `yahboomcar_ctrl` - Joystick control
- ✅ `yahboomcar_description` - Robot model
- ✅ `yahboomcar_msgs` - Custom messages
- ✅ `orbbec_camera` - Astra camera driver
- ✅ `arm_autopilot` - Arm-based color object tracking (arm camera → `/TargetAngle`)
- ✅ `arm_mediapipe` - Hand/pose tracking for arm control (arm camera → `/TargetAngle`)
- ✅ `arm_color_transport` - Color-based pick and transport with navigation (arm camera + Nav2)

### Verify Package Installation

```bash
# Check if packages are built
ros2 pkg list | grep yahboomcar

# Expected output should include:
# yahboomcar_astra
# yahboomcar_bringup
# yahboomcar_ctrl
# yahboomcar_description
# yahboomcar_msgs
```

---

## System Architecture

### Topic Flow

**Path A: KCF Tracker (Astra fixed camera, chassis following)**

```
Astra Pro Plus Camera
    ├── /color/image_raw → KCF Tracker
    ├── /depth/image_raw → KCF Tracker (distance measurement)
    └── /depth/points → Optional (point cloud)

KCF Tracker Node
    ├── Subscribes: /color/image_raw, /depth/image_raw, /JoyState
    ├── Publishes: /KCF_image (visualization), /cmd_vel (robot movement)
    └── Output: Tracking box, object center, distance

Robot Control
    └── /cmd_vel → driver_node → Chassis movement
```

**Path B: Arm Autopilot (arm camera, color tracking, arm following)**

```
Arm Camera (USB: /dev/camera_usb or /dev/video0)
    └── Direct capture in arm_autopilot node (OpenCV)

Arm Autopilot Node (arm_autopilot/autopilot_main.py)
    ├── Subscribes: (none; uses OpenCV VideoCapture)
    ├── Publishes: /TargetAngle (yahboomcar_msgs/ArmJoint)
    ├── Config: config/HSV.yaml (red, green, blue, yellow)
    └── Output: Color-based object center → arm joint commands

Robot Control
    └── /TargetAngle → driver_node → Arm movement (6-DOF + gripper)
```

**Path C: MediaPipe (arm camera, hand/pose tracking, arm control)**

```
Arm Camera (USB: /dev/camera_usb or /dev/video0)
    └── Direct capture in arm_mediapipe node (OpenCV)

Arm MediaPipe Node (arm_mediapipe/ArmCtrl.py)
    ├── Subscribes: (none; uses OpenCV VideoCapture)
    ├── Publishes: /TargetAngle (yahboomcar_msgs/ArmJoint)
    └── Output: Hand/pose landmarks → arm joint commands (gesture/pose control)

Robot Control
    └── /TargetAngle → driver_node → Arm movement
```

**Path D: Arm Color Transport (arm camera, color detection, navigation + arm)**

```
Arm Camera (USB: /dev/camera_usb or /dev/video0)
    └── Direct capture in arm_color_transport node (OpenCV)

Color Transport Node (arm_color_transport/transport_main.py)
    ├── Subscribes: RGBLight, color_end_pose (MarkerArray – color goal poses)
    ├── Publishes: /TargetAngle, /cmd_vel, /Buzzer, /Transport/rgb
    ├── Nav2: navigate_to_pose action (drive to color pose)
    └── Output: Color in center ROI (red/yellow/green/blue) → grip → navigate to pose → place → return

Robot Control
    ├── /TargetAngle → driver_node → Arm movement
    └── /cmd_vel → driver_node → Chassis (or Nav2 drives chassis)
```

**Note:** arm_color_transport typically requires the full navigation stack (map, AMCL, move_base/Nav2) and a node that publishes `color_end_pose` (e.g. marker drawing / map goals). Launch may be `color_transport.launch.py` (node only) with bringup and nav launched separately.

### Key Topics

| Topic | Type | Purpose |
|-------|------|---------|
| `/color/image_raw` | `sensor_msgs/Image` | Astra RGB feed for KCF tracking |
| `/depth/image_raw` | `sensor_msgs/Image` | Depth data for distance measurement |
| `/JoyState` | `std_msgs/Bool` | Safety override (must be `true` for chassis movement) |
| `/KCF_image` | `sensor_msgs/Image` | KCF tracking visualization output |
| `/cmd_vel` | `geometry_msgs/Twist` | Chassis movement commands (KCF) |
| `/TargetAngle` | `yahboomcar_msgs/ArmJoint` | Arm commands (arm_autopilot, arm_mediapipe, arm_color_transport) |
| `color_end_pose` | `visualization_msgs/MarkerArray` | Color goal poses for transport (arm_color_transport) |
| `/Transport/rgb` | `sensor_msgs/Image` | Transport visualization (arm_color_transport) |

---

## Test Procedures

### Phase 1: Basic System Verification

#### Test 1.1: Verify Camera Connection

**Purpose:** Ensure Astra Pro Plus camera is detected and publishing data.

**Steps:**

1. **Check USB device:**
   ```bash
   lsusb | grep -i orbbec
   # Expected: Device with Orbbec identifier
   ```

2. **Launch camera only:**
   ```bash
   ros2 launch yahboomcar_astra astra.launch.py
   ```

3. **Verify topics in new terminal:**
   ```bash
   ros2 topic list | grep -E 'color|depth|camera'
   # Expected (Astra driver): /color/image_raw, /depth/image_raw, /ir/image_raw, /depth/points, etc.
   ```

4. **Check image publishing:**
   ```bash
   ros2 topic hz /color/image_raw
   # Expected: ~10–30 Hz
   ```

5. **View camera feed (optional):**
   ```bash
   ros2 run image_view image_view --ros-args -r image:=/color/image_raw
   # Or in rqt_image_view: select topic /color/image_raw
   # Note: Camera driver publishes bgr8; yuv422_yuy2 was fixed in uvc_camera_driver.cpp for rqt_image_view compatibility.
   ```

**Expected Results:**
- ✅ Camera detected via USB
- ✅ All camera topics publishing
- ✅ RGB image at ~30 FPS
- ✅ Depth image available

**Troubleshooting:**
- If camera not detected: Check USB connection, try different USB port
- If no topics: Verify `orbbec_camera` package is installed
- If low FPS: Check USB bandwidth, close other applications

---

#### Test 1.2: Verify Base System

**Purpose:** Ensure robot base system is operational.

**Steps:**

1. **Launch base system:**
   ```bash
   ros2 launch yahboomcar_bringup yahboomcar.launch.py robot_type:=X3plus
   ```

2. **Verify driver node:**
   ```bash
   ros2 node list | grep driver
   # Expected: /driver_node
   ```

3. **Check odometry:**
   ```bash
   ros2 topic echo /odom --once
   # Expected: Odometry message with pose and twist
   ```

4. **Check joint states:**
   ```bash
   ros2 topic echo /joint_states --once
   # Expected: Joint states for 6 arm joints
   ```

**Expected Results:**
- ✅ Driver node running
- ✅ Odometry publishing
- ✅ Joint states available for all 6 arm joints

---

#### Test 1.3: Verify Joystick Control

**Purpose:** Ensure joystick safety override is working.

**Steps:**

1. **Launch joystick control:**
   ```bash
   ros2 launch yahboomcar_ctrl yahboom_joy.launch.py
   ```

2. **Check JoyState topic:**
   ```bash
   ros2 topic echo /JoyState --once
   # Expected: data: true or false
   ```

3. **Test joystick buttons:**
   - Press right trigger (gas button) to toggle `/JoyState`
   - Verify topic changes between `true` and `false`

**Expected Results:**
- ✅ `/JoyState` topic publishing
- ✅ Right trigger toggles state
- ✅ `true` enables movement, `false` disables

**Note:** KCF tracker requires `/JoyState` to be `true` before robot will move.

---

### Phase 2: KCF Tracker Basic Testing

#### Test 2.1: Launch KCF Tracker System

**Purpose:** Start complete tracking system with all dependencies.

**Steps:**

1. **Launch KCF tracker (includes all dependencies):**
   ```bash
   ros2 launch yahboomcar_astra KCFTracker.launch.py robot_type:=X3plus
   ```

2. **Verify all nodes are running:**
   ```bash
   ros2 node list
   # Expected nodes:
   # /KCF_Tracker
   # /driver_node
   # /yahboom_joy
   # /joy_node
   # /robot_state_publisher
   # Camera nodes (from orbbec_camera)
   ```

3. **Check KCF tracker topics:**
   ```bash
   ros2 topic list | grep -E "KCF|camera|JoyState|cmd_vel"
   # Expected:
   # /KCF_image
   # /camera/rgb/image_raw
   # /camera/depth/image_raw
   # /JoyState
   # /cmd_vel
   ```

**Expected Results:**
- ✅ All nodes start successfully
- ✅ KCF tracker node running
- ✅ All required topics available
- ✅ Camera feed visible (if using image_view)

---

#### Test 2.2: Object Selection and Initialization

**Purpose:** Test object selection and tracker initialization.

**Steps:**

1. **View tracking visualization:**
   ```bash
   ros2 run image_view image_view --ros-args -r image:=/KCF_image
   ```

2. **Select object:**
   - A window should appear showing camera feed
   - **Click and drag** with mouse to draw rectangle around object
   - Release mouse button to initialize tracker

3. **Verify tracking initialization:**
   - Yellow rectangle should appear around selected object
   - Red dot should mark object center
   - Tracker should follow object if it moves

**Expected Results:**
- ✅ Mouse selection works
- ✅ Yellow tracking box appears
- ✅ Red center dot visible
- ✅ Tracker maintains object identity

**Troubleshooting:**
- If no window appears: Check DISPLAY variable, ensure X11 forwarding if using SSH
- If selection doesn't work: Verify mouse input, try clicking and dragging slowly
- If tracker loses object immediately: Check lighting, ensure object has good contrast

---

#### Test 2.3: Tracking Persistence Test

**Purpose:** Verify tracker maintains object identity during movement and occlusions.

**Steps:**

1. **Initialize tracker** (as in Test 2.2)

2. **Test scenarios:**

   **Scenario A: Slow Movement**
   - Move object slowly left/right
   - Move object slowly up/down
   - **Expected:** Yellow box follows smoothly, red dot tracks center

   **Scenario B: Fast Movement**
   - Move object quickly across frame
   - **Expected:** Tracker follows, may lag slightly but recovers

   **Scenario C: Partial Occlusion**
   - Partially cover object with hand or another object
   - **Expected:** Tracker maintains position, recovers when object reappears

   **Scenario D: Scale Changes**
   - Move object closer/farther from camera
   - **Expected:** Yellow box scales appropriately

   **Scenario E: Rotation**
   - Rotate object in place
   - **Expected:** Tracker maintains lock (KCF handles rotation well)

3. **Monitor tracking quality:**
   ```bash
   # Watch for tracking failures in node output
   # Look for messages about tracking loss or re-initialization
   ```

**Expected Results:**
- ✅ Tracker follows object in all scenarios
- ✅ Yellow box maintains correct size and position
- ✅ Red dot stays at object center
- ✅ No false tracking of background objects
- ✅ Recovery after brief occlusions

**Success Criteria:**
- Tracking maintained for >30 seconds continuous
- Recovery from occlusion within 1-2 seconds
- No false positives (tracking wrong object)

---

#### Test 2.4: Depth Measurement Test

**Purpose:** Verify depth sensing integration for distance measurement.

**Steps:**

1. **Launch tracker** (as in Test 2.1)

2. **Place object at known distances:**
   - 0.5 meters from camera
   - 1.0 meters from camera
   - 1.5 meters from camera
   - 2.0 meters from camera

3. **Initialize tracker** on object at each distance

4. **Check depth output:**
   - Tracker takes 5 depth measurements around object center
   - Averages values for stability
   - Ignores values < 0.4m or > 10m

5. **Verify distance accuracy:**
   ```bash
   # Monitor node output for depth values
   # Compare with measured distance
   ```

**Expected Results:**
- ✅ Depth values reported for each distance
- ✅ Accuracy within ±10cm at 1m distance
- ✅ Depth updates smoothly as object moves
- ✅ Invalid depths (< 0.4m, > 10m) filtered out

**Note:** Depth accuracy depends on:
- Object surface properties (textured surfaces work better)
- Lighting conditions
- Camera calibration

---

### Phase 3: Robot Following (Chassis Movement)

#### Test 3.1: Enable Safety Override

**⚠️ SAFETY WARNING: Robot will move! Clear area around robot (minimum 2m x 2m).**

**Purpose:** Enable robot movement via joystick safety override.

**Steps:**

1. **Verify joystick is connected:**
   ```bash
   ros2 topic echo /joy --once
   # Should show joystick axes and buttons
   ```

2. **Enable movement:**
   - Press **right trigger** (gas button) on joystick
   - Verify `/JoyState` becomes `true`:
     ```bash
     ros2 topic echo /JoyState --once
     # Expected: data: true
     ```

3. **Keep joystick ready:**
   - Keep right trigger pressed or toggled to `true`
   - Be ready to disable (press trigger again) if needed

**Expected Results:**
- ✅ `/JoyState` is `true`
- ✅ Robot ready to receive movement commands

---

#### Test 3.2: Basic Following Test

**Purpose:** Test robot following a stationary object.

**⚠️ SAFETY: Clear area, keep joystick ready to disable movement.**

**Steps:**

1. **Launch complete system:**
   ```bash
   ros2 launch yahboomcar_astra KCFTracker.launch.py robot_type:=X3plus
   ```

2. **Enable safety override** (Test 3.1)

3. **Place object 1-2 meters in front of robot**

4. **Initialize tracker** on object

5. **Enable depth following:**
   - Press **SPACE** key in tracker window
   - This enables distance-based following

6. **Observe robot behavior:**
   - Robot should maintain distance to object
   - Robot should center object in frame (angular adjustment)
   - Monitor `/cmd_vel` topic:
     ```bash
     ros2 topic echo /cmd_vel
     # Should show linear and angular velocity commands
     ```

**Expected Results:**
- ✅ Robot maintains target distance (minDist parameter)
- ✅ Robot centers object horizontally
- ✅ Smooth movement, no jerky motions
- ✅ Robot stops when object is centered and at correct distance

**PID Tuning:**
If robot oscillates or overshoots:
- Adjust linear PID parameters (distance control)
- Adjust angular PID parameters (centering control)
- Parameters can be tuned via ROS2 parameter system

---

#### Test 3.3: Moving Object Following

**Purpose:** Test robot following a moving object.

**⚠️ SAFETY: Larger clear area needed (3m x 3m minimum).**

**Steps:**

1. **Setup:**
   - Launch system (Test 3.1)
   - Enable safety override
   - Initialize tracker on object

2. **Test scenarios:**

   **Scenario A: Slow Horizontal Movement**
   - Move object slowly left/right
   - **Expected:** Robot rotates to follow, maintains distance

   **Scenario B: Slow Forward/Backward Movement**
   - Move object closer/farther
   - **Expected:** Robot moves forward/backward to maintain distance

   **Scenario C: Combined Movement**
   - Move object in circle or figure-8 pattern
   - **Expected:** Robot follows smoothly, maintaining distance and centering

3. **Monitor performance:**
   ```bash
   # Check cmd_vel commands
   ros2 topic hz /cmd_vel
   # Should be ~10-30 Hz
   ```

**Expected Results:**
- ✅ Robot follows object smoothly
- ✅ Maintains target distance
- ✅ Keeps object centered
- ✅ No excessive oscillation
- ✅ Recovers if object temporarily lost

**Success Criteria:**
- Following maintained for >60 seconds
- Distance maintained within ±20cm
- Object stays within center 1/3 of frame

---

#### Test 3.4: Tracking Loss Recovery

**Purpose:** Test system behavior when object is lost.

**Steps:**

1. **Setup tracking** (as in previous tests)

2. **Test loss scenarios:**

   **Scenario A: Object Leaves Frame**
   - Move object out of camera view
   - **Expected:** Robot stops, tracker reports loss

   **Scenario B: Object Occluded**
   - Completely cover object for 2-3 seconds
   - **Expected:** Robot may continue briefly, then stop

   **Scenario C: Object Reappears**
   - After loss, bring object back into view
   - **Expected:** Need to re-initialize tracker (draw new rectangle)

3. **Verify safety behavior:**
   - Robot should stop when tracking lost
   - No erratic movement
   - Ready for re-initialization

**Expected Results:**
- ✅ Safe stop when object lost
- ✅ Clear indication of tracking loss
- ✅ Easy re-initialization when object returns

---

### Phase 4: Arm Autopilot Object Tracking

The **arm_autopilot** package (`src/arm_autopilot`) provides **color-based object tracking** using the **arm/gripper USB camera**. The arm follows colored objects (red, green, blue, yellow) and can pick/place. This complements the KCF tracker (Astra camera, chassis following).

**Prerequisites for Phase 5:**
- Base system running (driver_node)
- Arm camera available: `/dev/camera_usb` or `/dev/video0` (Microdia USB camera)
- Optional: Joystick for override; `arm_autopilot_full.launch.py` can include bringup + joy

---

#### Test 4.1: Verify Arm Autopilot Prerequisites

**Purpose:** Ensure arm camera and base system are ready for arm_autopilot.

**Steps:**

1. **Verify arm camera device:**
   ```bash
   ls -l /dev/camera_usb /dev/video0 2>/dev/null || ls -l /dev/video*
   # Expected: camera device present (e.g. /dev/video0 for Microdia arm camera)
   ```

2. **Verify base system (driver_node) is running:**
   ```bash
   ros2 node list | grep driver
   # Expected: /driver_node
   ```

3. **Verify arm topics exist when driver is running:**
   ```bash
   ros2 topic list | grep -E "TargetAngle|ArmAngleUpdate|joint_states"
   # Expected: /TargetAngle, /ArmAngleUpdate, /joint_states
   ```

**Expected Results:**
- ✅ Arm camera device present
- ✅ driver_node running
- ✅ /TargetAngle and /ArmAngleUpdate available

---

#### Test 4.2: Launch Arm Autopilot

**Purpose:** Start the arm autopilot node (color-based object tracking).

**Steps:**

1. **Launch base system first** (if not already running):
   ```bash
   ros2 launch yahboomcar_bringup yahboomcar.launch.py robot_type:=X3plus
   ```

2. **Launch arm autopilot (node only):**
   ```bash
   ros2 launch arm_autopilot arm_autopilot.launch.py
   ```

   **Or launch with dependencies (bringup + joy + autopilot):**
   ```bash
   ros2 launch arm_autopilot arm_autopilot_full.launch.py
   ```
   *(Note: full launch may use different bringup launch name; adjust if `yahboomcar_bringup_X3_launch.py` is not present—use `yahboomcar_bringup_X3plus_launch.py` or equivalent.)*

3. **Verify node is running:**
   ```bash
   ros2 node list | grep line_detect
   # Expected: /line_detect (arm_autopilot main node)
   ```

4. **Check for OpenCV window:**
   - A window titled `frame` should appear showing the arm camera feed and (when a colored object is in view) tracking overlay.

**Expected Results:**
- ✅ arm_autopilot node (`line_detect`) running
- ✅ Camera window visible with arm camera feed
- ✅ No errors in console about missing camera or driver

**Troubleshooting:**
- If camera not found: Ensure `/dev/camera_usb` or `/dev/video0` is available and not used by another process. In container, devices are passed via `run_docker_ros2.sh`.
- If bringup not running: Launch `yahboomcar_bringup` first so `/TargetAngle` is subscribed by the driver.

---

#### Test 4.3: Color-Based Object Tracking

**Purpose:** Verify arm follows a colored object (red, green, blue, or yellow).

**Steps:**

1. **Ensure arm_autopilot is running** (Test 4.2).

2. **Place a colored object** (e.g. red or green block/ball) in view of the **arm camera** (on the gripper/arm).

3. **Select target color** (if configurable in UI or parameters):
   - Default target is red. Colors: 0=red, 1=green, 2=blue, 3=yellow.
   - HSV ranges are in `arm_autopilot/config/HSV.yaml`.

4. **Observe behavior:**
   - Arm should move so the object stays centered in the camera view.
   - Monitor `/TargetAngle`:
     ```bash
     ros2 topic echo /TargetAngle
     ```
   - You should see `ArmJoint` messages when the object is detected.

5. **Move the object slowly** left/right and up/down; arm should follow.

**Expected Results:**
- ✅ Colored object is detected (visual feedback in camera window).
- ✅ Arm moves to center the object.
- ✅ /TargetAngle publishes when object is in view.
- ✅ Smooth arm motion, no joint limit errors.

**Success Criteria:**
- Tracking maintained for >30 seconds with slow object movement.
- Arm recovers when object is temporarily lost.

---

#### Test 4.4: HSV Calibration (Optional)

**Purpose:** Tune color detection for your lighting and objects.

**Steps:**

1. **Edit HSV configuration:**
   ```bash
   # In workspace
   nano src/arm_autopilot/config/HSV.yaml
   ```
   Adjust `Hmin`, `Hmax`, `Smin`, `Smax`, `Vmin`, `Vmax` for the target color (red, green, blue, yellow).

2. **Or use dynamic parameters** (if supported):
   ```bash
   ros2 param list /line_detect
   ros2 param set /line_detect Hmin 0
   ros2 param set /line_detect Hmax 9
   # ... etc.
   ```

3. **Restart arm_autopilot** after YAML changes and re-run Test 4.3.

**Expected Results:**
- ✅ Improved detection for your environment and colored objects.

---

#### Test 4.5: Integration with KCF Tracker (Optional)

**Purpose:** Run both tracking systems where applicable (e.g. chassis follow with KCF + arm ready, or arm follow with arm_autopilot). They use different cameras and control outputs.

**Note:**
- **KCF tracker:** Astra camera → `/cmd_vel` (chassis).
- **Arm autopilot:** Arm camera → `/TargetAngle` (arm).
- Running both: launch bringup + joy, then either KCF or arm_autopilot (or both if desired). Do not run two nodes that both command the arm without coordination.

**Steps:**

1. Launch bringup + joy.
2. Launch either:
   - `ros2 launch yahboomcar_astra KCFTracker.launch.py robot_type:=X3plus` for chassis following, or
   - `ros2 launch arm_autopilot arm_autopilot.launch.py` for arm color following.
3. Verify only one arm-controlling node is active if you expect deterministic behavior.

**Expected Results:**
- ✅ Either chassis or arm (or coordinated use) responds to the intended tracker.

---

### Phase 5: MediaPipe Object Tracking

The **arm_mediapipe** package (`src/arm_mediapipe`) provides **hand and pose tracking** on the arm camera for gesture/pose-based arm control. The arm responds to hand gestures or body pose landmarks. This complements KCF (chassis) and arm_autopilot (color).

**Prerequisites for Phase 5:**
- Base system running (driver_node)
- Arm camera available: `/dev/camera_usb` or `/dev/video0`

---

#### Test 5.1: Launch MediaPipe Arm Control

**Purpose:** Start hand/pose tracking for arm control.

**Steps:**

1. **Launch base system first** (if not already running):
   ```bash
   ros2 launch yahboomcar_bringup yahboomcar.launch.py robot_type:=X3plus
   ```

2. **Launch MediaPipe arm control:**
   ```bash
   ros2 launch arm_mediapipe arm_mediapipe.launch.py
   ```

3. **Verify node is running:**
   ```bash
   ros2 node list | grep hand_ctrl_arm
   # Expected: /hand_ctrl_arm
   ```

4. **Check for OpenCV window:** A window should appear showing the arm camera feed with hand/pose overlay when a hand or body is in view.

**Expected Results:**
- ✅ arm_mediapipe node (`hand_ctrl_arm`) running
- ✅ Camera window visible with arm camera feed
- ✅ Hand/pose landmarks visible when hand or body is in frame

---

#### Test 5.2: Hand/Pose Tracking and Arm Response

**Purpose:** Verify arm responds to hand gestures or pose.

**Steps:**

1. **Ensure arm_mediapipe is running** (Test 5.1).

2. **Place hand (or full body for pose) in view of the arm camera.**

3. **Observe behavior:** Arm should move according to gestures/pose. Monitor `/TargetAngle`:
   ```bash
   ros2 topic echo /TargetAngle
   ```

4. **Try different gestures/poses** as documented for the node (e.g. hand open/close, pose landmarks).

**Expected Results:**
- ✅ Hand/pose detected (visual overlay in window).
- ✅ /TargetAngle publishes when gestures/pose are recognized.
- ✅ Arm moves in response to tracking.

**Note:** Do not run arm_mediapipe and arm_autopilot at the same time; both publish to `/TargetAngle`. Use one tracking mode at a time.

---

### Phase 6: Arm Color Transport Object Tracking

The **arm_color_transport** package (`src/arm_color_transport`) provides **color-based pick and transport**: it detects a color (red, yellow, green, blue) in the arm camera’s center ROI, grips the target, then uses **Nav2** to navigate to a stored color pose, places, and returns. It combines color tracking with navigation and arm control.

**Prerequisites for Phase 6:**
- Base system running (driver_node)
- Arm camera available: `/dev/camera_usb` or `/dev/video0`
- **Navigation stack** (map server, AMCL, Nav2 / move_base) if testing full transport
- A node or process that publishes **`color_end_pose`** (MarkerArray) with goal poses per color (e.g. from map or marker drawing), or test color detection only without navigation

---

#### Test 6.1: Launch Arm Color Transport

**Purpose:** Start the color transport node (color detection + optional Nav2).

**Steps:**

1. **Launch base system first** (if not already running):
   ```bash
   ros2 launch yahboomcar_bringup yahboomcar.launch.py robot_type:=X3plus
   ```

2. **Launch color transport (node only):**
   ```bash
   ros2 launch arm_color_transport color_transport.launch.py
   ```
   For full transport demos, also start the navigation stack and any node that publishes `color_end_pose` (see package docs or `arm_areas_launch_catalog.md` for transport_base-style launch).

3. **Verify node is running:**
   ```bash
   ros2 node list | grep color_transport
   # Expected: /color_transport
   ```

4. **Check for OpenCV window:** A window should appear showing the arm camera feed with center ROI (280,180)–(360,260) and detected color (H min/max).

**Expected Results:**
- ✅ arm_color_transport node (`color_transport`) running
- ✅ Camera window visible with arm camera feed and center ROI
- ✅ No errors about missing camera or driver

---

#### Test 6.2: Color Detection in Center ROI

**Purpose:** Verify color detection (red, yellow, green, blue) in the center region.

**Steps:**

1. **Ensure arm_color_transport is running** (Test 6.1).

2. **Place a colored object** (red, yellow, green, or blue) in the **center** of the arm camera view (within the green rectangle).

3. **Observe:** The node prints H min/max and detects color. States: Init → Grip (SPACE or joy action) → Transport (when `color_end_pose` has goals) → Grip_down → come_back.

4. **Press SPACE** (or trigger joy action) to enter Grip mode; node will detect color in ROI and can start Grip_Target (arm movement).

**Expected Results:**
- ✅ Color detected in center ROI (red/yellow/green/blue).
- ✅ H min/max displayed; state advances to Grip when triggered.
- ✅ Arm can move to grip (Grip_Target) when color is detected.

---

#### Test 6.3: Full Transport with Navigation (Optional)

**Purpose:** Test full flow: grip → navigate to color pose → place → return. Requires Nav2 and `color_end_pose` publisher.

**Steps:**

1. **Launch navigation stack** (map, AMCL, Nav2) and any node that publishes `color_end_pose` (MarkerArray) with goal poses for red/green/blue/yellow.

2. **Run arm_color_transport** (Test 6.1).

3. **Place colored object in center ROI**, press SPACE to enter Grip; after grip, node should send Nav2 goal from `color_end_pose` for that color.

4. **Monitor:** Navigation to goal → Grip_down (place) → come_back (return to start). Check `/TargetAngle`, `navigate_to_pose`, and `/Transport/rgb` if needed.

**Expected Results:**
- ✅ After grip, robot navigates to corresponding color pose.
- ✅ On arrival, arm places; then robot returns to start.
- ✅ Buzzer/visual feedback as per node logic.

**Note:** Do not run arm_color_transport together with arm_autopilot or arm_mediapipe; only one node should command `/TargetAngle` at a time.

---

## Parameter Tuning

### KCF Tracker Parameters

The KCF tracker can be tuned via ROS2 parameters. Common parameters:

| Parameter | Default | Description |
|-----------|---------|-------------|
| `minDist` | 1500 (mm) | Minimum distance to maintain |
| `linear_Kp` | 3.0 | Linear PID proportional gain |
| `linear_Ki` | 0.0 | Linear PID integral gain |
| `linear_Kd` | 1.0 | Linear PID derivative gain |
| `angular_Kp` | 0.5 | Angular PID proportional gain |
| `angular_Ki` | 0.0 | Angular PID integral gain |
| `angular_Kd` | 2.0 | Angular PID derivative gain |

### Tuning Procedure

1. **Launch tracker with parameter file:**
   ```bash
   ros2 launch yahboomcar_astra KCFTracker.launch.py robot_type:=X3plus
   # Parameters can be set via launch file arguments or parameter files
   ```

2. **Test and adjust:**
   - If robot oscillates: Reduce `Kp`, increase `Kd`
   - If robot too slow: Increase `Kp`
   - If robot overshoots: Increase `Kd`, reduce `Kp`

3. **Save tuned parameters:**
   - Create parameter YAML file
   - Load via launch file

---

## Troubleshooting

### Common Issues

#### Issue 1: Tracker Loses Object Immediately

**Symptoms:**
- Yellow box disappears right after initialization
- Red dot not visible

**Solutions:**
- ✅ Check lighting conditions (ensure good contrast)
- ✅ Try different object (high contrast, textured)
- ✅ Ensure object fills reasonable portion of frame (not too small)
- ✅ Check camera focus
- ✅ Verify camera is not moving/shaking

#### Issue 2: Robot Doesn't Move

**Symptoms:**
- Tracker working (yellow box visible)
- No movement commands published

**Solutions:**
- ✅ Verify `/JoyState` is `true` (press right trigger)
- ✅ Check `/cmd_vel` topic:
  ```bash
  ros2 topic echo /cmd_vel
  # Should show velocity commands
  ```
- ✅ Verify driver node is running
- ✅ Check for errors in node output
- ✅ Ensure SPACE key was pressed to enable depth following

#### Issue 3: Robot Moves Erratically

**Symptoms:**
- Oscillation
- Jerky movements
- Overshooting target

**Solutions:**
- ✅ Tune PID parameters (reduce `Kp`, increase `Kd`)
- ✅ Check depth measurement accuracy
- ✅ Verify object is not too close/far (0.4m - 10m range)
- ✅ Ensure stable camera mount (no vibration)

#### Issue 4: Camera Not Detected

**Symptoms:**
- No camera topics
- Error messages about camera

**Solutions:**
- ✅ Check USB connection: `lsusb | grep -i orbbec`
- ✅ Try different USB port (prefer USB 3.0)
- ✅ Verify camera permissions:
  ```bash
  ls -l /dev/video*
  sudo chmod 666 /dev/video0  # if needed
  ```
- ✅ Check if other processes using camera
- ✅ Restart camera driver

#### Issue 5: Low Tracking Performance

**Symptoms:**
- Low FPS
- Lag in tracking

**Solutions:**
- ✅ Close other applications
- ✅ Reduce image resolution (if configurable)
- ✅ Check CPU usage: `htop`
- ✅ Verify USB bandwidth (use USB 3.0 port)
- ✅ Disable unnecessary nodes

---

## Performance Benchmarks

### Expected Performance

| Metric | Target | Notes |
|--------|--------|-------|
| Tracking FPS | 20-30 Hz | Depends on CPU and image resolution |
| Depth Update Rate | 10-30 Hz | Matches camera depth rate |
| Command Rate | 10-30 Hz | `/cmd_vel` publishing rate |
| Tracking Latency | <100ms | From frame capture to command |
| Distance Accuracy | ±10cm @ 1m | Depends on object and lighting |
| Tracking Range | 0.4m - 10m | Depth camera limits |

### System Resources

| Component | CPU Usage | RAM Usage |
|-----------|-----------|-----------|
| KCF Tracker | 15-25% | ~200MB |
| Camera Driver | 5-10% | ~100MB |
| Base Driver | 5-10% | ~100MB |
| Total System | 30-50% | ~500MB |

**Note:** Performance depends on hardware (Jetson Nano vs Orin, etc.)

---

## Test Checklist

Use this checklist to verify all functionality:

### Phase 1: Basic Verification
- [ ] Camera detected and publishing
- [ ] Base system operational
- [ ] Joystick control working
- [ ] `/JoyState` toggles correctly

### Phase 2: Tracker Testing
- [ ] KCF tracker launches successfully
- [ ] Object selection works (mouse drag)
- [ ] Tracking box appears (yellow rectangle)
- [ ] Center dot visible (red dot)
- [ ] Tracking persists during movement
- [ ] Tracking handles occlusions
- [ ] Depth measurement accurate

### Phase 3: Robot Following
- [ ] Safety override enabled (`/JoyState` = true)
- [ ] Robot maintains distance
- [ ] Robot centers object
- [ ] Smooth following movement
- [ ] Handles moving objects
- [ ] Safe stop on tracking loss

### Phase 4: Arm Autopilot Object Tracking
- [ ] Arm camera device present (/dev/camera_usb or /dev/video0)
- [ ] arm_autopilot launches successfully
- [ ] Camera window shows arm camera feed
- [ ] Colored object detected (red/green/blue/yellow)
- [ ] Arm follows colored object (/TargetAngle publishing)
- [ ] HSV calibration tested (optional)
- [ ] Integration with KCF (optional) understood

### Phase 5: MediaPipe Object Tracking
- [ ] arm_mediapipe launches successfully
- [ ] Camera window shows arm camera feed with hand/pose overlay
- [ ] Hand or pose detected
- [ ] Arm responds to gestures/pose (/TargetAngle publishing)

### Phase 6: Arm Color Transport Object Tracking
- [ ] arm_color_transport launches successfully
- [ ] Camera window shows arm camera feed with center ROI
- [ ] Color detected in center (red/yellow/green/blue)
- [ ] Grip mode triggers arm grip when color in ROI
- [ ] Full transport with Nav2 tested (optional; requires nav stack and color_end_pose)

---

## Advanced Usage

### Custom Object Tracking

To track specific objects:

1. **Use color-based pre-filtering:**
   - Combine with color tracker for specific colored objects
   - Use YOLOv8 for object class detection

2. **Multi-object tracking:**
   - Current KCF tracker tracks single object
   - For multiple objects, consider YOLOv8 with tracking

### Integration with Other Systems

**SLAM Integration:**
- KCF tracker can work alongside SLAM
- Use separate namespaces if needed

**Navigation Stack:**
- Tracker can override navigation commands
- Use priority system or separate `/cmd_vel` topics

**Arm Autopilot:**
- Can combine with arm autopilot for pick-and-place
- Coordinate via shared topics or services

---

## References

### Documentation
- `yahboomcar_astra` package documentation
- `yahboomcar_astra_launch_catalog.md` - Launch file details
- `yahboomcar_topics.md` - Topic reference
- `TEST_PLAN.md` - General testing procedures

### Key Files
- Launch: `yahboomcar_astra/launch/KCFTracker.launch.py`
- Node: `yahboomcar_astra/src/KCF_Tracker.cpp`
- Tracker: `yahboomcar_astra/src/yahboomcar_astra/kcftracker.cpp`
- Arm autopilot: `arm_autopilot/launch/arm_autopilot.launch.py`, `arm_autopilot_full.launch.py`
- Arm autopilot node: `arm_autopilot/arm_autopilot/autopilot_main.py`
- Arm autopilot config: `arm_autopilot/config/HSV.yaml`
- MediaPipe: `arm_mediapipe/launch/arm_mediapipe.launch.py`
- MediaPipe node: `arm_mediapipe/arm_mediapipe/ArmCtrl.py`
- Arm color transport: `arm_color_transport/launch/color_transport.launch.py`
- Color transport node: `arm_color_transport/arm_color_transport/transport_main.py`
- Color transport common: `arm_color_transport/arm_color_transport/transport_common.py`

### Related Packages
- `yahboomcar_bringup` - Base robot driver
- `yahboomcar_ctrl` - Joystick control
- `yahboomcar_astra` - Camera and KCF tracking
- `orbbec_camera` - Astra camera driver
- `arm_autopilot` - Arm-based color object tracking (arm camera → /TargetAngle)
- `arm_mediapipe` - Hand/pose tracking for arm control (arm camera → /TargetAngle)
- `arm_color_transport` - Color-based pick and transport with navigation (arm camera + Nav2)

---

## Conclusion

This test guide covers object tracking testing only: **KCF** (Astra camera, chassis following), **arm_autopilot** (color tracking on arm camera), **MediaPipe** (hand/pose tracking on arm camera), and **arm_color_transport** (color-based pick and transport with navigation). It does not include generic arm control testing; see `ARM_JOYSTICK_CONTROL.md` for that.

For issues or questions, refer to:
- Troubleshooting section above
- Package documentation
- ROS2 topic debugging tools (`ros2 topic`, `ros2 node`, etc.)

---

**Document Version:** 1.0  
**Last Updated:** January 2025  
**Platform:** Yahboom ROSMASTER X3PLUS 6DOF Robot  
**ROS Distribution:** ROS2 Humble
