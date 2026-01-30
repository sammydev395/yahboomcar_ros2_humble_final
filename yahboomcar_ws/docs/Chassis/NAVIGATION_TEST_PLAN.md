# Navigation (Chassis) Test Plan
## Yahboom ROS2 Workspace — yahboomcar_nav

**Date:** January 2025  
**Platform:** Yahboom ROSMASTER X3 Plus (or R2)  
**ROS Distribution:** ROS2 Humble  
**Package:** `yahboomcar_nav`

---

## Overview

This test plan covers **navigation stack testing** for the Yahboom car: mapping (SLAM), localization, and path planning. The stack supports **LIDAR-only** and **camera + LIDAR** configurations.

### Sensor Modes

| Mode | Sensors | Use cases |
|------|---------|-----------|
| **LIDAR-only** | RPLidar (A1 / S2 / 4ROS) → `/scan` | Gmapping, Cartographer, DWA/TEB Nav2 (scan-only costmap), laser bringup |
| **Camera + LIDAR** | RGB-D camera + `/scan` | RTAB-Map SLAM (RGB-D + scan), TEB/Carto+TEB costmap (depth point cloud + scan) |

### Key Features

- **Laser bringup:** RPLidar driver and base_link→laser TF; optional scan filter (downsampled scan).
- **Mapping:** Gmapping (2D), Cartographer (2D), RTAB-Map (RGB-D + scan).
- **Localization:** Cartographer localization, RTAB-Map localization, IMU+odom.
- **Navigation (Nav2):** DWA, TEB, Cartographer+DWB, RTAB-Map; costmaps use `/scan` and optionally depth (`/intel_realsense_r200_depth/points` or camera depth).
- **Maps:** Pre-built maps in `yahboomcar_nav/maps/`; save/load and display via launch files.

---

## Prerequisites

### Hardware Requirements

- Yahboom chassis (X3 Plus or R2) with base driver
- RPLidar (A1, S2, or 4ROS) for `/scan`
- For **camera + LIDAR** tests: RGB-D camera (e.g. Orbbec Astra, Intel RealSense) publishing `/camera/color/image_raw`, `/camera/depth/image_raw`, and optionally point cloud
- Sufficient open space for mapping and navigation (e.g. 2 m × 2 m minimum)
- Joystick (recommended for safety override during nav tests)

### Software Requirements

- ROS2 Humble installed and sourced
- Workspace built and sourced:

```bash
cd /home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws
source /opt/ros/humble/setup.bash
source install/setup.bash
```

### Required Packages

- `yahboomcar_nav` — Navigation launch files, params, maps, scan filter
- `yahboomcar_bringup` — Robot base (required by laser bringup)
- `nav2_bringup` — Nav2 stack (for navigation_* launches)
- `sllidar_ros2` — RPLidar A1/S2 driver
- `ydlidar_ros2_driver` — YDLidar 4ROS driver
- For RTAB-Map: `rtabmap_launch`, `rtabmap_sync`, `rtabmap_slam`
- For Cartographer: `cartographer_ros`

### Verify Package Installation

```bash
ros2 pkg list | grep -E 'yahboomcar_nav|nav2_bringup|sllidar|ydlidar|rtabmap|cartographer'
# Expect: yahboomcar_nav, nav2_bringup, and lidar/rtabmap/cartographer as applicable
```

---

## System Architecture

### Topic Flow — LIDAR-Only

```
RPLidar (A1 / S2 / 4ROS)
    └── /scan (sensor_msgs/LaserScan)

laser_bringup_launch.py
    ├── Brings up yahboomcar_bringup (robot_type: x3 / r2)
    ├── Brings up sllidar_ros2 or ydlidar_ros2_driver (rplidar_type: a1 / s2 / 4ROS)
    └── static_transform_publisher: base_link → laser

Optional: scan_filter (yahboomcar_nav)
    ├── Subscribes: /scan
    └── Publishes: /downsampled_scan

Gmapping / Cartographer
    ├── Subscribes: /scan (and /odom for Cartographer)
    └── Publishes: /map

Nav2 (DWA / TEB with scan-only costmap)
    ├── Subscribes: /scan, /odom, /map, /tf
    └── Action servers: navigate_to_pose, etc.
```

### Topic Flow — Camera + LIDAR (RTAB-Map)

```
RGB-D Camera
    ├── /camera/color/image_raw
    ├── /camera/color/camera_info
    └── /camera/depth/image_raw

RPLidar → /scan

rtabmap_sync_launch.py (via map_rtabmap_launch.py)
    ├── rgbd_sync: syncs RGB + depth
    ├── rtabmap: subscribe_rgbd=True, subscribe_scan=True
    └── Remappings: rgb/image, depth/image, odom
```

