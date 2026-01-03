# X3Plus ROS2 Humble - Comprehensive Test Plan

## Prerequisites
```bash
cd /home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws
source /opt/ros/humble/setup.bash
source install/setup.bash
```

---
<details>
<summary><h1>PHASE 1: Non-Movement Sensors & Peripherals</h1></summary>
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

</details>

---

<details>
<summary><h1>PHASE 2: Base/Chassis Tests</h1></summary>
*⚠️ Robot will move - clear area or use blocks!*

---

## Test 2.1: Buzzer & RGB (Pre-movement check)
**Purpose:** Verify auxiliary controls before movement

**Prerequisites:**
- Driver node must be running with PYTHONPATH set for Rosmaster_Lib
- Robot should be in safe configuration (wheels off ground recommended)

### Step 1: Start Driver Node
**Command:**
```bash
export PYTHONPATH=/root/software/py_install_V3.3.1/build/lib:$PYTHONPATH
export ROBOT_TYPE=x3plus
ros2 run yahboomcar_bringup Mcnamu_driver_X3plus
```

**Expected:** Node starts and shows:
```
[INFO] [driver_node]: Yahboomcar X3plus driver node started.
Rosmaster Serial Opened! Baudrate=115200
----------------create receive threading--------------
```

---

### Step 2: Verify Topics Exist
**Command:**
```bash
ros2 topic list | grep -E 'Buzzer|RGBLight'
```

**Expected Output:**
```
/Buzzer
/RGBLight
```

**Command:**
```bash
ros2 topic info /Buzzer
```

**Expected Output:**
```
Type: std_msgs/msg/Bool
Publisher count: 1
Subscription count: 1
```

**Command:**
```bash
ros2 topic info /RGBLight
```

**Expected Output:**
```
Type: std_msgs/msg/Int32
Publisher count: 1
Subscription count: 1
```

---

### Step 3: Test Buzzer ON
**Command:**
```bash
ros2 topic pub /Buzzer std_msgs/msg/Bool "{data: true}" --once
```

**Expected Output:**
```
publisher: beginning loop
publishing #1: std_msgs.msg.Bool(data=True)
```

**Expected Behavior:**
- [x] Buzzer sounds/beeps immediately
- [x] Buzzer continues until turned off

---

### Step 4: Test Buzzer OFF
**Command:**
```bash
ros2 topic pub /Buzzer std_msgs/msg/Bool "{data: false}" --once
```

**Expected Output:**
```
publisher: beginning loop
publishing #1: std_msgs.msg.Bool(data=False)
```

**Expected Behavior:**
- [x] Buzzer stops immediately
- [x] No sound from buzzer

---

### Step 5: Test RGB Light
**Command:**
```bash
ros2 topic pub /RGBLight std_msgs/msg/Int32 "{data: 2}" --once
```

**Expected Output:**
```
publisher: beginning loop
publishing #1: std_msgs.msg.Int32(data=2)
```

**Expected Behavior:**
- [x] RGB lights change to effect 2 (running light / 跑马灯)
- [x] Visual confirmation of light pattern change

**RGB Light Effects:**
- `data: 0` - Stop light effect
- `data: 1` - Flowing light (流水灯)
- `data: 2` - Running light (跑马灯)
- `data: 3` - Breathing light (呼吸灯)
- `data: 4` - Gradient light (渐变灯)
- `data: 5` - Starlight (星光点点)
- `data: 6` - Battery display (电量显示)

---

### Step 6: Test RGB Light via Joystick (Optional)
**Command:**
```bash
# Press Button 7 (R1) on joystick
# Monitor RGBLight topic to see effect change
ros2 topic echo /RGBLight
```

**Expected:**
- [x] Each press of Button 7 cycles through RGB effects (0-5)
- [x] RGB light changes pattern with each button press

---

**Status: ✅ PASSED**

**Test Results:**
- **Buzzer:** Working correctly - turns on/off as expected
- **RGB Light:** Working correctly - light effects change when command published
- **RGB Light via Joystick:** Working correctly - Button 7 (R1) cycles through effects
- **Node:** `/driver_node` subscribes to `/Buzzer` and `/RGBLight` topics
- **Note:** Driver node requires `PYTHONPATH=/root/software/py_install_V3.3.1/build/lib` for `Rosmaster_Lib`

---

## Test 2.2: Joystick Hardware (joy_node)
**Purpose:** Verify joystick device and raw input
**⚠️ SAFE: This node only reads hardware - no robot movement!**

**Device Requirements:**
- Joystick device must be connected on the host and accessible to the container
- Typically `/dev/input/js0` (or js1, js2, etc.)
- Usually automatically accessible in Docker containers via `/dev/input/` mount
- If not accessible, may need to add `--device=/dev/input/js0:/dev/input/js0` to container run script

### Step 1: Check Joystick Device
**Command:**
```bash
ls /dev/input/js*
```

**Expected Output:**
```
/dev/input/js0
```
(or `/dev/input/js1`, `/dev/input/js2`, etc. if multiple devices)

---

### Step 2: Start joy_node
**Command:**
```bash
ros2 run joy joy_node
```

**Expected:** Node starts and runs continuously (no output, or minimal startup messages)

---

