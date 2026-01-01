# X3Plus ROS2 Humble - Comprehensive Test Plan

## Prerequisites
```bash
cd /home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws
source /opt/ros/humble/setup.bash
source install/setup.bash
```

---

# PHASE 1: Non-Movement Sensors & Peripherals
*Robot can stay stationary - safe testing*

---

## Test 1.1: LiDAR
**Purpose:** Verify laser scanner is working
```bash
# Check if LiDAR device exists
ls /dev/ttyUSB*

# Launch LiDAR driver (check which one is used)
ros2 launch yahboomcar_laser laser_X3_launch.py
```
**Verify:**
```bash
# Check topic exists
ros2 topic list | grep scan

# View scan data
ros2 topic echo /scan --once

# Check scan rate
ros2 topic hz /scan
```
**Expected:**
- [ ] `/scan` topic publishing
- [ ] Range values make sense (not all 0 or inf)
- [ ] ~10Hz publish rate
- [ ] No driver errors

**Troubleshooting:**
```bash
# If permission denied
sudo chmod 666 /dev/ttyUSB0

# Check LiDAR is recognized
dmesg | tail -20
```

---

## Test 1.2: Voice Control Module
**Purpose:** Verify speech recognition hardware
```bash
# Check what voice control packages exist
ros2 pkg list | grep voice

# Launch voice control
ros2 launch yahboomcar_voice_ctrl voice_ctrl_X3_launch.py
```
**Verify:**
```bash
# Check topics
ros2 topic list | grep -i voice

# Check if speech module is responding
ros2 topic echo /speech_result
```
**Expected:**
- [ ] Voice control node starts
- [ ] Speech commands recognized
- [ ] No serial communication errors

**Note:** Voice control may require specific Yahboom speech module hardware.

---

## Test 1.3: Astra Depth Camera
**Purpose:** Verify RGB-D camera
```bash
# Check if Astra device exists
ls /dev/astra* 2>/dev/null || ls /dev/bus/usb/*/*

# Launch Astra camera
ros2 launch yahboomcar_astra astra_X3.launch.py
```
**Verify:**
```bash
# Check camera topics
ros2 topic list | grep camera

# Check RGB image
ros2 topic hz /camera/color/image_raw

# Check depth image
ros2 topic hz /camera/depth/image_raw

# Check camera info
ros2 topic echo /camera/color/camera_info --once
```
**Expected:**
- [ ] `/camera/color/image_raw` publishing at ~30Hz
- [ ] `/camera/depth/image_raw` publishing
- [ ] `/camera/depth/points` (point cloud) available
- [ ] No "device not found" errors

**View camera (if display available):**
```bash
# Install if needed: sudo apt install ros-humble-rqt-image-view
ros2 run rqt_image_view rqt_image_view
```

**Troubleshooting:**
```bash
# Check Astra udev rules
cat /etc/udev/rules.d/*orbbec* 2>/dev/null

# Reload udev
sudo udevadm control --reload-rules
sudo udevadm trigger
```

---

## Test 1.4: IMU Data (Driver Only, No Movement)
**Purpose:** Verify IMU sensor without moving robot
```bash
# Start driver (arm will initialize but no chassis movement)
ros2 run yahboomcar_bringup Mcnamu_driver_X3plus
```
**Verify (in another terminal):**
```bash
# Check IMU raw data
ros2 topic echo /pub_imu --once

# Check magnetometer
ros2 topic echo /pub_mag --once

# Check battery voltage
ros2 topic echo /voltage --once

# Check IMU rate
ros2 topic hz /pub_imu
```
**Expected:**
- [ ] Accelerometer shows ~9.8 m/s² on Z axis (gravity)
- [ ] Gyroscope near 0 when stationary
- [ ] Battery voltage 10-12V range
- [ ] ~20Hz publish rate

---

# PHASE 2: Arm Tests (Stationary Robot)
*Robot stays in place - only arm moves*

---

## Test 2.1: Arm Joint States
**Purpose:** Verify arm position feedback
```bash
# Driver should still be running from Test 1.4
# If not: ros2 run yahboomcar_bringup Mcnamu_driver_X3plus
```
**Verify:**
```bash
# Check joint states topic
ros2 topic echo /joint_states --once

# Verify 6 joints listed
ros2 topic echo /joint_states --once | grep -A 10 "name:"
```
**Expected:**
- [ ] 6 joint names: arm_joint1-5, grip_joint
- [ ] Position values in radians
- [ ] Values match physical arm position

---

## Test 2.2: Arm Current Angle Service
**Purpose:** Query actual arm positions
```bash
ros2 service call /CurrentAngle yahboomcar_msgs/srv/RobotArmArray "{apply: 'GetArmJoints'}"
```
**Expected:**
- [ ] Returns 6 angle values in degrees
- [ ] Values approximately [90, 145, 0, 45, 90, 30] (initial position)