### Topic Flow — Camera + LIDAR (TEB / Carto+TEB costmap)

```
/scan (LaserScan)
/intel_realsense_r200_depth/points (PointCloud2) — or equivalent depth cloud

Nav2 (teb_nav_params / cartoteb_nav_params)
    ├── observation_sources: scan + point cloud
    └── Costmap uses both for obstacles
```

### Key Topics

| Topic | Type | Purpose |
|-------|------|---------|
| `/scan` | `sensor_msgs/LaserScan` | 2D laser scan (LIDAR) |
| `/downsampled_scan` | `sensor_msgs/LaserScan` | Downsampled scan (scan_filter) |
| `/odom` | `nav_msgs/Odometry` | Wheel odometry |
| `/map` | `nav_msgs/OccupancyGrid` | Map from SLAM or map_server |
| `/camera/color/image_raw` | `sensor_msgs/Image` | RGB (RTAB-Map) |
| `/camera/depth/image_raw` | `sensor_msgs/Image` | Depth (RTAB-Map) |
| `/intel_realsense_r200_depth/points` | `sensor_msgs/PointCloud2` | Depth cloud for costmap (TEB/Carto+TEB) |
| `/cmd_vel` | `geometry_msgs/Twist` | Velocity commands to chassis |

---

## Test Procedures

### Phase 1: LIDAR-Only — Laser Bringup and Scan

#### Test 1.1: Laser Bringup (LIDAR Only)

**Purpose:** Verify RPLidar and base are running and `/scan` is published.

**Steps:**

1. Set robot and lidar type (match hardware):
   ```bash
   export ROBOT_TYPE=x3
   export RPLIDAR_TYPE=a1
   ```
   (Use `a1`, `s2`, or `4ROS` for `RPLIDAR_TYPE`.)

2. Launch laser bringup:
   ```bash
   ros2 launch yahboomcar_nav laser_bringup_launch.py
   ```

3. Check nodes:
   ```bash
   ros2 node list | grep -E 'driver|sllidar|ydlidar|static_transform'
   ```

4. Check `/scan`:
   ```bash
   ros2 topic list | grep scan
   ros2 topic hz /scan
   ros2 topic echo /scan --once
   ```

5. Check TF (base_link → laser):
   ```bash
   ros2 run tf2_ros tf2_echo base_link laser
   ```

**Expected Results:**

- ✅ Driver and lidar nodes running
- ✅ `/scan` publishing at expected rate (e.g. 5–10 Hz)
- ✅ LaserScan has non-empty `ranges`, valid `angle_min`/`angle_max`
- ✅ TF from `base_link` to `laser` (or `laser_link`) exists

**Pass/Fail:** Pass if all checkmarks above are met.

---

#### Test 1.2: Scan Filter (Downsampled Scan)

**Purpose:** Verify optional scan_filter node downsamples `/scan` to `/downsampled_scan`.

**Steps:**

1. With laser bringup running, start the scan filter node (if run as standalone):
   ```bash
   ros2 run yahboomcar_nav scan_filter.py
   ```
   (If scan_filter is launched from another launch file, use that launch instead.)

2. Check topic:
   ```bash
   ros2 topic hz /downsampled_scan
   ros2 topic echo /downsampled_scan --once
   ```

3. Compare sizes: `len(ranges)` on `/downsampled_scan` should be about half of `/scan` (multiple=2 in code).

**Expected Results:**

- ✅ `/downsampled_scan` publishing
- ✅ Fewer range samples than `/scan` (downsampled by factor of 2)

**Pass/Fail:** Pass if `/downsampled_scan` is published and has fewer points than `/scan`.

---

### Phase 2: LIDAR-Only — Mapping (Gmapping / Cartographer)

#### Test 2.1: Gmapping (2D SLAM, LIDAR Only)

**Purpose:** Build a 2D occupancy grid map using only `/scan`.

**Steps:**

1. Start laser bringup (and base) in one terminal.
2. In another terminal, launch Gmapping (use the launch that matches your lidar; e.g. A1):
   ```bash
   ros2 launch yahboomcar_nav map_gmapping_a1_launch.py
   ```
   (Alternatives: `map_gmapping_launch.py`, `map_gmapping_s2_launch.py`, `map_gmapping_4ros_launch.py`.)