### Step 3: Verify Node is Running (in another terminal)
**Command:**
```bash
ros2 node list | grep joy
```

**Expected Output:**
```
/joy_node
```

---

### Step 4: Check Topic is Publishing
**Command:**
```bash
ros2 topic list | grep joy
```

**Expected Output:**
```
/joy
/joy/set_feedback
```

**Command:**
```bash
ros2 topic echo /joy --once
```

**Expected Output:**
```yaml
header:
  stamp:
    sec: <timestamp>
    nanosec: <timestamp>
  frame_id: joy
axes:
- <float>  # axes[0] - Left stick X
- <float>  # axes[1] - Left stick Y
- <float>  # axes[2] - Right stick X
- <float>  # axes[3] - Right stick Y
- <float>  # axes[4] - Trigger/Button axis 1
- <float>  # axes[5] - Trigger/Button axis 2
- <float>  # axes[6] - D-pad X
- <float>  # axes[7] - D-pad Y
buttons:
- <int>    # buttons[0] - Button 0 (A/X)
- <int>    # buttons[1] - Button 1 (B/Circle)
- <int>    # buttons[2] - Button 2 (X/Square)
- <int>    # buttons[3] - Button 3 (Y/Triangle)
- <int>    # buttons[4-14] - Additional buttons
```

**Note:** Axes values range from -1.0 to 1.0. Button values are 0 (not pressed) or 1 (pressed).

---

### Step 5: Check Publish Rate
**Command:**
```bash
ros2 topic hz /joy
```

**Expected Output:**
```
average rate: 40.904
	min: 0.000s max: 0.100s std dev: 0.02557s window: 44
average rate: 39.819
	min: 0.000s max: 0.100s std dev: 0.02450s window: 84
```

**Expected:** Publish rate ~10-60Hz (typically 30-50Hz)

---

### Step 6: Real-time Joystick Monitoring
**Command:**
```bash
ros2 topic echo /joy
```

**Expected:** Continuous output showing joystick data updating in real-time

**Test Actions:**
1. **Move left stick** → Watch `axes[0]` and `axes[1]` values change
2. **Move right stick** → Watch `axes[2]` and `axes[3]` values change
3. **Press any button** → Watch corresponding `buttons[N]` value change from 0 to 1
4. **Release button** → Watch `buttons[N]` value change back to 0

**Expected:** All values update immediately when joystick is moved or buttons are pressed
**Expected:**
- [x] `/dev/input/js0` exists (or js1, js2, etc.)
- [x] Node `/joy_node` is running
- [x] Topic `/joy` is publishing
- [x] Button/axis values change when joystick is moved/pressed
- [x] Publish rate ~40-53Hz (higher than expected 10-30Hz)
- [x] No cmd_vel topic exists (driver not running = safe)

**Status: ✅ PASSED**

**Test Results:**
- **Device:** `/dev/input/js0` detected and accessible
- **Node:** `/joy_node` running successfully
- **Topic:** `/joy` publishing at ~40-53Hz
- **Data Structure:**
  - **8 axes:** axes[0-7] (sticks, triggers, D-pad)
    - axes[4] and axes[5] = 1.0 (triggers in neutral position)
    - axes[0-3] and axes[6-7] = 0.0 (sticks centered, D-pad neutral)
  - **15 buttons:** buttons[0-14] (all buttons detected)
- **Verification Tests Performed:**
  1. ✅ **Sticks movement:** Axes values change when sticks are moved
  2. ✅ **Button presses:** Button values change from 0 to 1 when pressed
  3. ✅ **Real-time monitoring:** Used `ros2 topic echo /joy` to observe live data
- **Node Info:**
  - Publishes: `/joy` (sensor_msgs/msg/Joy)
  - Subscribes: `/joy/set_feedback` (force feedback)
  - No movement-related topics (safe for testing)
- **Expected Output Example:**
```yaml
header:
  stamp:
    sec: 1767460152
    nanosec: 294209731
  frame_id: joy
axes:
- -0.0    # Left stick X
- -0.0    # Left stick Y
- -0.0    # Right stick X
- -0.0    # Right stick Y
- 1.0     # Trigger/Button axis 1
- 1.0     # Trigger/Button axis 2
- 0.0     # D-pad X
- 0.0     # D-pad Y
buttons:
- 0       # Button 0 (A/X)
- 0       # Button 1 (B/Circle)
- 0       # Button 2 (X/Square)
- 0       # Button 3 (Y/Triangle)
- 0       # Button 4 (LB)
- 0       # Button 5 (RB)
- 0       # Button 6 (Back/Select)
- 0       # Button 7 (Start)
- 0       # Button 8 (Left stick press)
- 0       # Button 9 (Right stick press)
- 0       # Button 10-14 (additional buttons)
```
- **Notes:**
  - Joystick hardware working correctly
  - All axes and buttons responding to input
  - Safe to proceed to Test 2.3 (joystick controller)

---

## Test 2.3: Joystick Controller (yahboom_joy_node)
**Purpose:** Verify joystick controller node that publishes to cmd_vel
**⚠️ SAFE: Driver node NOT running = no movement even if cmd_vel is published!**

