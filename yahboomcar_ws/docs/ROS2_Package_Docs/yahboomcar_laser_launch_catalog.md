# yahboomcar_laser/launch Launch File Catalog

---

## yahboomcar_laser/launch/base.launch
**Purpose:** Bring up the lidar, robot base, and joystick control.

**Starts:**
- Lidar node (via lidar.launch)
- Low-level driver node (via yahboomcar_bringup/yahboomcar.launch)
- Joystick/remote control (via yahboomcar_ctrl/yahboom_joy.launch)

**Use:** For basic robot and lidar bringup with remote control.

---

## yahboomcar_laser/launch/base_rplidar.launch
**Purpose:** Bring up the RPLidar, robot base, and joystick control.

**Starts:**
- Lidar node (via lidar.launch)
- Low-level driver node (via yahboomcar_bringup/yahboomcar.launch)
- Joystick/remote control (via yahboomcar_ctrl/yahboom_joy.launch)

**Use:** For basic robot and RPLidar bringup with remote control.

---

## yahboomcar_laser/launch/laser_Avoidance.launch
**Purpose:** Lidar-based obstacle avoidance demo.

**Starts:**
- Full base bringup (via base.launch)
- Lidar obstacle avoidance node (`laser_Avoidance.py`)

**Use:** For demonstrating obstacle avoidance using lidar.

---

## yahboomcar_laser/launch/laser_Tracker.launch
**Purpose:** Lidar-based following demo.

**Starts:**
- Full base bringup (via base.launch)
- Lidar following node (`laser_Tracker.py`)

**Use:** For demonstrating following behavior using lidar.

---

## yahboomcar_laser/launch/laser_Warning.launch
**Purpose:** Lidar-based warning/guard demo.

**Starts:**
- Full base bringup (via base.launch)
- Lidar warning/guard node (`laser_Warning.py`)

**Use:** For demonstrating guard/warning behavior using lidar.

---

## yahboomcar_laser/launch/lidar.launch
**Purpose:** Bring up the lidar node only.

**Starts:**
- Lidar node (via ydlidar_ros_driver/TG.launch)

**Use:** For standalone lidar bringup. 