3. Check map topic:
   ```bash
   ros2 topic hz /map
   ros2 topic echo /map --once
   ```

4. Drive the robot slowly through the environment (joystick or teleop). Ensure coverage of the area.
5. Optionally open RViz with display_map or equivalent and confirm map updates.

**Expected Results:**

- ✅ `/map` publishes `nav_msgs/OccupancyGrid`
- ✅ Map resolution and dimensions are reasonable
- ✅ Map content updates as robot moves and matches environment (walls/obstacles)

**Pass/Fail:** Pass if `/map` is published and map looks consistent with the environment after a short drive.

---

#### Test 2.2: Cartographer (2D, LIDAR Only)

**Purpose:** Build a 2D map using Cartographer with `/scan`.

**Steps:**

1. Start laser bringup (and base).
2. Launch Cartographer mapping:
   ```bash
   ros2 launch yahboomcar_nav map_cartographer_launch.py
   ```
   (Or use `cartographer_launch.py` / `cartographer_bringup_launch.py` as provided.)

3. Check map and Cartographer nodes:
   ```bash
   ros2 topic hz /map
   ros2 node list | grep cartographer
   ```

4. Drive the robot and verify map building in RViz if available.

**Expected Results:**

- ✅ Cartographer node(s) running
- ✅ `/map` publishing
- ✅ Map builds and aligns as robot moves (no large drift in typical indoor run)

**Pass/Fail:** Pass if Cartographer runs and produces a coherent map.

---

### Phase 3: Camera + LIDAR — RTAB-Map

#### Test 3.1: RTAB-Map SLAM (RGB-D + Scan)

**Purpose:** Verify RTAB-Map SLAM with RGB-D camera and LIDAR.

**Prerequisites:** RGB-D camera running (e.g. Astra) publishing `/camera/color/image_raw`, `/camera/color/camera_info`, `/camera/depth/image_raw`. LIDAR publishing `/scan`.

**Steps:**

1. Start camera driver (e.g. `ros2 launch yahboomcar_astra astra.launch.py` or equivalent).
2. Start laser bringup.
3. Launch RTAB-Map mapping (this typically includes laser_bringup and rtabmap_sync):
   ```bash
   ros2 launch yahboomcar_nav map_rtabmap_launch.py
   ```

4. Check RTAB-Map and sync nodes:
   ```bash
   ros2 node list | grep -E 'rtabmap|rgbd_sync'
   ```

5. Check topics:
   ```bash
   ros2 topic list | grep -E 'map|camera|scan'
   ```

6. Move robot slowly; verify in RViz that map/visualization updates.

**Expected Results:**

- ✅ `rtabmap` and `rgbd_sync` (or equivalent) nodes running
- ✅ Camera and `/scan` topics available and used by RTAB-Map
- ✅ Map or RTAB-Map visualization updates when robot moves

**Pass/Fail:** Pass if RTAB-Map runs without errors and uses both camera and scan.

---

### Phase 4: Navigation (Nav2) — LIDAR-Only

#### Test 4.1: Nav2 with DWA (Scan-Only Costmap)

**Purpose:** Run Nav2 using only `/scan` for costmap (no camera).

**Prerequisites:** A pre-built map (e.g. from Gmapping/Cartographer) and a way to set initial pose and goal (RViz or CLI).

**Steps:**

1. Start laser bringup and base.
2. Launch Nav2 with DWA and map (adjust map path if needed):
   ```bash
   ros2 launch yahboomcar_nav navigation_dwa_launch.py
   ```
   Ensure `params_file` points to `dwa_nav_params.yaml` and map path is correct.

3. Verify Nav2 nodes:
   ```bash
   ros2 node list | grep -E 'controller_server|planner_server|bt_navigator|amcl'
   ```

4. Set initial pose in RViz (2D Pose Estimate) and send a goal (Nav2 Goal). Confirm robot plans and moves (or at least plans).

**Expected Results:**

- ✅ Nav2 nodes (controller, planner, bt_navigator, amcl) running
- ✅ Costmap uses `/scan` only (no depth)
- ✅ Path is planned and robot attempts to follow (or completes short goal)

**Pass/Fail:** Pass if Nav2 starts, localizes, and plans a path; optional: robot reaches goal.

---

#### Test 4.2: Nav2 with TEB (Scan-Only or Scan + Depth)

**Purpose:** Run Nav2 with TEB local planner. Params may use scan only or scan + depth cloud.

**Steps:**

1. Start laser bringup (and, if using depth, camera/depth cloud).
2. Launch TEB navigation:
   ```bash
   ros2 launch yahboomcar_nav navigation_teb_launch.py
   ```

