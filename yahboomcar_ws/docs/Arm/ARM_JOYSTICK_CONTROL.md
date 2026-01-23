# Arm Control by Joystick

## Prerequisites
```bash
cd /home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws
source /opt/ros/humble/setup.bash
source install/setup.bash
```

---

## Joystick Button Controls (Jetson Controller)

| Control | Button/Axis | Function | Details |
|---------|-------------|----------|---------|
| **Chassis Control** | | | |
| Forward/Back/Strafe | Left Stick (axes[0,1]) | Chassis movement | Forward/backward and left/right strafe |
| Rotation | Right Stick (axes[2,3]) | Chassis rotation | Rotate robot in place |
| **Arm Control** | | | |
| Joint 1 Down | Button 1 (B) | Arm joint 1 down | Lower joint 1 |
| Joint 1 Up | Button 3 (X) | Arm joint 1 up | Raise joint 1 |
| Joint 2 Down | Button 0 (A) | Arm joint 2 down | Lower joint 2 |
| Joint 2 Up | Button 4 (Y) | Arm joint 2 up | Raise joint 2 |
| Joints 3/4 | D-pad (axes[6,7]) | Arm joints 3/4 | Control joints 3 and 4 |
| **Gripper Control** | | | |
| Mode Toggle | Button 10 (SELECT) | Toggle gripper/joint 5 mode | Switch between gripper and joint 5 control |
| Open / Joint 5 Up | Button 6 (L1) | Gripper open or joint 5 up | Opens gripper (gripper mode) or raises joint 5 (joint mode) |
| Close / Joint 5 Down | Axes 5 (L2) | Gripper close or joint 5 down | Closes gripper (gripper mode) or lowers joint 5 (joint mode) |
| **Auxiliary Controls** | | | |
| RGB Light | Button 7 (R1) | RGB Light effects | Cycles through 6 effects: 0=stop, 1=flowing, 2=running, 3=breathing, 4=gradient, 5=starlight |
| Buzzer | Button 11 (START) | Buzzer toggle | Toggle buzzer on/off |

**Note:** RGB light can also be controlled via joystick button (verified during testing)

**Notes:**
- Controller node works correctly
- Full joystick control verified: chassis (wheels) and arm both respond correctly
- RGB light and buzzer can be controlled via joystick buttons
- Test performed with robot wheels off ground for safety
- Arm movement verified via joystick (will test more detailed arm movements later)

---

## Test 3.2: Arm Joint States
**Purpose:** Verify arm position feedback

**Prerequisites:** Driver node should be running from Test 1.4. If not:
```bash
ros2 run yahboomcar_bringup Mcnamu_driver_X3plus
```

### Step 1: Check Topic Exists
**Command:**
```bash
ros2 topic list | grep joint_states
```

**Expected Output:**
```
/joint_states
```

### Step 2: Check Topic Info
**Command:**
```bash
ros2 topic info /joint_states
```

**Expected Output:**
```
Type: sensor_msgs/msg/JointState
Publisher count: 1
Subscription count: 0
```

### Step 3: Check Publish Rate
**Command:**
```bash
ros2 topic hz /joint_states
```

**Expected Output:**
```
average rate: 32.237
	min: 0.005s max: 0.053s std dev: 0.01566s window: 99
```

**Expected:** Publish rate ~10-35 Hz (typically ~30 Hz)

### Step 4: Verify Joint Names
**Command:**
```bash
ros2 topic echo /joint_states --once | grep -A 15 "name:"
```

**Expected Output:**
```
name:
- arm_joint1
- arm_joint2
- arm_joint3
- arm_joint4
- arm_joint5
- grip_joint
```

**Expected:** 6 joint names: arm_joint1-5, grip_joint

### Step 5: Verify Joint States Data
**Command:**
```bash
ros2 topic echo /joint_states --once
```

**Expected Output:**
```yaml
header:
  stamp:
    sec: 1767653886
    nanosec: 716023825
  frame_id: joint_states
name:
- arm_joint1
- arm_joint2
- arm_joint3
- arm_joint4
- arm_joint5
- grip_joint
position:
- -0.733038152679564
- 0.4049165995155717
- -1.5219271376996177
- 0.03141584664106523
- 0.0
- -1.0262536001726656
velocity: []
effort: []
```