---

## Test 2.3: Arm Single Joint Movement
**Purpose:** Move individual joints carefully
```bash
# ⚠️ SAFETY: Clear area around arm before testing!

# Move joint 1 (base rotation) - small movement
ros2 topic pub /TargetAngle yahboomcar_msgs/msg/ArmJoint "{id: 1, angle: 100.0, run_time: 1000}" --once

# Return to center
ros2 topic pub /TargetAngle yahboomcar_msgs/msg/ArmJoint "{id: 1, angle: 90.0, run_time: 1000}" --once

# Move joint 2 (shoulder) - small movement
ros2 topic pub /TargetAngle yahboomcar_msgs/msg/ArmJoint "{id: 2, angle: 100.0, run_time: 1000}" --once

# Return
ros2 topic pub /TargetAngle yahboomcar_msgs/msg/ArmJoint "{id: 2, angle: 145.0, run_time: 1000}" --once
```
**Expected:**
- [ ] Joint 1 rotates base ~10 degrees
- [ ] Joint 2 moves shoulder slightly
- [ ] Smooth movement (1 second duration)

---

## Test 2.4: Gripper Open/Close
**Purpose:** Test gripper control
```bash
# Open gripper (joint 6)
ros2 topic pub /TargetAngle yahboomcar_msgs/msg/ArmJoint "{id: 6, angle: 30.0, run_time: 500}" --once

# Wait and close
sleep 2
ros2 topic pub /TargetAngle yahboomcar_msgs/msg/ArmJoint "{id: 6, angle: 150.0, run_time: 500}" --once
```
**Expected:**
- [ ] Gripper opens fully
- [ ] Gripper closes fully
- [ ] No grinding or stalling sounds

---

## Test 2.5: Arm Full Position Command
**Purpose:** Move all joints simultaneously
```bash
# Neutral/safe position
ros2 topic pub /TargetAngle yahboomcar_msgs/msg/ArmJoint "{joints: [90.0, 90.0, 90.0, 90.0, 90.0, 90.0], run_time: 1500}" --once

# Wait
sleep 2

# Return to initial reach position
ros2 topic pub /TargetAngle yahboomcar_msgs/msg/ArmJoint "{joints: [90.0, 145.0, 0.0, 45.0, 90.0, 30.0], run_time: 1500}" --once
```
**Expected:**
- [ ] All 6 joints move together
- [ ] Smooth coordinated motion

---

## Test 2.6: Arm Camera (Gripper Camera)
**Purpose:** Verify camera mounted on arm/gripper
```bash
# Check USB cameras
ls /dev/video*

# Check camera device
v4l2-ctl --list-devices

# If separate from Astra, test with:
ros2 run usb_cam usb_cam_node_exe --ros-args -p video_device:=/dev/video0
```
**Verify:**
```bash
ros2 topic list | grep -i image
ros2 topic hz /image_raw
```
**Expected:**
- [ ] Camera image topic publishing
- [ ] ~30Hz frame rate

---

# PHASE 3: Chassis Movement Tests
*⚠️ Robot will move - clear area!*

---

## Test 3.1: Buzzer & RGB (Pre-movement check)
**Purpose:** Verify auxiliary controls before movement
```bash
# Buzzer beep (confirms communication)
ros2 topic pub /Buzzer std_msgs/msg/Bool "{data: true}" --once
sleep 1
ros2 topic pub /Buzzer std_msgs/msg/Bool "{data: false}" --once

# RGB light (visual confirmation)
ros2 topic pub /RGBLight std_msgs/msg/Int32 "{data: 2}" --once
```
**Expected:**
- [ ] Buzzer sounds for 1 second
- [ ] RGB lights change color

---

## Test 3.2: Chassis Movement (cmd_vel)
**Purpose:** Test mecanum wheel control
```bash
# ⚠️ SAFETY: Robot on blocks OR clear 2m radius!

# Forward (0.1 m/s for safety)
ros2 topic pub /cmd_vel geometry_msgs/msg/Twist "{linear: {x: 0.1}}" --once
sleep 2
ros2 topic pub /cmd_vel geometry_msgs/msg/Twist "{}" --once  # Stop

# Strafe right (mecanum test)
ros2 topic pub /cmd_vel geometry_msgs/msg/Twist "{linear: {y: -0.1}}" --once
sleep 2
ros2 topic pub /cmd_vel geometry_msgs/msg/Twist "{}" --once

# Rotate
ros2 topic pub /cmd_vel geometry_msgs/msg/Twist "{angular: {z: 0.3}}" --once
sleep 2
ros2 topic pub /cmd_vel geometry_msgs/msg/Twist "{}" --once
```
**Expected:**
- [ ] Robot moves forward
- [ ] Robot strafes sideways (mecanum)
- [ ] Robot rotates in place
- [ ] All 4 wheels respond

---

