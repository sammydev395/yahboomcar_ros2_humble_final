# ROS Nodes and Topics Documentation

## Node Package and Launch File Information

### Main Launch Files
- `yahboomcar_bringup/launch/bringup.launch` - Main launch file that starts most nodes
- `yahboomcar_ctrl/launch/yahboom_joy.launch` - Launch file for joystick control nodes

## 1. `/driver_node` (yahboomcar_bringup/Mcnamu_X3plus.py)
**Package**: `yahboomcar_bringup`
**Launch File**: `yahboomcar_bringup/launch/bringup.launch`
**Purpose**: Main driver node for the Yahboom robot

### Published Topics:

#### 1. `/ArmAngleUpdate` [yahboomcar_msgs/ArmJoint]
- **Message Type**: `yahboomcar_msgs/ArmJoint`
- **Message Definition**:
  ```yaml
  int32 id          # Joint ID
  int32 run_time    # Execution time
  float32 angle     # Target angle
  float64[] joints  # Array of joint angles
  ```
- **Subscribers**: `/yahboom_joy`

#### 2. `/imu/data_raw` [sensor_msgs/Imu]
- **Message Type**: `sensor_msgs/Imu`
- **Message Definition**:
  ```yaml
  Header header
  Quaternion orientation
  float64[9] orientation_covariance
  Vector3 angular_velocity
  float64[9] angular_velocity_covariance
  Vector3 linear_acceleration
  float64[9] linear_acceleration_covariance
  ```

#### 3. `/joint_states` [sensor_msgs/JointState]
- **Message Type**: `sensor_msgs/JointState`
- **Message Definition**:
  ```yaml
  Header header
  string[] name
  float64[] position
  float64[] velocity
  float64[] effort
  ```

#### 4. `/mag/mag_raw` [sensor_msgs/MagneticField]
- **Message Type**: `sensor_msgs/MagneticField`
- **Message Definition**:
  ```yaml
  Header header
  Vector3 magnetic_field
  float64[9] magnetic_field_covariance
  ```

#### 5. `/vel_raw` [geometry_msgs/Twist]
- **Message Type**: `geometry_msgs/Twist`
- **Message Definition**:
  ```yaml
  Vector3 linear
  Vector3 angular
  ```

## 2. `/ekf_localization` (robot_localization/ekf_localization_node)
**Package**: `robot_localization`
**Launch File**: `yahboomcar_bringup/launch/bringup.launch`
**Purpose**: Extended Kalman Filter for robot localization

### Published Topics:

#### 1. `/odom` [nav_msgs/Odometry]
- **Message Type**: `nav_msgs/Odometry`
- **Message Definition**:
  ```yaml
  Header header
  string child_frame_id
  PoseWithCovariance pose
  TwistWithCovariance twist
  ```

#### 2. `/tf` [tf2_msgs/TFMessage]
- **Message Type**: `tf2_msgs/TFMessage`
- **Message Definition**:
  ```yaml
  geometry_msgs/TransformStamped[] transforms
  ```

## 3. `/imu_filter_madgwick` (imu_filter_madgwick/imu_filter_node)
**Package**: `imu_filter_madgwick`
**Launch File**: `yahboomcar_bringup/launch/bringup.launch`
**Purpose**: IMU data filtering using Madgwick algorithm

### Published Topics:

#### 1. `/imu/data` [sensor_msgs/Imu]
- **Message Type**: `sensor_msgs/Imu`
- **Message Definition**: Same as `/imu/data_raw`

## 4. `/joy_node` (joy/joy_node)
**Package**: `joy`
**Launch File**: `yahboomcar_ctrl/launch/yahboom_joy.launch`
**Purpose**: Joystick input handling

### Published Topics:

#### 1. `/joy` [sensor_msgs/Joy]
- **Message Type**: `sensor_msgs/Joy`
- **Message Definition**:
  ```yaml
  Header header
  float32[] axes
  int32[] buttons
  ```

## 5. `/odometry_publisher` (yahboomcar_bringup/base_node)
**Package**: `yahboomcar_bringup`
**Launch File**: `yahboomcar_bringup/launch/bringup.launch`
**Purpose**: Raw odometry data publisher

### Published Topics:

#### 1. `/odom_raw` [nav_msgs/Odometry]
- **Message Type**: `nav_msgs/Odometry`
- **Message Definition**: Same as `/odom`

## 6. `/robot_state_publisher` (robot_state_publisher/robot_state_publisher)
**Package**: `robot_state_publisher`
**Launch File**: `yahboomcar_bringup/launch/bringup.launch`
**Purpose**: Robot state and transform publisher

### Published Topics:

#### 1. `/tf` [tf2_msgs/TFMessage]
- **Message Type**: `tf2_msgs/TFMessage`
- **Message Definition**: Same as above

#### 2. `/tf_static` [tf2_msgs/TFMessage]
- **Message Type**: `tf2_msgs/TFMessage`
- **Message Definition**: Same as above

## 7. `/yahboom_joy` (yahboomcar_ctrl/yahboom_joy.py)
**Package**: `yahboomcar_ctrl`
**Launch File**: `yahboomcar_ctrl/launch/yahboom_joy.launch`
**Purpose**: Yahboom-specific joystick control

### Published Topics:

#### 1. `/Buzzer` [std_msgs/Bool]
- **Message Type**: `std_msgs/Bool`
- **Message Definition**:
  ```yaml
  bool data
  ```

#### 2. `/JoyState` [std_msgs/Bool]
- **Message Type**: `std_msgs/Bool`
- **Message Definition**:
  ```yaml
  bool data
  ```

#### 3. `/RGBLight` [std_msgs/Int32]
- **Message Type**: `std_msgs/Int32`
- **Message Definition**:
  ```yaml
  int32 data
  ```

#### 4. `/TargetAngle` [yahboomcar_msgs/ArmJoint]
- **Message Type**: `yahboomcar_msgs/ArmJoint`
- **Message Definition**: Same as `/ArmAngleUpdate`

#### 5. `/cmd_vel` [geometry_msgs/Twist]
- **Message Type**: `geometry_msgs/Twist`
- **Message Definition**: Same as `/vel_raw`

## Common Message Types

### Header (std_msgs/Header)
```yaml
uint32 seq
time stamp
string frame_id
```

### Vector3 (geometry_msgs/Vector3)
```yaml
float64 x
float64 y
float64 z
```

### Quaternion (geometry_msgs/Quaternion)
```yaml
float64 x
float64 y
float64 z
float64 w
```

### PoseWithCovariance (geometry_msgs/PoseWithCovariance)
```yaml
Pose pose
float64[36] covariance
```

### TwistWithCovariance (geometry_msgs/TwistWithCovariance)
```yaml
Twist twist
float64[36] covariance
``` 