**Expected:**
- [x] 6 joint names: arm_joint1-5, grip_joint
- [x] Position values in radians
- [x] Values match physical arm position
- [x] Topic publishing at ~30-32 Hz
- [x] Publisher: `/driver_node`

**Status: ✅ PASSED**

**Test Results:**
- **Topic:** `/joint_states` publishing at ~30-32 Hz
- **Publisher:** `/driver_node` (from `Mcnamu_driver_X3plus`)
- **Message Type:** `sensor_msgs/msg/JointState`
- **Joint Names (6 total):**
  1. `arm_joint1` - Base rotation
  2. `arm_joint2` - Shoulder
  3. `arm_joint3` - Elbow
  4. `arm_joint4` - Wrist
  5. `arm_joint5` - Wrist rotation
  6. `grip_joint` - Gripper
- **Data Format:**
  - Positions: Radians (float values)
  - Velocity: Empty array (not published)
  - Effort: Empty array (not published)
- **Position Values:** Represent current arm joint angles in radians
- **Verification:** Joint states match physical arm position

---

## Test 3.3: Arm Current Angle Service
**Purpose:** Query actual arm positions

**Prerequisites:** Driver node must be running (from Test 1.4 or Test 3.2)

### Step 1: Check Service Exists
**Command:**
```bash
ros2 service list | grep CurrentAngle
```

**Expected Output:**
```
/CurrentAngle
```

### Step 2: Check Service Type
**Command:**
```bash
ros2 service type /CurrentAngle
```

**Expected Output:**
```
yahboomcar_msgs/srv/RobotArmArray
```

### Step 3: Call Service to Get Current Angles
**Command:**
```bash
ros2 service call /CurrentAngle yahboomcar_msgs/srv/RobotArmArray "{apply: 'GetArmJoints'}"
```

**Expected Output:**
```
waiting for service to become available...
requester: making request: yahboomcar_msgs.srv.RobotArmArray_Request(apply='GetArmJoints')

response:
yahboomcar_msgs.srv.RobotArmArray_Response(angles=[48.0, 113.0, 2.0, 91.0, 90.0, 82.0])
```

**Expected:**
- [x] Service `/CurrentAngle` is available
- [x] Returns 6 angle values in degrees
- [x] Values represent current arm position (may vary from initial position)
- [x] Response format: `angles=[<float>, <float>, <float>, <float>, <float>, <float>]`

**Status: ✅ PASSED**

**Test Results:**
- **Service:** `/CurrentAngle` (yahboomcar_msgs/srv/RobotArmArray)
- **Service Provider:** `/driver_node` (from `Mcnamu_driver_X3plus`)
- **Request:** `{apply: 'GetArmJoints'}`
- **Response:** Array of 6 angles in degrees
  - `angles[0]` - arm_joint1 (base rotation)
  - `angles[1]` - arm_joint2 (shoulder)
  - `angles[2]` - arm_joint3 (elbow)
  - `angles[3]` - arm_joint4 (wrist)
  - `angles[4]` - arm_joint5 (wrist rotation)
  - `angles[5]` - grip_joint (gripper)
- **Example Response:** `angles=[48.0, 113.0, 2.0, 91.0, 90.0, 82.0]`
- **Note:** Values represent current arm position, not necessarily initial position. Initial position is approximately [90, 145, 0, 45, 90, 30] degrees.

---

## Test 3.4: Arm Single Joint Movement
**Purpose:** Move individual joints carefully

**⚠️ SAFETY: Clear area around arm before testing! Ensure no obstacles in arm's range of motion.**

**Prerequisites:** Driver node must be running (from Test 1.4 or Test 3.2)

### Step 1: Verify TargetAngle Topic
**Command:**
```bash
ros2 topic info /TargetAngle
```

**Expected Output:**
```
Type: yahboomcar_msgs/msg/ArmJoint
Publisher count: 1
Subscription count: 2
```

**Note:** Subscribers should include `/driver_node` (arm controller).

### Step 2: Get Current Arm Position
**Command:**
```bash
ros2 service call /CurrentAngle yahboomcar_msgs/srv/RobotArmArray "{apply: 'GetArmJoints'}"
```

**Expected Output:**
```
response:
yahboomcar_msgs.srv.RobotArmArray_Response(angles=[<current_values>])
```

