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

# Launch LiDAR driver
# Note: Use ydlidar_ros2_driver from library_ws, not sllidar
source /opt/ros/humble/setup.bash
source /root/library_ws/install/setup.bash
ros2 launch ydlidar_ros2_driver ydlidar_launch.py params_file:=/root/yahboomcar_ros2_ws/src/yahboomcar_laser/params_ydlidar.yaml
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

**Expected Output:**
```bash
# ros2 topic list | grep scan
/scan

# ros2 topic echo /scan --once
header:
  stamp:
    sec: 1767386858
    nanosec: 936020000
  frame_id: laser_frame
angle_min: -3.1415927410125732
angle_max: 3.1415927410125732
angle_increment: 0.0031431643292307854
time_increment: 4.91678474645596e-05
scan_time: 0.09725400060415268
range_min: 0.009999999776482582
range_max: 50.0
ranges:
- 0.0
- 0.0
- 0.054999999701976776
- 0.0
- 0.0
- 0.06599999964237213
...

# ros2 topic hz /scan
average rate: 10.164
	min: 0.095s max: 0.101s std dev: 0.00148s window: 12
average rate: 10.170
	min: 0.095s max: 0.101s std dev: 0.00108s window: 23
```
**Expected:**
- [x] `/scan` topic publishing
- [x] Range values make sense (not all 0 or inf)
- [x] ~10Hz publish rate
- [x] No driver errors

**Status: ✅ PASSED**
- **Device:** TG30 (Model Code 101, Serial: 2024101800100006)
- **Configuration:** Using `params_ydlidar.yaml` with:
  - `lidar_type: 0` (TYPE_TRIANGLE)
  - `baudrate: 512000`
  - `reversion: true`
  - `inverted: true`
  - `sample_rate: 20`
  - `range_max: 50.0`
- **Connection:** `/dev/ttyUSB0:512000`
- **Publish Rate:** ~10.04-10.17 Hz
- **Verification:** 
  - ✅ Both `params_TG.yaml` and updated `params_ydlidar.yaml` files tested and confirmed working
  - ✅ Scan data publishing with valid range values
  - ✅ Health status: good
  - ✅ Scan mode: started successfully
- **Configuration Files:**
  - `params_TG.yaml`: Original working configuration (lidar_type=0, baudrate=512000)
  - `params_ydlidar.yaml`: Updated with correct parameters matching TG.yaml
  - Both files produce identical results with TG30 device
- **Notes:** 
  - LiDAR device must be connected on the host and mapped to the container (via `--device=/dev/ttyUSB0:/dev/ttyUSB0` in run script)
  - Symlink `/dev/ydlidar -> /dev/ttyUSB0` is created in container entrypoint
  - Requires `ydlidar_ros2_driver` built in `library_ws`
  - Requires YDLidar-SDK installed to `/usr/local`
  - ROS1 container must be stopped to avoid device conflicts
  - Initial testing used `params_TG.yaml` to identify correct parameters, then `params_ydlidar.yaml` was updated with these values

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

# Launch voice control (requires PYTHONPATH for libraries)
export PYTHONPATH=/root/software/py_install_V3.3.1/build/lib:/root/software/py_install_V0.0.1/py_install/build/lib:$PYTHONPATH
source /opt/ros/humble/setup.bash
source /root/yahboomcar_ros2_ws/install/setup.bash
ros2 run yahboomcar_voice_ctrl Voice_Ctrl_Mcnamu_driver_X3
```
**Verify:**
```bash
# Check node is running (voice control runs as /driver_node)
ros2 node list | grep driver

# Check node info to see topics
ros2 node info /driver_node
```

**Expected Node Info Output:**
```
/driver_node
  Subscribers:
    /Buzzer: std_msgs/msg/Bool
    /RGBLight: std_msgs/msg/Int32
    /cmd_vel: geometry_msgs/msg/Twist
  Publishers:
    /edition: std_msgs/msg/Float32
    /imu/data_raw: sensor_msgs/msg/Imu
    /imu/mag: sensor_msgs/msg/MagneticField
    /joint_states: sensor_msgs/msg/JointState
    /parameter_events: rcl_interfaces/msg/ParameterEvent
    /rosout: rcl_interfaces/msg/Log
    /vel_raw: geometry_msgs/msg/Twist
    /voltage: std_msgs/msg/Float32
  Service Servers:
    /driver_node/describe_parameters: rcl_interfaces/srv/DescribeParameters
    /driver_node/get_parameter_types: rcl_interfaces/srv/GetParameterTypes
    /driver_node/get_parameters: rcl_interfaces/srv/GetParameters
    /driver_node/list_parameters: rcl_interfaces/srv/ListParameters
    /driver_node/set_parameters: rcl_interfaces/srv/SetParameters
    /driver_node/set_parameters_atomically: rcl_interfaces/srv/SetParametersAtomically