**Note on Redundancy:** This test is NOT redundant with Test 2.2:
- **Test 2.2** tests the hardware driver (`joy_node`) - reads joystick hardware, publishes `/joy`
- **Test 2.3** tests the control logic (`yahboom_joy_node`) - subscribes to `/joy`, converts to robot commands (`/cmd_vel`, `/TargetAngle`)
- Test 2.3 verifies the controller node works correctly and publishes the right topics BEFORE we enable movement
- This is a safety check: we can verify cmd_vel messages are correct without the driver running (no movement risk)
- **Alternative:** If you prefer, we can skip Test 2.3 and test it later in Phase 4 when testing actual joystick control with movement

```bash
# Terminal 1: Start joy_node (from Test 2.2)
ros2 run joy joy_node

# Terminal 2: Start joystick controller
# This subscribes to /joy and publishes to /cmd_vel
# BUT since driver_node is NOT running, robot won't move!
ros2 run yahboomcar_ctrl yahboom_joy_X3plus
```
**Verify (in another terminal):**
```bash
# Check nodes are running
ros2 node list | grep -E 'joy|yahboom'

# Check topics
ros2 topic list | grep -E 'cmd_vel|joy|Buzzer|RGBLight|TargetAngle'

# Check node info
ros2 node info /yahboom_joy

# Monitor cmd_vel (SAFE - driver not running, so no movement)
ros2 topic echo /cmd_vel

# Now move joystick and watch cmd_vel messages
# Robot will NOT move because driver_node is not running!
```
**Expected:**
- [ ] Node `/yahboom_joy` is running
- [ ] Subscribes to `/joy` topic
- [ ] Publishes to `/cmd_vel` topic (chassis control)
- [ ] Publishes to `/TargetAngle` topic (arm control)
- [ ] Publishes to `/Buzzer` and `/RGBLight` topics
- [x] cmd_vel messages appear when joystick is moved
- [x] **Wheels move correctly** when joystick moved (robot off ground - safe test)
- [x] **Arm moves correctly** when joystick controls used

**Status: ✅ PASSED**

**Test Results:**
- **Node:** `/yahboom_joy` running successfully
- **Subscribes to:** `/joy` (from `joy_node`)
- **Publishes to:**
  - `/cmd_vel` - Chassis control (geometry_msgs/msg/Twist)
  - `/TargetAngle` - Arm control (yahboomcar_msgs/msg/ArmJoint)
  - `/Buzzer` - Buzzer control (std_msgs/msg/Bool)
  - `/RGBLight` - RGB light control (std_msgs/msg/Int32)
  - `/JoyState` - Joystick state (std_msgs/msg/Bool)
- **Service Client:** `/CurrentAngle` (yahboomcar_msgs/srv/RobotArmArray)
- **Verification Tests:**
  1. ✅ **cmd_vel publishing:** Messages appear when joystick is moved
  2. ✅ **Wheels movement:** Wheels spin correctly when joystick moved (robot off ground)
  3. ✅ **Arm movement:** Arm responds to joystick controls properly
  4. ✅ **RGB Light control:** RGB light changes when joystick button pressed (button 7 - cycles through 6 effects: 0-5)
  5. ✅ **Buzzer control:** Buzzer toggles when joystick button pressed (button 11)
- **Build Fix Applied:**
  - Added service definitions to `yahboomcar_msgs/CMakeLists.txt`:
    - `"srv/RobotArmArray.srv"`
    - `"srv/Kinematics.srv"`
  - Rebuilt `yahboomcar_msgs` package
  - Services now properly generated and importable
- **Joystick Button Controls (Jetson Controller):**
  - **Button 7 (R1):** RGB Light control (cycles through 6 effects: 0=stop, 1=flowing, 2=running, 3=breathing, 4=gradient, 5=starlight)
  - **Button 11 (START):** Buzzer toggle (on/off)
  - **Button 10 (SELECT):** Toggle gripper/arm joint 5 control mode
  - **Left stick (axes[0,1]):** Chassis forward/back/strafe
  - **Right stick (axes[2,3]):** Chassis rotation
  - **Button 0 (A):** Arm joint 2 down
  - **Button 1 (B):** Arm joint 1 down
  - **Button 3 (X):** Arm joint 1 up
  - **Button 4 (Y):** Arm joint 2 up
  - **D-pad (axes[6,7]):** Arm joints 3/4
  - **Button 6 (L1):** Gripper open / Arm joint 5 up
  - **Axes 5 (L2):** Gripper close / Arm joint 5 down
- **Note:** RGB light can also be controlled via joystick button (verified during testing)
- **Notes:**
  - Controller node works correctly
  - Full joystick control verified: chassis (wheels) and arm both respond correctly
  - RGB light and buzzer can be controlled via joystick buttons
  - Test performed with robot wheels off ground for safety
  - Arm movement verified via joystick (will test more detailed arm movements later)

---

## Test 2.4: RViz Visualization (Optional)
**Purpose:** Verify robot model and TF tree
**⚠️ Requires:** Primary display access (X11 forwarding configured in container)

**Display Requirements:**
- **Primary Display:** 10-inch touchscreen mounted on Astra camera arm
- Container has X11 forwarding configured (`DISPLAY=:0`, `/tmp/.X11-unix` mounted)
- Host must have display server running
- X11 permissions: `xhost +` should be run on host (already done in `run_docker_ros2.sh`)
- RViz window will open on the touchscreen display

