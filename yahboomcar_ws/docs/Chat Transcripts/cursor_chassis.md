# Chassis
_Exported on 2/1/2026 at 09:47:05 PST from Cursor (2.4.21)_

---

**User**

what is this fro /home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws/src/yahboomcar_nav

---

**Cursor**


Checking a couple of launch files and the Python module to summarize the package:



**`yahboomcar_nav`** is a ROS 2 package for **navigation** on the Yahboom car. It provides mapping, localization, and path planning for the robot.

## What it does

- **Mapping** – Build and load occupancy grid maps (Cartographer, Gmapping, RTAB-Map).
- **Localization** – Localize the robot on a map (Cartographer, RTAB-Map, IMU+odom).
- **Navigation** – Run Nav2 with different planners:
  - DWA (`navigation_dwa_launch.py`)
  - TEB (`navigation_teb_launch.py`)
  - Cartographer + DWB (`navigation_cartodwb_launch.py`)
  - RTAB-Map (`navigation_rtabmap_launch.py`)

## Package layout

| Part | Purpose |
|------|--------|
| **`launch/`** | Launch files for mapping, localization, Nav2, RTAB-Map, Cartographer, Gmapping, laser bringup, map display/save. |
| **`maps/`** | Pre-made maps (`.pgm`/`.yaml`) and Cartographer `.pbstream` files. |
| **`params/`** | Nav2 and Cartographer config (DWA, TEB, RTAB-Map, Cartographer Lua). |
| **`rviz/`** | RViz configs for map view and nav view. |
| **`yahboomcar_nav/`** | Python node `scan_filter.py` – downsamples laser scans and publishes to `/downsampled_scan`. |

So: **`yahboomcar_nav`** is the navigation stack for the Yahboom car: mapping, localization, Nav2 planners, and a laser scan filter, plus launch/config and maps.

---

**User**

so this is all based on the LIDAR or camra + Lidar

---

**Cursor**


Checking which sensors the nav package uses—LIDAR only or camera + LIDAR.



Checking RTAB-Map sync for camera/depth usage:



Here’s how sensors are used in **yahboomcar_nav**:

---

## LIDAR-only

These use **only** the laser (`/scan`):

- **Gmapping** – 2D SLAM from `/scan`
- **Cartographer** – 2D laser scans (`lds_2d.lua`: `num_laser_scans = 1`)
- **Laser bringup** – Starts RPLidar (A1, S2, or 4ROS) and publishes `/scan`
- **DWA navigation** – Costmap uses only `observation_sources: scan` → `/scan`
- **RTAB-Map Nav2 params** – Costmap uses only `/scan` (no camera in costmap)

So mapping and Nav2 can run with **LIDAR only**.

---

## Camera + LIDAR

These use **RGB-D/depth + LIDAR**:

- **RTAB-Map SLAM** (`rtabmap_sync_launch.py`):
  - `subscribe_rgbd: True`, `subscribe_scan: True`
  - Topics: `/camera/color/image_raw`, `/camera/color/camera_info`, `/camera/depth/image_raw`, plus scan
  - So RTAB-Map mapping is **RGB-D camera + LIDAR**.

- **TEB and Carto+TEB navigation** (`teb_nav_params.yaml`, `cartoteb_nav_params.yaml`):
  - Costmap has both:
    - `scan` → `/scan` (LaserScan)
    - depth cloud → `/intel_realsense_r200_depth/points`
  - So costmap is **LIDAR + depth (RealSense)**.

- **RViz** (e.g. `rtabmap_map.rviz`) is set up for both:
  - LaserScan `/scan`
  - Image `/camera/color/image_raw`
  - Frames: `camera_link`, `camera_depth_*`, `laser_link`

---

## Summary

| Use case              | Sensor setup        |
|-----------------------|---------------------|
| Gmapping, Cartographer, DWA nav, laser bringup | **LIDAR only** (`/scan`) |
| RTAB-Map SLAM         | **Camera + LIDAR** (RGB-D + `/scan`) |
| TEB / Carto+TEB nav   | **Camera + LIDAR** (RealSense depth + `/scan`) |

So the package supports **LIDAR-only** operation, and **camera + LIDAR** for RTAB-Map and for TEB/Carto+TEB costmaps.

---

**User**

please create a test plan for testing the above and put it in the /home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws/docs/Chassis folder


---

**Cursor**


Checking the Chassis folder and existing test-plan format in the docs:



Checking the nav launch catalog and another test plan for structure:



Creating the Chassis folder and a navigation test plan that matches the existing docs' structure.



Summary of what was added:

**Created:** `/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws/docs/Chassis/NAVIGATION_TEST_PLAN.md`

The test plan includes:

1. **Overview** — LIDAR-only vs camera + LIDAR modes and where each is used (Gmapping, Cartographer, RTAB-Map, DWA, TEB, etc.).

2. **Prerequisites** — Hardware (chassis, RPLidar, optional RGB-D), software (ROS2 Humble, workspace), and packages (`yahboomcar_nav`, `nav2_bringup`, lidar drivers, RTAB-Map, Cartographer).

3. **System architecture** — Topic flows for:
   - LIDAR-only (laser bringup, scan_filter, Gmapping/Cartographer, Nav2 DWA)
   - Camera + LIDAR (RTAB-Map RGB-D + scan; TEB/Carto+TEB costmap with depth cloud)
   - Key topics table (`/scan`, `/odom`, `/map`, camera/depth topics, `/cmd_vel`).

4. **Test procedures** (with steps, expected results, pass/fail):
   - **Phase 1:** Laser bringup, scan filter (downsampled scan).
   - **Phase 2:** Gmapping, Cartographer (LIDAR-only mapping).
   - **Phase 3:** RTAB-Map SLAM (RGB-D + scan).
   - **Phase 4:** Nav2 DWA and Nav2 TEB (LIDAR-only or scan-only costmap).
   - **Phase 5:** Nav2 with depth costmap (TEB/Carto+TEB using depth point cloud).
   - **Phase 6:** Save map, display map/nav.

5. **Summary checklist** — Table of all tests with a pass column to tick off.

6. **References** — Links to `yahboomcar_nav_launch_catalog.md` and camera docs.

The layout and style follow your existing docs (e.g. `PERSISTENT_TRACKING_TEST_GUIDE.md`, `FACIAL_GESTURE_RECOGNITION_TESTING_PLAN.md`): prerequisites, architecture, step-by-step tests, and explicit pass/fail criteria.

---

**User**

plz add, commit and push to git

---

**Cursor**


Checking whether the commit that added the Chassis doc has been pushed:



Adding only the Chassis doc (excluding build artifacts), then committing and pushing:



Chassis doc is already committed. Pushing to origin.



Summary:

- **Add:** `yahboomcar_ws/docs/Chassis/NAVIGATION_TEST_PLAN.md` is already tracked and matches the last commit (nothing new to add).
- **Commit:** It’s already committed in `a9bb18a7` (“Object tracking and camera: UVC YUV422->BGR fix, Chassis nav test plan”).
- **Push:** `git push origin main` completed; remote reported **“Everything up-to-date”**.

So the navigation test plan in `docs/Chassis/` is already in the repo and pushed to `origin/main`. No further action was required.