3. If using depth in params (`/intel_realsense_r200_depth/points`), ensure that topic is published; otherwise use scan-only config.
4. Set initial pose and goal in RViz; verify planning and movement.

**Expected Results:**

- ✅ Nav2 with TEB starts
- ✅ Costmap receives `/scan` (and depth cloud if configured)
- ✅ TEB plans path and robot responds to goal

**Pass/Fail:** Pass if Nav2 TEB runs and responds to goals.

---

### Phase 5: Navigation — Camera + LIDAR (TEB / Carto+TEB with Depth)

#### Test 5.1: Nav2 TEB or Carto+TEB with Depth Point Cloud

**Purpose:** Verify costmap uses both `/scan` and depth point cloud (e.g. RealSense).

**Prerequisites:** Depth camera publishing point cloud (e.g. `/intel_realsense_r200_depth/points`). Map and localization running.

**Steps:**

1. Start laser bringup, base, and depth camera (point cloud).
2. Launch navigation that uses `teb_nav_params.yaml` or `cartoteb_nav_params.yaml` (with observation_sources including the point cloud topic).
3. In RViz, check costmap layers: both laser and point cloud should contribute to obstacles.
4. Set goal and verify obstacle avoidance uses both sensors.

**Expected Results:**

- ✅ Costmap shows obstacles from both `/scan` and depth cloud
- ✅ Nav2 plans and avoids obstacles from both sources

**Pass/Fail:** Pass if costmap clearly uses both and behavior is correct.

---

### Phase 6: Map Save / Load and Display

#### Test 6.1: Save Map

**Purpose:** Save current map (e.g. from Gmapping/Cartographer) to file.

**Steps:**

1. With a mapping pipeline running and map built, launch save_map:
   ```bash
   ros2 launch yahboomcar_nav save_map_launch.py
   ```
   (Confirm launch file name in package; adjust if it is different, e.g. `save_map_launch.py`.)

2. Trigger save (method depends on node used; e.g. service call or node logic).
3. Check that `.pgm` and `.yaml` (and `.pbstream` for Cartographer if applicable) are written to the configured path (e.g. `yahboomcar_nav/maps/`).

**Expected Results:**

- ✅ Map saved to disk
- ✅ `.yaml` references correct `.pgm` path

**Pass/Fail:** Pass if map files are created and loadable.

---

#### Test 6.2: Display Map and Display Nav

**Purpose:** Load a saved map and run Nav2 display (localization + Nav2).

**Steps:**

1. Start laser bringup and base.
2. Launch display_nav (or display_map first to confirm map loads):
   ```bash
   ros2 launch yahboomcar_nav display_nav_launch.py
   ```
   (Use correct map path argument if required.)

3. Verify map_server and AMCL (or localization) and Nav2 are running.
4. In RViz, confirm map is displayed and robot pose is localized; send a Nav2 goal.

**Expected Results:**

- ✅ Map loads and displays
- ✅ Localization and Nav2 run
- ✅ Robot can be given goals and plans path

**Pass/Fail:** Pass if map displays and navigation stack responds to goals.

---

## Summary Checklist

| Test | Description | Sensor | Pass |
|------|-------------|--------|------|
| 1.1 | Laser bringup | LIDAR | ☐ |
| 1.2 | Scan filter | LIDAR | ☐ |
| 2.1 | Gmapping | LIDAR | ☐ |
| 2.2 | Cartographer | LIDAR | ☐ |
| 3.1 | RTAB-Map SLAM | Camera + LIDAR | ☐ |
| 4.1 | Nav2 DWA | LIDAR | ☐ |
| 4.2 | Nav2 TEB | LIDAR (optional depth) | ☐ |
| 5.1 | Nav2 with depth costmap | Camera + LIDAR | ☐ |
| 6.1 | Save map | — | ☐ |
| 6.2 | Display map/nav | — | ☐ |

---

## References

- **Launch catalog:** [yahboomcar_nav_launch_catalog.md](../ROS2_Package_Docs/yahboomcar_nav_launch_catalog.md)
- **Package:** `yahboomcar_nav` — `launch/`, `params/`, `maps/`, `yahboomcar_nav/scan_filter.py`
- **Camera (for RTAB-Map / depth):** [Camera_Integration_Guide.md](../Camera/Camera_Integration_Guide.md), [YAHBOOMCAR_VISUAL_TEST_PLAN.md](../Camera/YAHBOOMCAR_VISUAL_TEST_PLAN.md)