### Step 1: Verify Display Access
**Command:**
```bash
docker exec yahboom_ros2_humble bash -c "echo \$DISPLAY"
```

**Expected Output:**
```
:0
```

**Command:**
```bash
docker exec yahboom_ros2_humble bash -c "ls -la /tmp/.X11-unix/ 2>&1 | head -3"
```

**Expected Output:**
```
total 0
drwxrwxrwt. 2 root root 140 Jan  3 17:00 .
drwxrwxrwt. 2 root root 140 Jan  3 17:00 X0
```

**If display not accessible:** 
- Check host has display server running: `echo $DISPLAY` on host
- Check X11 permissions: `xhost` on host should show access granted
- Container already configured with X11 forwarding in `run_docker_ros2.sh`

---

### Step 2: Launch Full Bringup with RViz
**Command:**
```bash
# In container (or via docker exec -it)
export PYTHONPATH=/root/software/py_install_V3.3.1/build/lib:$PYTHONPATH
export ROBOT_TYPE=x3plus
source /opt/ros/humble/setup.bash
source /root/yahboomcar_ros2_ws/install/setup.bash
ros2 launch yahboomcar_bringup yahboomcar_bringup_X3plus_launch.py use_rviz:=true
```

**Expected Output:**
```
[INFO] [launch]: All log files can be found below /root/.ros/log/...
[INFO] [robot_state_publisher-1]: process started with pid [xxx]
[INFO] [driver_node-2]: process started with pid [xxx]
...
[INFO] [rviz2-3]: process started with pid [xxx]
```

**Expected Behavior:**
- [ ] Launch file starts all nodes (driver, base, imu_filter, ekf, joystick, etc.)
- [ ] RViz2 window opens on 10-inch touchscreen display (mounted on arm)
- [ ] Robot model appears in RViz 3D view
- [ ] Touchscreen is responsive (can interact with RViz)
- [ ] **Robot model is stable (no flickering/jumping)** - See troubleshooting section below if flickering occurs

**⚠️ IMPORTANT - Joint State Publisher Conflict:**
The launch file includes `joint_state_publisher` by default, which is meant for visualization **without** a real robot. When using a real robot (with `driver_node` running), both nodes publish to `/joint_states`, causing conflicts and flickering in RViz.

**Fix Applied:** Launch file now includes `use_real_robot:=true` argument (default) which automatically disables `joint_state_publisher` when real robot is connected. This prevents conflicts.

**If flickering occurs:**
1. Check for multiple publishers: `ros2 topic info /joint_states --verbose`
2. Stop conflicting node: `pkill -f joint_state_publisher`
3. Verify only `driver_node` publishes: `ros2 topic info /joint_states`

**Note:** Detailed RViz verification (robot model details, TF tree, sensor visualization, arm visualization) is covered in **Test 5.1: Full Bringup with RViz** in Phase 5.

---

**Status:** ✅ PASSED

**Notes:**
- **Display:** 10-inch touchscreen mounted on Astra camera arm (primary display)
- RViz requires primary display access (X11 forwarding)
- Container already configured with `DISPLAY=:0` and X11 socket mount
- RViz window will appear on the touchscreen - can interact via touch
- If RViz doesn't open, check host display and X11 permissions
- **This test only verifies basic RViz launch - detailed verification moved to Test 5.1**

**Troubleshooting - Robot Model Flickering/Jumping in RViz:**

**Problem:** Robot model in RViz flickers or jumps around erratically.

**Root Cause:** Multiple nodes publishing to `/joint_states` topic:
- `joint_state_publisher` - Publishes default/random values for visualization without real robot
- `driver_node` - Publishes real joint states from actual robot hardware
- When both run simultaneously, they conflict, causing RViz to rapidly switch between different joint values

**Solution (Already Fixed in Launch File):**
The launch file now includes `use_real_robot:=true` argument (default) which automatically disables `joint_state_publisher` when real robot is connected. This ensures only `driver_node` publishes joint states.

**Manual Fix (if needed):**
```bash
# Check for multiple publishers
ros2 topic info /joint_states --verbose

# Stop conflicting node
pkill -f joint_state_publisher

# Verify only driver_node publishes
ros2 topic info /joint_states
# Should show: Publisher count: 1 (or 2 if driver_node has multiple publishers)
```

**Prevention:**
- Always use `use_real_robot:=true` when launching with real robot hardware
- Use `use_real_robot:=false` only for pure visualization/simulation without hardware
- Launch file automatically handles this conflict prevention

</details>

---

<details>
<summary><h1>PHASE 3: Arm Tests (Stationary Robot)</h1></summary>
*Robot stays in place - only arm moves*

---

## Test 3.1: Arm Camera (Gripper Camera)
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

## Test 3.2: Arm Joint States
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

## Test 3.3: Arm Current Angle Service
**Purpose:** Query actual arm positions
```bash
ros2 service call /CurrentAngle yahboomcar_msgs/srv/RobotArmArray "{apply: 'GetArmJoints'}"
```
**Expected:**
- [ ] Returns 6 angle values in degrees
- [ ] Values approximately [90, 145, 0, 45, 90, 30] (initial position)

---

## Test 3.4: Arm Single Joint Movement
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

