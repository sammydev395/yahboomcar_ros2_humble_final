# yahboomcar_description Package Catalog

---

## Overview
**Path:** `src/yahboomcar_description/`

Provides the full robot model (URDF/Xacro), 3D meshes, and visualization tools for Yahboom ROSMASTER robots (X3, X3plus, etc.), supporting simulation, state publishing, and RViz display.

---

## URDF/Xacro Files (`src/yahboomcar_description/urdf/`)

### src/yahboomcar_description/urdf/yahboomcar_X3.urdf
**Purpose:** URDF for the X3 robot base.

### src/yahboomcar_description/urdf/yahboomcar_X3.urdf.xacro
**Purpose:** Xacro (macro-enabled URDF) for the X3 robot base.

### src/yahboomcar_description/urdf/yahboomcar_X3.gazebo.urdf.xacro
**Purpose:** Gazebo simulation Xacro for the X3 robot, includes simulation plugins.

### src/yahboomcar_description/urdf/yahboomcar_X3.gazebo.xacro
**Purpose:** Additional Gazebo plugin and wheel parameter definitions for X3.

### src/yahboomcar_description/urdf/yahboomcar_X3plus.urdf
**Purpose:** URDF for the X3plus robot base.

### src/yahboomcar_description/urdf/yahboomcar_X3plus.urdf.xacro
**Purpose:** Xacro (macro-enabled URDF) for the X3plus robot base.

### src/yahboomcar_description/urdf/yahboomcar_X3plus.gazebo.urdf.xacro
**Purpose:** Gazebo simulation Xacro for the X3plus robot, includes simulation plugins.

### src/yahboomcar_description/urdf/yahboomcar_X3plus.gazebo.xacro
**Purpose:** Additional Gazebo plugin and wheel parameter definitions for X3plus.

### src/yahboomcar_description/urdf/sensors/camera.xacro
**Purpose:** Modular camera sensor definition for inclusion in robot models.

### src/yahboomcar_description/urdf/sensors/imu.xacro
**Purpose:** Modular IMU sensor definition for inclusion in robot models.

### src/yahboomcar_description/urdf/sensors/lidar.xacro
**Purpose:** Modular lidar sensor definition for inclusion in robot models.

---

## Meshes (`src/yahboomcar_description/meshes/`)

### src/yahboomcar_description/meshes/X3/
**Purpose:** STL models for X3 robot and mecanum wheels.

### src/yahboomcar_description/meshes/X3/mecanum/
**Purpose:** STL/DAE models for X3 mecanum wheels and sensors.

### src/yahboomcar_description/meshes/X3plus/visual/
**Purpose:** Visual meshes for X3plus robot and arm links.

### src/yahboomcar_description/meshes/X3plus/collision/
**Purpose:** Collision meshes for X3plus robot and arm links.

### src/yahboomcar_description/meshes/sensor/visual/
**Purpose:** Visual meshes for camera, lidar, and mono sensors.

### src/yahboomcar_description/meshes/sensor/collision/
**Purpose:** Collision meshes for camera, lidar, and mono sensors.

---

## Launch Files (`src/yahboomcar_description/launch/`)

### src/yahboomcar_description/launch/display.launch
**Purpose:**
- Visualize the robot model in RViz
- Loads robot description (URDF/Xacro)
- Starts joint state publisher (with or without GUI)
- Starts robot state publisher
- Launches RViz with Yahboomcar config
**Args:** `ns`, `format` (urdf/xacro), `use_gui`, `robot_type`

---

## RViz Config (`src/yahboomcar_description/rviz/`)

### src/yahboomcar_description/rviz/yahboomcar.rviz
**Purpose:** Preconfigured RViz display for the Yahboom robot, showing all links, joints, and sensor frames.

---

**This package is required for all Yahboom robot simulation, visualization, and state publishing.** 