**Note:** Record current position to verify return movement.

### Step 3: Move Joint 1 (Base Rotation) - Small Movement
**Command:**
```bash
ros2 topic pub /TargetAngle yahboomcar_msgs/msg/ArmJoint "{id: 1, angle: 100.0, run_time: 1000}" --once
```

**Expected Output:**
```
publisher: beginning loop
publishing #1: yahboomcar_msgs.msg.ArmJoint(id=1, angle=100.0, run_time=1000, joints=[])
```

**Expected:** Joint 1 (base) rotates ~10 degrees from current position. Movement should be smooth over 1 second.

### Step 4: Return Joint 1 to Center
**Command:**
```bash
ros2 topic pub /TargetAngle yahboomcar_msgs/msg/ArmJoint "{id: 1, angle: 90.0, run_time: 1000}" --once
```

**Expected:** Joint 1 returns to ~90 degrees (center position). Smooth movement over 1 second.

### Step 5: Move Joint 2 (Shoulder) - Small Movement
**Command:**
```bash
ros2 topic pub /TargetAngle yahboomcar_msgs/msg/ArmJoint "{id: 2, angle: 100.0, run_time: 1000}" --once
```

**Expected Output:**
```
publisher: beginning loop
publishing #1: yahboomcar_msgs.msg.ArmJoint(id=2, angle=100.0, run_time=1000, joints=[])
```

**Expected:** Joint 2 (shoulder) moves slightly. Movement should be smooth over 1 second.

### Step 6: Return Joint 2
**Command:**
```bash
ros2 topic pub /TargetAngle yahboomcar_msgs/msg/ArmJoint "{id: 2, angle: 145.0, run_time: 1000}" --once
```

**Expected:** Joint 2 returns to ~145 degrees. Smooth movement over 1 second.

### Step 7: Verify Joint States Updated
**Command:**
```bash
ros2 topic echo /joint_states --once | grep -A 6 "position:"
```

**Expected Output:**
```
position:
- <updated_value>  # arm_joint1
- <updated_value>  # arm_joint2
- <updated_value>  # arm_joint3
- <updated_value>  # arm_joint4
- <updated_value>  # arm_joint5
- <updated_value>  # grip_joint
```

**Expected:**
- [x] Joint 1 rotates base ~10 degrees when commanded
- [x] Joint 2 moves shoulder when commanded
- [x] Smooth movement (1 second duration per command)
- [x] Joint states reflect position changes
- [x] No grinding or stalling sounds
- [x] Arm returns to commanded positions

**Status: ✅ PASSED**

**Test Results:**
- **Single Joint Movement:** ✅ Working correctly
- **Joint 1 (Base):** Rotates smoothly when commanded (tested: 90° → 100° → 90°)
- **Joint 2 (Shoulder):** Moves smoothly when commanded (tested: 145° → 100° → 145°)
- **Movement Duration:** Commands execute over specified `run_time` (1000ms = 1 second)
- **Joint States:** Position updates reflected in `/joint_states` topic
- **No Errors:** Smooth operation, no grinding or stalling sounds

**Test Results:**
- **Topic:** `/TargetAngle` (yahboomcar_msgs/msg/ArmJoint)
- **Command Format:**
  - Single joint: `{id: <1-6>, angle: <degrees>, run_time: <ms>}`
  - Joint IDs: 1=base, 2=shoulder, 3=elbow, 4=wrist, 5=wrist_rotation, 6=gripper
- **Movement Characteristics:**
  - Smooth, controlled motion
  - Movement duration matches `run_time` parameter (1000ms = 1 second)
  - Joint states update to reflect new positions
- **Safety:** Small movements tested, arm returns to safe positions
- **Verification:** Joint states topic confirms position changes

---

## Test 3.5: Gripper Open/Close
**Purpose:** Test gripper control

**⚠️ SAFETY: Ensure nothing is in gripper's path before testing!**

**Prerequisites:** Driver node must be running (from Test 1.4 or Test 3.2)

### Step 1: Get Current Gripper Position
**Command:**
```bash
ros2 service call /CurrentAngle yahboomcar_msgs/srv/RobotArmArray "{apply: 'GetArmJoints'}" | grep angles
```

**Expected Output:**
```
angles=[<j1>, <j2>, <j3>, <j4>, <j5>, <gripper>]
```