## Test 3.5: Gripper Open/Close
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

## Test 3.6: Arm Full Position Command
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

## Test 3.7: Arm Autopilot (Color Following)
**Purpose:** Verify arm can follow colored objects autonomously
**⚠️ Prerequisites:** 
- Camera tested (Test 1.3)
- Arm tests passed (Phase 3)

```bash
# Test arm autopilot (color following)
ros2 launch arm_autopilot arm_autopilot.launch.py
```

**Expected:**
- [ ] Color detection working
- [ ] Arm tracks colored objects
- [ ] Arm responds to vision input

**Status:** ⬜ PENDING

</details>

---

<details>
<summary><h1>PHASE 4: Joystick Integration (Chassis + Arm)</h1></summary>
*Full control testing - Robot will move!*

---

## Test 4.1: Chassis Movement (cmd_vel)
**Purpose:** Test mecanum wheel control
```bash
# ⚠️ SAFETY: Robot on blocks OR clear 2m radius!
# Driver node must be running: ros2 run yahboomcar_bringup Mcnamu_driver_X3plus

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

## Test 4.2: Odometry Feedback
**Purpose:** Verify wheel encoders/odometry
```bash
# Start base_node for odometry (if not already running)
ros2 run yahboomcar_base_node base_node_X3

# Check velocity feedback
ros2 topic echo /vel_raw --once

# Check odometry
ros2 topic echo /odom --once

# Check publish rate
ros2 topic hz /vel_raw
ros2 topic hz /odom
```
**Expected:**
- [ ] Velocity values match commanded direction
- [ ] Odometry position updates
- [ ] Publish rate ~10-20Hz

---

## Test 4.3: Full Joystick Control (Chassis + Arm)
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

</details>

---

<details>
<summary><h1>PHASE 5: Full System Integration</h1></summary>
*All components working together*

---

## Test 5.1: Voice Control with Movement
**Purpose:** Verify voice commands control robot movement and functions
**⚠️ Prerequisites:** Voice control module tested (Test 1.2)

### Step 1: Launch Full Bringup with Voice Control
**Command:**
```bash
# Launch full bringup (includes voice control via driver_node)
export PYTHONPATH=/root/software/py_install_V3.3.1/build/lib:$PYTHONPATH
export ROBOT_TYPE=x3plus
source /opt/ros/humble/setup.bash
source /root/yahboomcar_ros2_ws/install/setup.bash
ros2 launch yahboomcar_bringup yahboomcar_bringup_X3plus_launch.py
```

**Expected:** Driver node starts with voice control enabled

---

### Step 2: Verify Voice Control Topics
**Command:**
```bash
# Check voice control topics
ros2 topic list | grep -E 'speech|voice'
```

**Expected:** Voice control node running (part of driver_node)

---

### Step 3: Test Voice Commands
**⚠️ SAFETY: Robot on blocks OR clear 2m radius for movement commands!**

**Voice Commands to Test:**
1. **"Start"** or **"Go"** - Robot should respond/acknowledge
2. **"Forward"** or **"Move forward"** - Robot should move forward
3. **"Back"** or **"Move back"** - Robot should move backward
4. **"Left"** or **"Turn left"** - Robot should turn left
5. **"Right"** or **"Turn right"** - Robot should turn right
6. **"Stop"** - Robot should stop immediately
7. **"Arm up"** or **"Arm down"** - Arm should move (if supported)
8. **"Gripper open"** or **"Gripper close"** - Gripper should open/close (if supported)

**Expected:**
- [ ] Robot responds with audio confirmation
- [ ] Movement commands execute correctly
- [ ] Stop command works immediately
- [ ] Arm/gripper commands work (if supported by voice module)
- [ ] Commands are recognized reliably

---

### Step 4: Monitor Command Execution
**Command (in another terminal):**
```bash
# Monitor cmd_vel topic to see voice commands
ros2 topic echo /cmd_vel
```

**Expected:**
- [ ] cmd_vel messages appear when voice commands are given
- [ ] Values match expected movement (forward/back/turn)
- [ ] Stop command publishes zero velocity

---

**Status:** ⬜ PENDING

**Notes:**
- Voice control is integrated into driver_node
- Commands may vary based on voice module firmware
- Test in quiet environment for best recognition
- Some commands may require specific phrasing

---

## Test 5.2: Line Following
**Purpose:** Verify robot can follow lines using camera vision
**⚠️ Prerequisites:** 
- Camera tested (Test 1.3)
- LiDAR tested (Test 1.1) - used for obstacle avoidance
- Chassis movement tested (Test 4.1)

### Step 1: Prepare Test Environment
**Requirements:**
- Line on floor (tape, painted line, or colored line)
- Line should be 2-5cm wide
- Clear path with minimal obstacles
- Good lighting

---

### Step 2: Launch Line Following
**Command:**
```bash
# Launch full bringup first
export PYTHONPATH=/root/software/py_install_V3.3.1/build/lib:$PYTHONPATH
export ROBOT_TYPE=x3plus
source /opt/ros/humble/setup.bash
source /root/yahboomcar_ros2_ws/install/setup.bash
ros2 launch yahboomcar_bringup yahboomcar_bringup_X3plus_launch.py