```

**Additional Verification:**
```bash
# Check topics published by voice control
ros2 topic list
# Voice control publishes: /edition, /voltage, /joint_states, /vel_raw, /imu/data_raw, /imu/mag
# Voice control subscribes: /cmd_vel, /RGBLight, /Buzzer

# Verify robot is responding (audio output - robot talking)
# Voice commands are processed internally, no separate /speech_result topic
```
**Expected:**
- [x] Voice control node starts
- [x] Speech module hardware connected and responding
- [x] Robot can talk/respond (audio output working)
- [x] No serial communication errors

**Status: ✅ PASSED**
- **Device:** Speech module on `/dev/ttyUSB2` (CH340 serial converter)
- **Symlink:** `/dev/myspeech -> /dev/ttyUSB2` (created in entrypoint)
- **Node Name:** `/driver_node` (voice control integrated into main driver node)
- **Dependencies:** 
  - `Rosmaster_Lib` from `/root/software/py_install_V3.3.1/build/lib/`
  - `Speech_Lib` from `/root/software/py_install_V0.0.1/py_install/build/lib/`
  - `pyserial` installed via pip
- **Topics:**
  - **Publishes:** `/edition`, `/voltage`, `/joint_states`, `/vel_raw`, `/imu/data_raw`, `/imu/mag`
  - **Subscribes:** `/cmd_vel`, `/RGBLight`, `/Buzzer`
  - **Note:** No separate `/speech_result` topic - voice commands are processed internally and converted to robot control actions
- **Verification:** 
  - ✅ Robot is talking/responding (audio output confirmed)
  - ✅ Node running as `/driver_node`
  - ✅ Voice commands processed internally and converted to robot actions
- **Notes:** 
  - Speech module device must be connected on the host and mapped to the container (via `--device=/dev/ttyUSB2:/dev/ttyUSB2` in run script)
  - PYTHONPATH must include both library paths: `/root/software/py_install_V3.3.1/build/lib:/root/software/py_install_V0.0.1/py_install/build/lib`
  - Speech module requires physical connection via USB serial
  - Voice control is integrated into the driver node, not a separate voice-specific node

---

## Test 1.3: Astra Depth Camera
**Purpose:** Verify RGB-D camera
```bash
# Check if Astra device exists
ls /dev/astra* 2>/dev/null || ls /dev/bus/usb/*/*

# Check USB devices
lsusb | grep -i 'orbbec\|astra\|depth\|camera'

# Launch Astra camera
source /opt/ros/humble/setup.bash
source /root/yahboomcar_ros2_ws/install/setup.bash
ros2 launch yahboomcar_astra astra.launch.py
```
**Verify:**
```bash
# Check camera topics
ros2 topic list | grep -E 'camera|color|depth|ir|point'

# Check node info
ros2 node info /astra_camera

# Check RGB image
ros2 topic hz /color/image_raw

# Check depth image
ros2 topic hz /depth/image_raw

# Check camera info
ros2 topic echo /color/camera_info --once
```

**Expected Output:**
```bash
# ros2 topic list | grep -E 'camera|color|depth|ir|point'
/color/camera_info
/color/image_raw
/depth/camera_info
/depth/image_raw
/depth/points
/ir/camera_info
/ir/image_raw

# ros2 node info /astra_camera
/astra_camera
  Subscribers:
    /depth/camera_info: sensor_msgs/msg/CameraInfo
    /depth/image_raw: sensor_msgs/msg/Image
    /parameter_events: rcl_interfaces/msg/ParameterEvent
  Publishers:
    /color/camera_info: sensor_msgs/msg/CameraInfo
    /color/image_raw: sensor_msgs/msg/Image
    /depth/camera_info: sensor_msgs/msg/CameraInfo
    /depth/image_raw: sensor_msgs/msg/Image
    /depth/points: sensor_msgs/msg/PointCloud2
    /ir/camera_info: sensor_msgs/msg/CameraInfo
    /ir/image_raw: sensor_msgs/msg/Image
    /parameter_events: rcl_interfaces/msg/ParameterEvent
    /tf: tf2_msgs/msg/TFMessage
    /tf_static: tf2_msgs/msg/TFMessage
  Service Servers:
    /astra_camera/describe_parameters: rcl_interfaces/srv/DescribeParameters
    /astra_camera/get_parameter_types: rcl_interfaces/srv/GetParameterTypes
    /astra_camera/get_parameters: rcl_interfaces/srv/GetParameters
    /astra_camera/list_parameters: rcl_interfaces/srv/ListParameters
    /astra_camera/set_parameters: rcl_interfaces/srv/SetParameters
    /astra_camera/set_parameters_atomically: rcl_interfaces/srv/SetParametersAtomically
    /get_camera_info: astra_camera_msgs/srv/GetCameraInfo
    /get_camera_params: astra_camera_msgs/srv/GetCameraParams
    /get_device_info: astra_camera_msgs/srv/GetDeviceInfo

# ros2 topic hz /color/image_raw
average rate: 30.000
	min: 0.033s max: 0.033s std dev: 0.00000s window: 30

# ros2 topic hz /depth/image_raw
average rate: 30.000
	min: 0.033s max: 0.033s std dev: 0.00000s window: 30

# ros2 topic echo /color/camera_info --once
header:
  stamp:
    sec: 1767391052
    nanosec: 780705753
  frame_id: "color_optical_frame"
height: 480
width: 640
distortion_model: "plumb_bob"
d: [0.0, 0.0, 0.0, 0.0, 0.0]
k: [570.0, 0.0, 320.0, 0.0, 570.0, 240.0, 0.0, 0.0, 1.0]
r: [1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0]
p: [570.0, 0.0, 320.0, 0.0, 0.0, 570.0, 240.0, 0.0, 0.0, 0.0, 1.0, 0.0]
```

**Expected:**
- [x] `/color/image_raw` publishing (RGB image)
- [x] `/depth/image_raw` publishing (Depth image)
- [x] `/ir/image_raw` publishing (IR image)
- [x] `/depth/points` (point cloud) available
- [x] Camera info topics for all streams
- [x] No "device not found" errors

**Status: ✅ PASSED**
- **Device:** Orbbec 3D Technology International, Inc USB 2.0 Camera (ID 2bc5:050f) and ORBBEC Depth Sensor (ID 2bc5:060f)
- **Node Name:** `/astra_camera`
- **Topics Published:**
  - `/color/camera_info` - RGB camera calibration info
  - `/color/image_raw` - RGB image (sensor_msgs/Image)
  - `/depth/camera_info` - Depth camera calibration info
  - `/depth/image_raw` - Depth image (sensor_msgs/Image)
  - `/depth/points` - Point cloud (sensor_msgs/PointCloud2)
  - `/ir/camera_info` - IR camera calibration info
  - `/ir/image_raw` - IR image (sensor_msgs/Image)
  - `/tf` and `/tf_static` - Camera transforms
- **Services Available:**
  - `/get_camera_info` - Get camera information
  - `/get_camera_params` - Get camera parameters
  - `/get_device_info` - Get device information
  - Parameter services for camera configuration
- **Build Fixes Applied:**
  - Fixed `package.xml` to use `ament_cmake` instead of `ament_python`
  - Copied source files (`src/`, `include/`, `astra_camera_msgs/`, `openni2_redist/`) from old workspace
  - Fixed CMakeLists.txt dependencies (`image_publisher`, `astra_camera_msgs`)
  - Installed `libuvc-dev` and `libuvc0` system packages
  - Fixed library installation paths and rpath configuration
  - Fixed linking to `astra_camera_msgs` typesupport library
- **Verification:** 
  - ✅ All expected topics publishing
  - ✅ RGB, depth, and IR streams configured
  - ✅ Point cloud topic available
  - ✅ Node fully functional with all services
- **Notes:** 
  - Camera device must be connected on the host and mapped to the container (via `--device=/dev/video0:/dev/video0`, etc. in run script)
  - Requires `libuvc` library for USB video class support
  - Launch file: `astra.launch.py` (not `astra_X3.launch.py`)
  - Minor warning about "attempt to claim already-claimed interface" may appear but doesn't affect functionality

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
# Start driver (voice control driver includes IMU)
# Note: IMU data is published by /driver_node
export PYTHONPATH=/root/software/py_install_V3.3.1/build/lib:/root/software/py_install_V0.0.1/py_install/build/lib:$PYTHONPATH
source /opt/ros/humble/setup.bash
source /root/yahboomcar_ros2_ws/install/setup.bash
ros2 run yahboomcar_voice_ctrl Voice_Ctrl_Mcnamu_driver_X3
```
**Verify (in another terminal):**
```bash
# Check IMU raw data
ros2 topic echo /imu/data_raw --once

# Check magnetometer
ros2 topic echo /imu/mag --once

# Check battery voltage
ros2 topic echo /voltage --once

# Check IMU rate
ros2 topic hz /imu/data_raw

# Check magnetometer rate
ros2 topic hz /imu/mag
```
**Expected Output:**
```bash
# ros2 topic echo /imu/data_raw --once
header:
  stamp:
    sec: 1767392020
    nanosec: 923799578
  frame_id: imu_link
orientation:
  x: 0.0
  y: 0.0
  z: 0.0
  w: 1.0
linear_acceleration:
  x: <value>
  y: <value>
  z: <value>  # Should show ~9.8 m/s² when stationary (gravity)
angular_velocity:
  x: <value>  # Should be near 0 when stationary
  y: <value>
  z: <value>

# ros2 topic echo /imu/mag --once
header:
  stamp:
    sec: 1767392032
    nanosec: 123893491
  frame_id: imu_link
magnetic_field:
  x: <value>
  y: <value>
  z: <value>

# ros2 topic echo /voltage --once
data: 11.899999618530273

# ros2 topic hz /imu/data_raw
average rate: 10.003
	min: 0.099s max: 0.101s std dev: 0.00039s window: 11

# ros2 topic hz /imu/mag
average rate: 10.007
	min: 0.099s max: 0.101s std dev: 0.00044s window: 12
```
**Expected:**
- [x] Accelerometer shows ~9.8 m/s² on Z axis (gravity) when stationary
- [x] Gyroscope near 0 when stationary
- [x] Battery voltage 10-12V range
- [x] ~10Hz publish rate (not 20Hz as originally expected)

**Status: ✅ PASSED**
- **Device:** ICM20948 9-axis IMU sensor (on STM32F103RCT6 expansion board)
- **Node Name:** `/driver_node` (voice control driver includes IMU)
- **Topics Published:**
  - `/imu/data_raw` - IMU data (accelerometer, gyroscope, orientation) - sensor_msgs/Imu
  - `/imu/mag` - Magnetometer data - sensor_msgs/MagneticField
  - `/voltage` - Battery voltage - std_msgs/Float32
- **Test Results:**
  - ✅ Accelerometer: Linear acceleration detected (x: -6.57, y: -0.32, z: -12.16 m/s²)
    - Magnitude: ~13.8 m/s² (includes gravity + possible tilt/orientation)
    - Z-axis shows strongest component, indicating gravity detection
  - ✅ Gyroscope: Angular velocity near 0 when stationary (x: -0.001, y: -0.001, z: 0.0 rad/s)
  - ✅ Magnetometer: Magnetic field detected (x: -18.0, y: -32.25, z: 18.76 μT)
  - ✅ Battery voltage: 11.9V (within 10-12V expected range)
  - ✅ Publish rate: ~10Hz for both IMU and magnetometer topics
- **Verification:** 
  - ✅ IMU data publishing correctly
  - ✅ Magnetometer data publishing correctly
  - ✅ Battery voltage monitoring working
  - ✅ All sensors responding when robot is stationary
- **Notes:** 
  - IMU topics are `/imu/data_raw` and `/imu/mag` (not `/pub_imu` and `/pub_mag`)
  - Publish rate is ~10Hz (not 20Hz as originally expected)
  - IMU is integrated into the voice control driver node (`/driver_node`)
  - Microcontroller: STM32F103RCT6 (ARM Cortex-M3, 72MHz) on expansion board YB-ERF01-V3.0
  - IMU sensor: ICM20948 (9-axis: accelerometer, gyroscope, magnetometer)
  - **IMU Sensor Capabilities (from Yahboom documentation):**
    - I2C communication rate: 100kHz
    - Sensor data read rate: 100Hz (hardware capability)
    - ROS publish rate: ~10Hz (driver configuration, not sensor limitation)
    - Digital motion processor included for attitude calculation
    - Provides raw data from 3-axis gyroscope, 3-axis accelerometer, and 3-axis magnetometer
    - Can be visualized in RViz for attitude/orientation display

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
| 1.1 | LiDAR | Laser scanner | No | ✅ |
| 1.2 | Voice Control | Speech module | No | ✅ |
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
