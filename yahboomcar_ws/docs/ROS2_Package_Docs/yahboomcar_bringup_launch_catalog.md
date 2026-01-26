# yahboomcar_bringup/launch Launch File Catalog

---

## yahboomcar_bringup/launch/bringup.launch
**Purpose:** Main bringup for the robot base and localization.

**Starts:**
- Robot model (URDF) and state publisher
- Low-level driver node (Mcnamu_X3plus.py for X3plus)
- Odometry publisher
- IMU filter (imu_filter_madgwick)
- Extended Kalman Filter (robot_localization/ekf_localization_node)
- Joystick/remote control (includes yahboom_joy.launch)
- Optionally RViz

**Does NOT start:** Cameras, lidar, arm, or voice nodes.

---

## yahboomcar_bringup/launch/bringup_calib.launch
**Purpose:** Bringup with IMU calibration and visualization.

**Starts:**
- Everything in bringup.launch
- IMU calibration node (imu_calib/apply_calib)
- IMU filter with calibration
- Optionally RViz

**Use:** When you want to calibrate and use the IMU with corrected data.

---

## yahboomcar_bringup/launch/calibrate_angular.launch
**Purpose:** Calibrate angular velocity of the robot.

**Starts:**
- calibrate_angular.py node for angular velocity calibration.

---

## yahboomcar_bringup/launch/calibrate_imu.launch
**Purpose:** Calibrate IMU installation errors.

**Starts:**
- Low-level driver node (via yahboomcar.launch)
- IMU calibration node (imu_calib/do_calib)

**Use:** When you want to generate a new IMU calibration file.

---

## yahboomcar_bringup/launch/calibrate_linear.launch
**Purpose:** Calibrate linear velocity of the robot.

**Starts:**
- calibrate_linear.py node for linear velocity calibration.

---

## yahboomcar_bringup/launch/patrol.launch
**Purpose:** Autonomous patrol demo.

**Starts:**
- Full bringup (via bringup.launch)
- Lidar (via ydlidar_ros_driver/TG.launch)
- Patrol node (patrol.py)

**Use:** For patrol/autonomous navigation demos.

---

## yahboomcar_bringup/launch/test_imu.launch
**Purpose:** Test IMU and odometry.

**Starts:**
- Low-level driver node
- Odometry publisher
- IMU filter
- Static TF for IMU
- Optionally RViz

**Use:** For IMU/odometry testing and visualization.

---

## yahboomcar_bringup/launch/view_odom.launch
**Purpose:** Visualize odometry in RViz.

**Starts:**
- RViz with odometry config.

---

## yahboomcar_bringup/launch/yahboomcar.launch
**Purpose:** Minimal bringup for the robot base.

**Starts:**
- Only the low-level driver node (Mcnamu_driver.py or Mcnamu_X3plus.py).

**Use:** For low-level testing or as a dependency in other launch files.

---

## yahboomcar_bringup/launch/yahboomcar_model.launch
**Purpose:** Bring up the robot model and static transforms.

**Starts:**
- Low-level driver node
- Robot model (URDF) and state publisher
- Static TF for camera

**Use:** For simulation or when you need the robot model and transforms. 