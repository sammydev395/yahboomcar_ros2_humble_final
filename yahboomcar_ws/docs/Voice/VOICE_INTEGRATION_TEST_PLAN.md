# Voice Integration Test Plan - X3Plus ROS2 Humble

This document outlines the test plan for verifying voice integration on the Yahboom X3Plus robot. It covers hardware verification, node functionality, and integrated movement control.

## 1. Prerequisites

Before testing, ensure the robot is in a safe state (wheels off the ground if testing movement) and the following environment is set:

```bash
cd /root/yahboomcar_ros2_ws_new/yahboomcar_ws
source /opt/ros/humble/setup.bash
source install/setup.bash

# Ensure PYTHONPATH includes the required libraries for speech and rosmaster
export PYTHONPATH=/root/software/py_install_V3.3.1/build/lib:/root/software/py_install_V0.0.1/py_install/build/lib:$PYTHONPATH
```

---

## 2. Phase 1: Hardware & Node Verification

### Test 1.1: Speech Module Hardware
**Purpose:** Verify the speech recognition hardware is connected and accessible.

**Steps:**
1. Check if the speech module device exists:
   ```bash
   ls /dev/myspeech
   # Or check the serial port directly
   ls /dev/ttyUSB2
   ```
2. Check for speech-related packages:
   ```bash
   ros2 pkg list | grep voice
   ```

**Expected Results:**
- `/dev/myspeech` symlink exists (usually points to `/dev/ttyUSB2`).
- `yahboomcar_voice_ctrl` package is installed.

### Test 1.2: Voice Control Driver (Stationary)
**Purpose:** Verify the voice control node starts and communicates with the hardware.

**Command:**
```bash
ros2 run yahboomcar_voice_ctrl Voice_Ctrl_Mcnamu_driver_X3
```

**Verify:**
1. Check if the node is running:
   ```bash
   ros2 node list | grep driver
   ```
2. Inspect node info:
   ```bash
   ros2 node info /driver_node
   ```

**Expected Results:**
- Node `/driver_node` (or `/voice_control`) is active.
- Node publishes `/voltage`, `/imu/data_raw`, etc.
- Robot provides audio feedback ("Robot is talking/responding").

---

## 3. Phase 2: Integrated Voice Control (Chassis & Arm)

### Test 2.1: Full Bringup with Voice
**Purpose:** Verify the unified voice control node can control multiple systems.

**Command:**
```bash
ros2 launch yahboomcar_bringup yahboomcar_bringup_X3plus_voice_launch.py
```

**Verify Topics:**
```bash
ros2 topic list | grep -E 'cmd_vel|TargetAngle|RGBLight|Buzzer'
```

---

## 4. Phase 3: Command Recognition Tests

Perform the following voice commands and verify the robot's physical response and topic output.

### 3.1 Chassis Movement Commands
*⚠️ Ensure 2m clear radius or robot is on blocks.*

| Voice Command | Code | Expected Behavior | Topic Verification |
|---------------|------|-------------------|-------------------|
| Stop          | 2/0  | Robot stops       | `ros2 topic echo /cmd_vel` (all 0s) |
| Forward       | 4    | Moves forward 5s  | `linear.x > 0` |
| Backward      | 5    | Moves backward 5s | `linear.x < 0` |
| Turn Left     | 6    | Turns left 5s     | `angular.z > 0` |
| Turn Right    | 7    | Turns right 5s    | `angular.z < 0` |

### 3.2 Arm & Gripper Commands
*⚠️ Ensure arm has clear range of motion.*

| Voice Command  | Code | Expected Behavior | Topic Verification |
|----------------|------|-------------------|-------------------|
| Arm Init Pose  | 49   | Moves to [90, 145, 0, 0, 90, 31] | `ros2 topic echo /TargetAngle` |
| Arm Up         | 39   | Arm raises        | `TargetAngle` values update |
| Arm Down       | 40   | Arm lowers        | `TargetAngle` values update |
| Gripper Open   | 44   | Gripper opens     | `TargetAngle` (joint 6 ~35) |
| Gripper Close  | 43   | Gripper closes    | `TargetAngle` (joint 6 ~150) |
| Arm Dance      | 52   | Performs sequence | Series of `TargetAngle` msgs |

### 3.3 Auxiliary Functions
| Voice Command    | Code | Expected Behavior | Topic Verification |
|------------------|------|-------------------|-------------------|
| Close Light      | 10   | RGB lights off    | `ros2 topic echo /RGBLight` |
| Red/Green Light  | 11/12| Color changes     | `RGBLight` updates |
| Display Battery  | 18   | Lights show level | `RGBLight` updates |

---

## 5. Troubleshooting

- **No Device Found:** Ensure the USB cable is connected. Run `dmesg | tail` to see if the serial converter is recognized.
- **Python Import Errors:** Double-check the `PYTHONPATH` exports. The libraries are usually in `/root/software/`.
- **Permission Denied:** Run `sudo chmod 666 /dev/ttyUSB2`.
- **Poor Recognition:** Test in a quiet environment. Ensure the microphone is not obstructed.
- **Node Crashes:** Check for serial communication errors in the terminal output.

---

## 6. Advanced Integration (Launch Catalog)

The following specialized voice tests can be performed using dedicated launch files:

- **Line Following:** `ros2 launch yahboomcar_voice_ctrl voice_ctrl_followline.launch`
- **Color Tracking:** `ros2 launch yahboomcar_voice_ctrl voice_ctrl_colorTracker.launch`
- **Navigation/Transport:** `ros2 launch yahboomcar_voice_ctrl voice_transport_base.launch`
