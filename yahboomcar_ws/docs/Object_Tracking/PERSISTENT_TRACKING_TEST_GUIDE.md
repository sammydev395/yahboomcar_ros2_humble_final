# Persistent Tracking Test Guide
## Yahboom ROSMASTER X3PLUS 6DOF Robot

**Date:** January 2025  
**Platform:** Yahboom ROSMASTER X3PLUS 6DOF Robot  
**ROS Distribution:** ROS2 Humble  
**Camera:** Orbbec Astra Pro Plus Depth Camera  
**Tracking Method:** KCF (Kernelized Correlation Filter) Tracker

---

## Overview

This guide provides comprehensive testing procedures for persistent object tracking using the KCF tracker on the Yahboom ROSMASTER X3PLUS 6DOF robot. The system enables real-time object tracking with depth sensing and optional arm integration for object manipulation.

### Key Features

- **Persistent Object Tracking:** KCF algorithm maintains object identity across frames
- **Depth Integration:** Uses Astra Pro Plus depth camera for distance measurement
- **Robot Following:** Chassis movement to follow tracked objects
- **Arm Integration:** Optional arm control for object manipulation
- **Visual Feedback:** Real-time visualization of tracking status

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

```
Astra Pro Plus Camera
    ├── /camera/rgb/image_raw → KCF Tracker
    ├── /camera/depth/image_raw → KCF Tracker (distance measurement)
    └── /camera/depth/points → Optional (point cloud)

KCF Tracker Node
    ├── Subscribes: /camera/rgb/image_raw, /camera/depth/image_raw, /JoyState
    ├── Publishes: /KCF_image (visualization), /cmd_vel (robot movement)
    └── Output: Tracking box, object center, distance

Robot Control
    ├── /cmd_vel → driver_node → Robot chassis movement
    └── /TargetAngle → driver_node → Arm control (optional)
```

### Key Topics

| Topic | Type | Purpose |
|-------|------|---------|
| `/camera/rgb/image_raw` | `sensor_msgs/Image` | RGB camera feed for tracking |
| `/camera/depth/image_raw` | `sensor_msgs/Image` | Depth data for distance measurement |
| `/JoyState` | `std_msgs/Bool` | Safety override (must be `true` for movement) |
| `/KCF_image` | `sensor_msgs/Image` | Tracking visualization output |
| `/cmd_vel` | `geometry_msgs/Twist` | Robot movement commands |
| `/TargetAngle` | `yahboomcar_msgs/ArmJoint` | Arm joint control (optional) |

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
   ros2 topic list | grep camera
   # Expected:
   # /camera/rgb/image_raw
   # /camera/depth/image_raw
   # /camera/ir/image_raw
   # /camera/depth/points
   ```

4. **Check image publishing:**
   ```bash
   ros2 topic hz /camera/rgb/image_raw
   # Expected: ~30 Hz
   ```

5. **View camera feed (optional):**
   ```bash
   ros2 run image_view image_view --ros-args -r image:=/camera/rgb/image_raw
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

### Phase 4: Arm Integration (Optional)

#### Test 4.1: Verify Arm Control System

**Purpose:** Ensure arm control is available for integration.

**Steps:**

1. **Check arm topics:**
   ```bash
   ros2 topic list | grep -i arm
   # Expected:
   # /TargetAngle
   # /ArmAngleUpdate
   # /joint_states
   ```

2. **Verify arm service:**
   ```bash
   ros2 service list | grep -i arm
   # Expected:
   # /CurrentAngle
   ```

3. **Get current arm position:**
   ```bash
   ros2 service call /CurrentAngle yahboomcar_msgs/srv/RobotArmArray "{apply: 'GetArmJoints'}"
   # Expected: Array of 6 joint angles
   ```

**Expected Results:**
- ✅ All arm topics available
- ✅ Arm service responds
- ✅ Current joint angles reported

---

#### Test 4.2: Arm Tracking Integration (Advanced)

**Purpose:** Test arm movement to point at tracked object.

**⚠️ Note:** This requires custom integration code. Basic KCF tracker does not include arm control.

**If arm tracking integration exists:**

1. **Launch system with arm integration:**
   ```bash
   # Use appropriate launch file if available
   ros2 launch <package> <arm_tracking_launch>.launch.py
   ```

2. **Initialize tracker** on object

3. **Verify arm movement:**
   - Arm should move to point at object
   - Monitor `/TargetAngle` topic:
     ```bash
     ros2 topic echo /TargetAngle
     # Should show arm joint commands
     ```

4. **Test arm following:**
   - Move object, arm should track
   - Verify smooth movement
   - Check for joint limits

**Expected Results:**
- ✅ Arm points at tracked object
- ✅ Smooth arm movement
- ✅ No joint limit violations
- ✅ Arm and chassis can work together

**Note:** If arm tracking integration doesn't exist, this would require:
- Custom node subscribing to KCF tracker output
- Inverse kinematics calculation
- Publishing to `/TargetAngle` topic

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

### Phase 4: Arm Integration (if applicable)
- [ ] Arm topics available
- [ ] Arm service responds
- [ ] Arm tracking integration works (if implemented)

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

### Related Packages
- `yahboomcar_bringup` - Base robot driver
- `yahboomcar_ctrl` - Joystick control
- `yahboomcar_astra` - Camera and tracking
- `orbbec_camera` - Camera driver

---

## Conclusion

This test guide provides comprehensive procedures for testing persistent object tracking on the Yahboom ROSMASTER X3PLUS 6DOF robot. The KCF tracker provides robust, real-time tracking with depth integration and robot following capabilities.

For issues or questions, refer to:
- Troubleshooting section above
- Package documentation
- ROS2 topic debugging tools (`ros2 topic`, `ros2 node`, etc.)

---

**Document Version:** 1.0  
**Last Updated:** January 2025  
**Platform:** Yahboom ROSMASTER X3PLUS 6DOF Robot  
**ROS Distribution:** ROS2 Humble
