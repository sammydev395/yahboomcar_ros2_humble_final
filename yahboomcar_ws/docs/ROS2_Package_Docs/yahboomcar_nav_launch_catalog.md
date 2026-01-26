# yahboomcar_nav/launch Launch File Catalog

---

## yahboomcar_nav/launch/astrapro_bringup.launch
**Purpose:** Bring up the Astra Pro camera, robot base, and convert depth images to laser scans for navigation.

**Starts:**
- Astra Pro camera node (via orbbec_camera/astraproplus.launch)
- Full robot base bringup (via yahboomcar_bringup/bringup.launch)
- Depth image to laser scan conversion (via yahboomcar_nav/library/depthimage_to_laserscan.launch)
- Static transform between camera and laser frames

**Use:** For navigation setups using the Astra Pro camera as a depth sensor and generating laser scans for SLAM or navigation.

---

## yahboomcar_nav/launch/laser_astrapro_bringup.launch
**Purpose:** Bring up lidar, Astra Pro camera, and robot base for navigation.

**Starts:**
- Lidar node (via ydlidar_ros_driver/TG.launch)
- Astra Pro camera node (via orbbec_camera/astraproplus.launch)
- Full robot base bringup (via yahboomcar_bringup/bringup.launch)
- Static transform for lidar

**Use:** For navigation setups using both lidar and Astra Pro camera.

---

## yahboomcar_nav/launch/laser_bringup.launch
**Purpose:** Bring up lidar and robot base for navigation.

**Starts:**
- Lidar node (via ydlidar_ros_driver/TG.launch)
- Full robot base bringup (via yahboomcar_bringup/bringup.launch)
- Static transform for lidar

**Use:** For navigation setups using only lidar.

---

## yahboomcar_nav/launch/laser_usb_bringup.launch
**Purpose:** Bring up lidar, USB camera, and robot base for navigation.

**Starts:**
- Lidar node (via ydlidar_ros_driver/TG.launch)
- USB camera node (via usb_cam-test.launch)
- Full robot base bringup (via yahboomcar_bringup/bringup.launch)
- Static transforms for camera and lidar

**Use:** For navigation setups using both lidar and a USB camera.

---

## yahboomcar_nav/launch/rrt_exploration.launch
**Purpose:** RRT-based exploration and mapping demo.

**Starts:**
- Gmapping SLAM (with scan dilution for certain lidars)
- Move base for navigation
- Simple exploration node
- Optionally RViz for visualization

**Use:** For autonomous exploration and mapping using RRT and SLAM.

---

## yahboomcar_nav/launch/yahboomcar_map.launch
**Purpose:** Flexible mapping/SLAM launch file supporting multiple algorithms.

**Starts:**
- Gmapping, Hector, Karto, or Cartographer SLAM (selectable)
- Scan dilution for certain lidars
- Optionally RViz for visualization

**Use:** For creating maps with different SLAM algorithms.

---

## yahboomcar_nav/launch/yahboomcar_navigation.launch
**Purpose:** Main navigation launch file using a pre-built map.

**Starts:**
- Marker array node
- Map server
- AMCL localization
- Move base for navigation
- Optionally RViz for visualization

**Use:** For autonomous navigation using a pre-built map.

---

## yahboomcar_nav/launch/yahboomcar_rtabmap.launch
**Purpose:** RTAB-Map SLAM launch file.

**Starts:**
- RTAB-Map SLAM node (via yahboomcar_nav/library/rtabmap.launch)
- Optionally RViz for visualization

**Use:** For 3D SLAM and mapping with RTAB-Map.

---

## yahboomcar_nav/launch/yahboomcar_rtabmap_nav.launch
**Purpose:** RTAB-Map navigation launch file.

**Starts:**
- Marker array node
- Move base for navigation
- RTAB-Map navigation node (via yahboomcar_nav/library/rtabmap_nav.launch)
- Optionally RViz for visualization

**Use:** For autonomous navigation using RTAB-Map. 