**Note:** Last value (index 5) is gripper position in degrees.

### Step 2: Open Gripper (Joint 6)
**Command:**
```bash
ros2 topic pub /TargetAngle yahboomcar_msgs/msg/ArmJoint "{id: 6, angle: 30.0, run_time: 500}" --once
```

**Expected Output:**
```
publisher: beginning loop
publishing #1: yahboomcar_msgs.msg.ArmJoint(id=6, angle=30.0, run_time=500, joints=[])
```

**Expected:** Gripper opens fully. Movement should be smooth over 0.5 seconds (500ms).

### Step 3: Wait for Movement to Complete
**Command:**
```bash
sleep 2
```

**Expected:** Wait time allows gripper to fully open and stabilize.

### Step 4: Close Gripper (Joint 6)
**Command:**
```bash
ros2 topic pub /TargetAngle yahboomcar_msgs/msg/ArmJoint "{id: 6, angle: 150.0, run_time: 500}" --once
```

**Expected Output:**
```
publisher: beginning loop
publishing #1: yahboomcar_msgs.msg.ArmJoint(id=6, angle=150.0, run_time=500, joints=[])
```

**Expected:** Gripper closes fully. Movement should be smooth over 0.5 seconds (500ms).

### Step 5: Verify Gripper Position
**Command:**
```bash
ros2 topic echo /joint_states --once | grep -A 1 "grip_joint"
```

**Expected Output:**
```
- grip_joint
position:
- <gripper_angle_in_radians>
```

**Expected:**
- [x] Gripper opens fully (angle ~30 degrees)
- [x] Gripper closes fully (angle ~150 degrees)
- [x] Smooth movement (0.5 second duration per command)
- [x] No grinding or stalling sounds
- [x] Joint states reflect gripper position changes

**Status: ✅ PASSED**

**Test Results:**
- **Gripper Control:** ✅ Working correctly
- **Open Position:** Gripper opens to ~30 degrees when commanded
- **Close Position:** Gripper closes to ~150 degrees when commanded
- **Movement Duration:** Commands execute over specified `run_time` (500ms = 0.5 seconds)
- **Smooth Operation:** No grinding or stalling sounds
- **Position Feedback:** Joint states reflect gripper position changes

**Test Results:**
- **Gripper Control:** Joint ID 6 (grip_joint)
- **Open Position:** ~30 degrees (fully open)
- **Close Position:** ~150 degrees (fully closed)
- **Movement Duration:** 500ms (0.5 seconds) per command
- **Movement Characteristics:**
  - Smooth, controlled motion
  - No grinding or stalling sounds
  - Position feedback via joint_states topic
- **Verification:** Joint states topic confirms gripper position changes

---

## Test 3.6: Arm Full Position Command
**Purpose:** Move all joints simultaneously

**⚠️ SAFETY: Clear large area around arm! All joints will move together. Ensure no obstacles in full arm range of motion.**

**Prerequisites:** Driver node must be running (from Test 1.4 or Test 3.2)

### Step 1: Get Current Arm Position
**Command:**
```bash
ros2 service call /CurrentAngle yahboomcar_msgs/srv/RobotArmArray "{apply: 'GetArmJoints'}"
```

**Expected Output:**
```
response:
yahboomcar_msgs.srv.RobotArmArray_Response(angles=[<current_values>])
```

**Note:** Record current position to verify return movement.

### Step 2: Move to Neutral/Safe Position (All Joints)
**Command:**
```bash
ros2 topic pub /TargetAngle yahboomcar_msgs/msg/ArmJoint "{joints: [90.0, 90.0, 90.0, 90.0, 90.0, 90.0], run_time: 1500}" --once
```

**Expected Output:**
```
publisher: beginning loop
publishing #1: yahboomcar_msgs.msg.ArmJoint(id=0, angle=0.0, run_time=1500, joints=[90.0, 90.0, 90.0, 90.0, 90.0, 90.0])
```

**Expected:** All 6 joints move together to neutral position (90 degrees each). Smooth coordinated motion over 1.5 seconds (1500ms).

**Joint Positions:**
- arm_joint1: 90.0° (base centered)
- arm_joint2: 90.0° (shoulder)
- arm_joint3: 90.0° (elbow)
- arm_joint4: 90.0° (wrist)
- arm_joint5: 90.0° (wrist rotation)
- grip_joint: 90.0° (gripper)