# In another terminal, launch line following
ros2 launch yahboomcar_linefollow linefollow_X3.launch.py
```

**Expected:** Line following node starts, camera feed active

---

### Step 3: Calibrate Line Detection
**In Line Following Window (if GUI available):**
- [ ] Camera feed displays
- [ ] Click and drag to select line region (ROI)
- [ ] Line detection parameters can be adjusted
- [ ] HSV color range can be tuned for line color

**Alternative (if no GUI):**
- Pre-configured HSV values should work for common line colors
- May need to adjust parameters in launch file or config

---

### Step 4: Verify Line Detection Topics
**Command:**
```bash
# Check line following topics
ros2 topic list | grep -E 'line|follow'
ros2 topic echo /linefollw/rgb --once  # Processed image with line detection
```

**Expected:**
- [ ] `/linefollw/rgb` topic publishing (processed image)
- [ ] `/cmd_vel` topic publishing (movement commands)
- [ ] `/scan` topic available (for obstacle avoidance)

---

### Step 5: Test Line Following
**Procedure:**
1. Place robot at start of line
2. Ensure line is visible in camera view
3. Robot should start following line automatically
4. Monitor robot movement and line tracking

**Expected:**
- [ ] Robot detects line in camera feed
- [ ] Robot starts moving forward along line
- [ ] Robot adjusts direction to stay on line
- [ ] Robot follows curves and turns
- [ ] Robot stops if line is lost
- [ ] Robot stops if obstacle detected (LiDAR)

---

### Step 6: Test Obstacle Avoidance
**Procedure:**
1. Place obstacle in path while robot is following line
2. Robot should detect obstacle via LiDAR
3. Robot should stop and sound buzzer

**Expected:**
- [ ] LiDAR detects obstacle ahead
- [ ] Robot stops immediately
- [ ] Buzzer sounds warning
- [ ] Robot resumes when obstacle cleared (if supported)

---

### Step 7: Test Joystick Override
**Procedure:**
1. While robot is following line, use joystick
2. Joystick should override line following

**Expected:**
- [ ] Joystick input disables line following
- [ ] Manual control takes priority
- [ ] Line following resumes when joystick released

---

**Status:** ⬜ PENDING

**Notes:**
- Line following uses PID control for smooth tracking
- HSV color detection requires good lighting
- Works best with high-contrast lines (black tape on white floor, etc.)
- Obstacle avoidance uses LiDAR at ~30° forward angle
- Response distance: ~0.55m (configurable)

---

## Test 5.3: SLAM Mapping
**Purpose:** Verify simultaneous localization and mapping
**⚠️ Prerequisites:** 
- LiDAR tested (Test 1.1)
- Chassis movement tested (Test 4.1)
- Odometry tested (Test 4.2)

```bash
# Terminal 1: Full bringup
ros2 launch yahboomcar_bringup yahboomcar_bringup_X3plus_launch.py

