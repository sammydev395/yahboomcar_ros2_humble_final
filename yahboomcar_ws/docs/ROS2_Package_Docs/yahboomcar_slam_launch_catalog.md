# yahboomcar_slam/launch Launch File Catalog

---

## yahboomcar_slam/launch/camera_driver.launch
**Purpose:** Bring up joystick control and robot model for SLAM experiments.

**Starts:**
- Joystick/remote control (via yahboomcar_ctrl/yahboom_joy.launch)
- Robot model and state publisher (via yahboomcar_bringup/yahboomcar_model.launch)

**Use:** For SLAM experiments requiring manual control and robot model.

---

## yahboomcar_slam/launch/camera_orb_slam.launch
**Purpose:** ORB-SLAM2 with camera input (mono, monoAR, or RGBD).

**Starts:**
- Astra Pro camera node (if RGBD)
- USB camera node (if mono/monoAR)
- ORB-SLAM2 node (Mono, MonoAR, or RGBD mode)

**Use:** For visual SLAM with different camera configurations.

---

## yahboomcar_slam/launch/robot_orb_octomap.launch
**Purpose:** ORB-SLAM2 with point cloud mapping and Octomap integration.

**Starts:**
- Static transform publisher (odom to world)
- Point cloud mapping node
- Octomap server
- RViz (optional)

**Use:** For 3D mapping and SLAM with Octomap.

---

## yahboomcar_slam/launch/robot_orb_slam.launch
**Purpose:** ORB-SLAM2 with RGBD input and robot base bringup.

**Starts:**
- Astra Pro camera node
- Full robot base bringup (via yahboomcar_bringup/bringup.launch)
- Static transform publisher (camera)
- ORB-SLAM2 RGBD pose node

**Use:** For RGBD SLAM with robot base.

---

## yahboomcar_slam/launch/test_orb_slam.launch
**Purpose:** Test ORB-SLAM2 with RGBD input.

**Starts:**
- Astra Pro camera node
- ORB-SLAM2 RGBD pose node

**Use:** For testing RGBD SLAM.

---

## yahboomcar_slam/launch/test_pcl_mapping.launch
**Purpose:** Test point cloud mapping with ORB-SLAM2 outputs.

**Starts:**
- Point cloud mapping node

**Use:** For testing 3D mapping from SLAM outputs.

---

## yahboomcar_slam/launch/view_orb_octomap.launch
**Purpose:** RViz visualization for ORB-SLAM2 and Octomap.

**Starts:**
- RViz with ORB-SLAM2/Octomap config

**Use:** For visualizing SLAM and Octomap results. 