## Test 3.3: Odometry Feedback
**Purpose:** Verify wheel encoders/odometry
```bash
# Check velocity feedback while robot was moving
ros2 topic echo /pub_vel --once

# Check odometry (if base_node running)
ros2 topic echo /odom --once
```
**Expected:**
- [ ] Velocity values match commanded direction
- [ ] Odometry position updates

---

# PHASE 4: Joystick Integration
*Full control testing*

---

## Test 4.1: Joystick Connection
**Purpose:** Verify controller connected
```bash
# Check joystick device
ls /dev/input/js*

# Test raw joystick
ros2 run joy joy_node &
ros2 topic echo /joy
```
**Expected:**
- [ ] `/dev/input/js0` exists
- [ ] Button/axis values change when pressed

---

## Test 4.2: Full Joystick Control
**Purpose:** Control chassis AND arm with joystick
```bash
# Stop any running nodes
# Launch full X3Plus bringup
ros2 launch yahboomcar_bringup yahboomcar_bringup_X3plus_launch.py
```
**Test Controls:**
1. Press **R2** (or equivalent) to enable control
2. **Left stick** - chassis forward/back/strafe
3. **Right stick** - chassis rotation
4. **A/Y buttons** - arm joint 2
5. **B/X buttons** - arm joint 1
6. **D-pad** - arm joints 3/4
7. **L1/L2** - gripper control

**Expected:**
- [ ] R2 enables movement (buzzer confirms)
- [ ] Left stick controls chassis
- [ ] Arm buttons control joints
- [ ] Gripper opens/closes with L1/L2

---

# PHASE 5: Full System Integration
*All components working together*

---

## Test 5.1: Full Bringup with RViz
```bash
ros2 launch yahboomcar_bringup yahboomcar_bringup_X3plus_launch.py use_rviz:=true
```
**Expected:**
- [ ] Robot model displays correctly
- [ ] TF tree complete (no errors)
- [ ] Arm position matches physical
- [ ] LiDAR scan visible
- [ ] Camera feed visible

---

## Test 5.2: SLAM Mapping
```bash
# Terminal 1: Full bringup
ros2 launch yahboomcar_bringup yahboomcar_bringup_X3plus_launch.py

# Terminal 2: SLAM
ros2 launch yahboomcar_slam slam_X3_launch.py
```
**Expected:**
- [ ] Map builds as robot moves
- [ ] Localization stable

---

## Test 5.3: Arm Applications
```bash
# Test arm autopilot (color following)
ros2 launch arm_autopilot arm_autopilot.launch.py

# Test MediaPipe (hand gesture control)
ros2 launch arm_mediapipe arm_mediapipe.launch.py
```
**Expected:**
- [ ] Color detection working
- [ ] Hand tracking working
- [ ] Arm responds to vision input

---

# Quick Reference: Test Order Checklist

| # | Test | Component | Movement? | Status |
|---|------|-----------|-----------|--------|
| 1.1 | LiDAR | Laser scanner | No | ⬜ |
| 1.2 | Voice Control | Speech module | No | ⬜ |
| 1.3 | Astra Camera | RGB-D | No | ⬜ |
| 1.4 | IMU Data | Sensors | No | ⬜ |
| 2.1 | Arm Joint States | Feedback | No | ⬜ |
| 2.2 | Arm Angle Service | Query | No | ⬜ |
| 2.3 | Arm Single Joint | Arm moves | ⚠️ Arm | ⬜ |
| 2.4 | Gripper | Gripper | ⚠️ Arm | ⬜ |
| 2.5 | Arm Full Position | All joints | ⚠️ Arm | ⬜ |
| 2.6 | Arm Camera | Gripper cam | No | ⬜ |
| 3.1 | Buzzer/RGB | Auxiliary | No | ⬜ |
| 3.2 | Chassis Movement | Wheels | ⚠️ Chassis | ⬜ |
| 3.3 | Odometry | Encoders | After move | ⬜ |
| 4.1 | Joystick | Controller | No | ⬜ |
| 4.2 | Full Joystick | Everything | ⚠️ Full | ⬜ |
| 5.1 | Full Bringup | Integration | ⚠️ Full | ⬜ |
| 5.2 | SLAM | Mapping | ⚠️ Full | ⬜ |
| 5.3 | Arm Apps | Vision+Arm | ⚠️ Arm | ⬜ |

---

# Troubleshooting Quick Reference

### Device not found
```bash
ls /dev/ttyUSB* /dev/ttyACM* /dev/video* /dev/input/js*
sudo dmesg | tail -30
```

### Permission denied
```bash
sudo chmod 666 /dev/ttyUSB0
sudo usermod -a -G dialout $USER
# Logout/login after usermod
```

### Topic not publishing
```bash
ros2 node list
ros2 topic list
ros2 topic info /topic_name
```

### Check message format
```bash
ros2 interface show yahboomcar_msgs/msg/ArmJoint
```