### Step 3: Wait for Movement to Complete
**Command:**
```bash
sleep 2
```

**Expected:** Wait time allows all joints to reach position and stabilize.

### Step 4: Verify Joint States
**Command:**
```bash
ros2 topic echo /joint_states --once | grep -A 6 "position:"
```

**Expected Output:**
```
position:
- 1.5707963267948966  # arm_joint1 (~90° in radians)
- 1.5707963267948966  # arm_joint2 (~90° in radians)
- 1.5707963267948966  # arm_joint3 (~90° in radians)
- 1.5707963267948966  # arm_joint4 (~90° in radians)
- 1.5707963267948966  # arm_joint5 (~90° in radians)
- 1.5707963267948966  # grip_joint (~90° in radians)
```

**Note:** 90 degrees = π/2 radians ≈ 1.5708 radians

### Step 5: Return to Initial Reach Position (All Joints)
**Command:**
```bash
ros2 topic pub /TargetAngle yahboomcar_msgs/msg/ArmJoint "{joints: [90.0, 145.0, 0.0, 45.0, 90.0, 30.0], run_time: 1500}" --once
```

**Expected Output:**
```
publisher: beginning loop
publishing #1: yahboomcar_msgs.msg.ArmJoint(id=0, angle=0.0, run_time=1500, joints=[90.0, 145.0, 0.0, 45.0, 90.0, 30.0])
```

**Expected:** All 6 joints move together to initial reach position. Smooth coordinated motion over 1.5 seconds (1500ms).

**Joint Positions:**
- arm_joint1: 90.0° (base centered)
- arm_joint2: 145.0° (shoulder extended)
- arm_joint3: 0.0° (elbow)
- arm_joint4: 45.0° (wrist)
- arm_joint5: 90.0° (wrist rotation)
- grip_joint: 30.0° (gripper open)

### Step 6: Verify Final Position
**Command:**
```bash
ros2 service call /CurrentAngle yahboomcar_msgs/srv/RobotArmArray "{apply: 'GetArmJoints'}"
```

**Expected Output:**
```
response:
yahboomcar_msgs.srv.RobotArmArray_Response(angles=[90.0, 145.0, 0.0, 45.0, 90.0, 30.0])
```

**Expected:**
- [x] All 6 joints move together when commanded
- [x] Smooth coordinated motion (1.5 second duration)
- [x] Joint states reflect all position changes
- [x] No grinding or stalling sounds
- [x] Arm reaches commanded positions accurately

**Status: ✅ PASSED**

**Test Results:**
- **Full Position Command:** ✅ Working correctly
- **Neutral Position:** All joints move to [90°, 90°, 90°, 90°, 90°, 90°] successfully
- **Initial Reach Position:** All joints move to [90°, 145°, 0°, 45°, 90°, 30°] successfully
- **Coordinated Motion:** All 6 joints move simultaneously and smoothly
- **Movement Duration:** Commands execute over specified `run_time` (1500ms = 1.5 seconds)
- **Position Accuracy:** Arm reaches commanded positions (verified via CurrentAngle service)
- **No Errors:** Smooth operation, no grinding or stalling sounds
- **Bug Fix Applied:** Fixed driver node crash when receiving `joints` array commands:
  - **Issue:** Line 194 was assigning `self.joints` instead of `msg.joints`
  - **Fix:** Changed to `arm_joint.joints = list(msg.joints)` and added validation/error handling
  - **Result:** Driver node no longer crashes on full position commands

**Test Results:**
- **Command Format:** `{joints: [<j1>, <j2>, <j3>, <j4>, <j5>, <gripper>], run_time: <ms>}`
- **Neutral Position:** All joints at 90° (safe, centered position)
- **Initial Reach Position:** [90°, 145°, 0°, 45°, 90°, 30°] (extended reach configuration)
- **Movement Characteristics:**
  - All joints move simultaneously
  - Smooth, coordinated motion
  - Movement duration matches `run_time` parameter (1500ms = 1.5 seconds)
  - Joint states update to reflect all position changes
- **Verification:** Both joint_states topic and CurrentAngle service confirm position changes
- **Safety:** Arm returns to safe positions after testing

---