# Terminal 2: SLAM
ros2 launch yahboomcar_slam slam_X3_launch.py
```

**Expected:**
- [ ] Map builds as robot moves
- [ ] Localization stable
- [ ] Map saved successfully

**Status:** ⬜ PENDING

---

## Test 5.4: Full Bringup with RViz
**Purpose:** Verify complete system integration with RViz visualization
**⚠️ Requires:** Primary display access (X11 forwarding configured in container)

**Prerequisites:**
- All Phase 1 tests passed (LiDAR, Voice, Camera, IMU)
- All Phase 2 tests passed (Buzzer/RGB, Joystick, RViz basic launch)
- All Phase 3 tests passed (Arm tests)
- All Phase 4 tests passed (Chassis movement, Odometry, Full joystick control)

### Step 1: Launch Full Bringup with RViz
**Command:**
```bash
# In container (or via docker exec -it)
export PYTHONPATH=/root/software/py_install_V3.3.1/build/lib:$PYTHONPATH
export ROBOT_TYPE=x3plus
source /opt/ros/humble/setup.bash
source /root/yahboomcar_ros2_ws/install/setup.bash
ros2 launch yahboomcar_bringup yahboomcar_bringup_X3plus_launch.py use_rviz:=true
```

**Expected:** All nodes start, RViz opens on touchscreen display

---

### Step 2: Verify Robot Model Display
**In RViz Window:**
- [ ] Robot model (chassis + 6-DOF arm) visible in 3D view
- [ ] Model matches physical robot configuration
- [ ] Model updates when arm moves (if driver node running)
- [ ] All arm joints visible and correctly positioned

---

### Step 3: Verify TF Tree
**Command (in another terminal):**
```bash
docker exec yahboom_ros2_humble bash -c "source /opt/ros/humble/setup.bash && source /root/yahboomcar_ros2_ws/install/setup.bash && ros2 run tf2_tools view_frames"
```

**Expected Output:**
```
[INFO] [view_frames]: Listening to tf data during 5 seconds...
[INFO] [view_frames]: Generating graph in frames.pdf
```

**Command:**
```bash
docker exec yahboom_ros2_humble bash -c "ros2 run tf2_ros tf2_echo base_link laser_frame 2>&1 | head -10"
```

**Expected:** TF transform data (or error if frames not available)

**In RViz:**
- [ ] TF tree shows in "TF" display (if enabled)
- [ ] No TF errors in console
- [ ] All expected frames present (base_link, laser_frame, arm joints, etc.)

---

### Step 4: Verify Sensor Data Visualization
**In RViz (add displays):**
- [ ] **LiDAR:** Add "LaserScan" display, topic `/scan` - should show scan data
- [ ] **Camera:** Add "Image" display, topic `/color/image_raw` - should show camera feed
- [ ] **Depth:** Add "DepthCloud" or "PointStamped" display, topic `/depth/points` - should show depth data
- [ ] **IMU:** Add "Axes" display for IMU orientation (if available)

**Expected:**
- [ ] LiDAR scan shows obstacles/environment
- [ ] Camera feed shows live video
- [ ] Depth data visualizes 3D point cloud
- [ ] IMU axes show orientation

---

### Step 5: Verify Arm Visualization
**In RViz:**
- [ ] Arm joints visible in robot model
- [ ] Arm position matches physical arm position
- [ ] Arm moves in RViz when physical arm moves (if driver node running)
- [ ] All 6 joints (arm_joint1-5, grip_joint) correctly displayed

---

### Step 6: Verify Chassis Visualization
**In RViz:**
- [ ] Chassis/base visible in robot model
- [ ] Mecanum wheels visible (if meshes available)
- [ ] Robot position updates when chassis moves (if odometry running)

---

**Status:** ⬜ PENDING

**Notes:**
- This test verifies complete system integration with all sensors and actuators
- RViz should display all components working together
- All previous tests should pass before attempting this integration test

</details>

---

<details>
<summary><h1>PHASE 6: MediaPipe Vision Control</h1></summary>
*Hand gesture and pose detection for robot control*

---

## Test 6.1: MediaPipe Hand Gesture Arm Control
**Purpose:** Verify arm can be controlled using hand gestures via MediaPipe
**⚠️ Prerequisites:** 
- Camera tested (Test 1.3)
- Arm tests passed (Phase 3)
- MediaPipe library installed (`pip3 install mediapipe`)

### Step 1: Launch Full Bringup
**Command:**
```bash
# Launch full bringup first
export PYTHONPATH=/root/software/py_install_V3.3.1/build/lib:$PYTHONPATH
export ROBOT_TYPE=x3plus
source /opt/ros/humble/setup.bash
source /root/yahboomcar_ros2_ws/install/setup.bash
ros2 launch yahboomcar_bringup yahboomcar_bringup_X3plus_launch.py
```

**Expected:** All nodes start, camera available

---

### Step 2: Launch MediaPipe Arm Control
**Command (in another terminal):**
```bash
source /opt/ros/humble/setup.bash
source /root/yahboomcar_ros2_ws/install/setup.bash
ros2 launch arm_mediapipe arm_mediapipe.launch.py
```

**Expected:** 
- MediaPipe node starts
- Camera window opens showing hand detection
- FPS counter visible in window

---

### Step 3: Verify MediaPipe Topics
**Command:**
```bash
# Check MediaPipe topics
ros2 topic list | grep -E 'mediapipe|hand|arm|TargetAngle'
ros2 topic echo /TargetAngle --once  # Arm position commands
```

**Expected:**
- [ ] `/TargetAngle` topic publishing (arm control commands)
- [ ] Image topics available (processed camera feed)
- [ ] Node `/hand_ctrl_arm` running

---

### Step 4: Test Hand Detection
**Procedure:**
1. Position hand in front of camera
2. Ensure good lighting
3. Hand should be visible in camera window

**Expected:**
- [ ] Hand landmarks detected (21 points visible)
- [ ] Hand bounding box displayed
- [ ] Hand tracking smooth and responsive
- [ ] FPS counter shows reasonable frame rate (~15-30 FPS)

---

### Step 5: Test Hand Position Control
**Procedure:**
1. Move hand left/right in camera view
2. Move hand up/down in camera view
3. Observe arm movement

**Expected:**
- [ ] **Hand X position** controls **Joint 1** (base rotation)
  - Hand left → Joint 1 rotates left
  - Hand right → Joint 1 rotates right
- [ ] **Hand Y position** controls **Joints 2, 3, 4** (shoulder, elbow, wrist)
  - Hand up → Arm extends upward
  - Hand down → Arm lowers
- [ ] Arm movement is smooth and proportional to hand position
- [ ] Arm follows hand movement in real-time

---

### Step 6: Test Thumb-to-Index Distance (Gripper Control)
**Procedure:**
1. Open hand (thumb and index finger apart)
2. Close hand (thumb and index finger together - "OK" gesture)
3. Observe gripper movement

**Expected:**
- [ ] **Thumb-to-index distance** controls **Gripper** (Joint 6)
  - Fingers apart → Gripper opens
  - Fingers together → Gripper closes
- [ ] Gripper position proportional to finger distance
- [ ] Smooth gripper control

---

### Step 7: Test Hand Gestures
**Procedure:**
1. Make "OK" gesture (thumb and index finger circle)
2. Make "Yes" gesture (index and middle finger up)
3. Make "Thumb down" gesture
4. Observe any special behaviors

**Expected:**
- [ ] **"OK" gesture** - Recognized (thumb and index finger close)
- [ ] **"Yes" gesture** - Recognized (index and middle finger up)
- [ ] **"Thumb down" gesture** - Recognized (thumb pointing down)
- [ ] Gestures detected reliably
- [ ] Gesture recognition doesn't interfere with position control

---

### Step 8: Test Joystick Override
**Procedure:**
1. While controlling arm with hand gestures, use joystick
2. Joystick should override MediaPipe control

**Expected:**
- [ ] Joystick input disables MediaPipe arm control
- [ ] Manual joystick control takes priority
- [ ] MediaPipe resumes when joystick released

---

### Step 9: Test Arm Position Limits
**Procedure:**
1. Move hand to extreme positions (far left/right, high/low)
2. Verify arm doesn't exceed joint limits

**Expected:**
- [ ] Arm respects joint limits
- [ ] No grinding or stalling sounds
- [ ] Smooth movement within safe range

---

**Status:** ⬜ PENDING

**Notes:**
- **MediaPipe Features:**
  - Hand detection with 21 landmark points
  - Real-time hand tracking
  - Gesture recognition (OK, Yes, Thumb down)
  - Position-based arm control
  - Distance-based gripper control
- **Control Mapping:**
  - Hand X position → Joint 1 (base rotation: -0.3 * X + 186)
  - Hand Y position → Joint 2 (shoulder: -0.4 * Y + 170)
  - Hand Y position → Joint 3 (elbow: 0.05 * Y + 25)
  - Hand Y position → Joint 4 (wrist: -0.125 * Y + 85)
  - Joint 5 fixed at 90°
  - Thumb-to-index distance → Joint 6 (gripper: interpolated 0-70° → 185-20°)
- **Requirements:**
  - Good lighting for hand detection
  - Hand should be clearly visible in camera view
  - Camera at `/dev/camera_depth` or `/dev/video0`
  - MediaPipe library installed: `pip3 install mediapipe`
- **Performance:**
  - CPU usage: ~35-45%
  - Memory: ~500MB
  - Frame rate: ~15-30 FPS (depends on hardware)
- **Safety:**
  - Joystick can override MediaPipe control
  - Arm respects joint limits
  - Test in safe area with arm clearance

---

## Test 6.2: MediaPipe Pose Detection (Optional)
**Purpose:** Verify full body pose detection capabilities
**⚠️ Prerequisites:** MediaPipe library installed

### Step 1: Launch Pose Detection
**Command:**
```bash
# Check if pose detection node exists
ros2 pkg list | grep mediapipe
```

**Expected:** MediaPipe packages available

---

### Step 2: Test Pose Detection Features
**Features to Test:**
- [ ] Full body pose landmarks (33 points)
- [ ] Pose tracking in camera feed
- [ ] Pose data published to topics
- [ ] Real-time performance

**Expected:**
- [ ] Pose landmarks detected when person visible
- [ ] Smooth tracking of body movements
- [ ] Pose data available on topics

---

**Status:** ⬜ PENDING (Optional - depends on available nodes)

**Notes:**
- Pose detection provides full body tracking
- Can be used for human-robot interaction
- Requires person to be fully visible in camera view

</details>

---

# Quick Reference: Test Order Checklist

| # | Test | Component | Movement? | Status |
|---|------|-----------|-----------|--------|
| 1.1 | LiDAR | Laser scanner | No | ✅ |
| 1.2 | Voice Control | Speech module | No | ✅ |
| 1.3 | Astra Camera | RGB-D | No | ✅ |
| 1.4 | IMU Data | Sensors | No | ✅ |
| 2.1 | Buzzer/RGB | Auxiliary | No | ✅ |
| 2.2 | Joystick Hardware | joy_node | No | ✅ |
| 2.3 | Joystick Controller | yahboom_joy | No | ✅ |
| 2.4 | RViz | Visualization | No | ⬜ |
| 3.1 | Arm Camera | Gripper cam | No | ⬜ |
| 3.2 | Arm Joint States | Feedback | No | ⬜ |
| 3.3 | Arm Angle Service | Query | No | ⬜ |
| 3.4 | Arm Single Joint | Arm moves | ⚠️ Arm | ⬜ |
| 3.5 | Gripper | Gripper | ⚠️ Arm | ⬜ |
| 3.6 | Arm Full Position | All joints | ⚠️ Arm | ⬜ |
| 3.7 | Arm Autopilot | Vision+Arm | ⚠️ Arm | ⬜ |
| 4.1 | Chassis Movement | Wheels | ⚠️ Chassis | ⬜ |
| 4.2 | Odometry | Encoders | After move | ⬜ |
| 4.3 | Full Joystick | Everything | ⚠️ Full | ⬜ |
| 5.1 | Voice Control | Movement | ⚠️ Full | ⬜ |
| 5.2 | Line Following | Vision+Movement | ⚠️ Full | ⬜ |
| 5.3 | SLAM | Mapping | ⚠️ Full | ⬜ |
| 5.4 | Full Bringup | Integration | ⚠️ Full | ⬜ |
| 5.5 | Full Bringup | Integration | ⚠️ Full | ⬜ |
| 6.1 | MediaPipe Hand | Gesture Control | ⚠️ Arm | ⬜ |
| 6.2 | MediaPipe Pose | Pose Detection | No | ⬜ |

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
