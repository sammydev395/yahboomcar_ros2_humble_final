# Yahboomcar Astra Launch Files Catalog

## Overview
This catalog documents the launch files in the `yahboomcar_astra` package, which provides functionality for the Orbbec Astra depth camera, including object tracking, depth sensing, and visual processing.

## Package Dependencies
### Hardware Dependencies
- Yahboom ROSMASTER X3Plus robot hardware
- Orbbec Astra Pro Plus camera
- Sufficient CPU/GPU for image processing
- Optional: Display for visualization

### Software Dependencies
- ROS2 Humble
- OpenCV
- NumPy
- yahboomcar_bringup
- yahboomcar_ctrl
- orbbec_camera

## Automatic Dependencies
Each launch file automatically includes and manages its dependencies through included launch files. Here's what gets started automatically:

### Fundamental Dependencies
- yahboomcar_description
  - Robot model (URDF)
  - TF transforms
  - Joint configurations
  - Sensor configurations
  - Required by ALL launch files that need robot model or transforms
- yahboomcar_msgs
  - Custom message definitions
  - Service definitions
  - Required by ALL launch files that use custom messages

### Base System Dependencies
- yahboomcar_bringup
  - Robot drivers
  - TF transforms
  - Core functionality
  - Depends on yahboomcar_description and yahboomcar_msgs
- yahboomcar_ctrl
  - Joystick control
  - /JoyState topic
  - /cmd_vel topic
  - Depends on yahboomcar_msgs
- Camera Driver
  - /camera/rgb/image_raw
  - /camera/depth/image_raw

### Processing Dependencies
- OpenCV
- NumPy
- ROS2 image processing tools

## Launch Files

### 1. KCFTracker.launch.py
**Purpose:** Object tracking using the KCF (Kernelized Correlation Filter) algorithm with Orbbec Astra Pro Plus camera input.

**Nodes/Includes:**
- `KCFTracker_node` (runs KCFTracker_node executable)
- Includes `yahboomcar_bringup/launch/yahboomcar.launch.py`
- Includes `yahboomcar_ctrl/launch/yahboom_joy.launch.py`
- Includes `orbbec_camera/launch/astraproplus.launch.py`

**Implementation Details:**
- **Object Selection and Tracking:**
  - Select object by drawing rectangle with mouse
  - Real-time object following
  - Yellow rectangle around tracked object
  - Red dot at object center

- **Depth Sensing:**
  - Uses Orbbec Astra Pro Plus depth sensor for distance measurement
  - Takes 5 depth measurements around object center
  - Averages depth values for stability
  - Ignores values < 0.4m or > 10m

- **Robot Control:**
  - PID controllers for:
    - Distance maintenance (minDist)
    - Object centering
  - Velocity commands to /cmd_vel:
    - Linear speed based on distance
    - Angular speed based on horizontal position

- **Visualization:**
  - Blue rectangle: Selection area
  - Yellow rectangle: Tracking box
  - Red dot: Object center
  - Published to /KCF_image

- **Controls:**
  - Mouse: Object selection
  - 'q' or ESC: Quit
  - 'r': Reset
  - Space: Enable depth following

- **PID Control:**
  - Linear PID: Distance maintenance
  - Angular PID: Centering control
  - ROS2 parameter system for tuning

**Topics:**
- Subscribes to:
  - `/camera/rgb/image_raw`: RGB camera feed
  - `/camera/depth/image_raw`: Depth camera feed
  - `/JoyState`: Joystick control state
- Publishes:
  - `/KCF_image`: Tracking visualization
  - `/cmd_vel`: Robot movement commands

**Automatic Dependencies:**
- Camera Driver:
  - Automatically starts the Orbbec Astra Pro Plus camera driver
  - Provides RGB and depth image streams
- Base System:
  - Includes yahboomcar_bringup
  - Provides TF and core functionality
- Control System:
  - Includes yahboomcar_ctrl
  - Provides joystick control

**Safe Combinations:**
- Can be run with:
  - yahboomcar_bringup (base system)
  - Navigation stack
  - SLAM nodes
  - Other visualization tools

**Unsafe Combinations:**
- Do not run with other object tracking nodes
- Do not run with other camera drivers
- Do not run with other joystick control nodes

**Usage Example:**
```bash
# Simply run the launch file - all dependencies are handled automatically
roslaunch yahboomcar_astra KCFTracker.launch.py
```

**Troubleshooting:**
- If camera doesn't start, check USB connection
- If tracking fails, ensure good lighting
- If robot doesn't move, verify joystick connection
- Verify all required topics are available:
  ```bash
  rostopic list | grep -E "camera|JoyState|cmd_vel|KCF"
  ```

**Launch File Dependencies:**
- Direct Dependencies:
  - `yahboomcar_bringup/launch/yahboomcar.launch.py`
    - Starts base robot system
    - Provides driver_node and odometry
    - Depends on yahboomcar_description and yahboomcar_msgs
  - `yahboomcar_ctrl/launch/yahboom_joy.launch.py`
    - Provides joystick control
    - Depends on yahboomcar_bringup and yahboomcar_msgs
  - `orbbec_camera/launch/astraproplus.launch.py`
    - Starts camera driver
    - Independent of other launch files

- Dependency Chain:
  ```
  KCFTracker.launch.py
  ├── yahboomcar.launch.py
  │   ├── yahboomcar_description
  │   │   └── (robot model and transforms)
  │   └── yahboomcar_msgs
  │       └── (message definitions)
  ├── yahboom_joy.launch.py
  │   ├── yahboomcar.launch.py
  │   └── yahboomcar_msgs
  └── astraproplus.launch.py
  ```

- Circular Dependencies: None
  - All dependencies form a directed acyclic graph
  - Each included launch file has a clear hierarchy

### 2. astra.launch
**Purpose:** Basic Astra camera initialization and configuration.

**Implementation Details:**
- **Camera Setup:**
  - Initializes RGB and depth streams
  - Configures camera parameters
  - Sets up image processing pipeline

- **Image Processing:**
  - RGB image processing
  - Depth image processing
  - Synchronization between streams

- **Topic Management:**
  - Standard ROS topic naming
  - Camera info publishing
  - Image format conversion

**Nodes:**
- Astra camera driver node
- Image processing nodes

**Topics:**
- Publishes:
  - `/camera/rgb/image_raw`: RGB camera feed
  - `/camera/depth/image_raw`: Depth camera feed
  - `/camera/rgb/camera_info`: Camera calibration info
  - `/camera/depth/camera_info`: Depth camera calibration info

**Launch File Dependencies:**
- Direct Dependencies:
  - yahboomcar_description (for camera TF)
  - yahboomcar_msgs (for custom messages)

- Dependency Chain:
  ```
  astra.launch
  ├── yahboomcar_description
  │   └── (camera TF configuration)
  └── yahboomcar_msgs
      └── (message definitions)
  ```

- Circular Dependencies: None
  - This is a leaf node in the dependency tree
  - Other launch files depend on it, but it doesn't depend on others

### 3. astra_scan.launch
**Purpose:** 3D point cloud generation from depth data.

**Implementation Details:**
- **Point Cloud Generation:**
  - Depth to point cloud conversion
  - Coordinate transformation
  - Point cloud filtering

- **Processing Pipeline:**
  - Depth image preprocessing
  - Point cloud generation
  - Basic filtering

- **Visualization:**
  - Point cloud visualization
  - Color mapping
  - Depth coloring

**Nodes:**
- Point cloud generator node
- Depth to point cloud converter

**Topics:**
- Subscribes to:
  - `/camera/depth/image_raw`: Depth camera feed
  - `/camera/rgb/camera_info`: Camera calibration
- Publishes:
  - `/camera/depth/points`: 3D point cloud data

**Launch File Dependencies:**
- Direct Dependencies:
  - `orbbec_camera/launch/astraproplus.launch.py`
    - Provides camera data
    - Required for point cloud generation
    - Depends on yahboomcar_description and yahboomcar_msgs
  - yahboomcar_description (for point cloud TF)
  - yahboomcar_msgs (for point cloud messages)

- Dependency Chain:
  ```
  astra_scan.launch
  ├── astraproplus.launch.py
  │   ├── yahboomcar_description
  │   └── yahboomcar_msgs
  ├── yahboomcar_description
  └── yahboomcar_msgs
  ```

- Circular Dependencies: None
  - Simple linear dependency on astraproplus.launch.py
  - No circular references

### 4. astra_scan_filter.launch
**Purpose:** Filtered point cloud processing for improved quality.

**Implementation Details:**
- **Filtering Pipeline:**
  - Statistical outlier removal
  - Voxel grid downsampling
  - Noise reduction
  - Point cloud smoothing

- **Processing Steps:**
  1. Raw point cloud input
  2. Statistical outlier removal
  3. Voxel grid filtering
  4. Smoothing
  5. Filtered output

- **Filter Parameters:**
  - Outlier removal threshold
  - Voxel size
  - Smoothing factor
  - Filter iterations

**Nodes:**
- Point cloud filter node
- Statistical outlier removal
- Voxel grid filter

**Topics:**
- Subscribes to:
  - `/camera/depth/points`: Raw point cloud
- Publishes:
  - `/camera/depth/points_filtered`: Filtered point cloud

**Launch File Dependencies:**
- Direct Dependencies:
  - `orbbec_camera/launch/astraproplus.launch.py`
    - Provides camera data
    - Required for point cloud generation
    - Depends on yahboomcar_description and yahboomcar_msgs
  - `orbbec_camera/launch/astra_scan.launch.py`
    - Provides point cloud generation
    - Already includes astraproplus.launch.py
    - Depends on yahboomcar_description and yahboomcar_msgs
  - yahboomcar_description (for filtered point cloud TF)
  - yahboomcar_msgs (for filtered point cloud messages)

- Dependency Chain:
  ```
  astra_scan_filter.launch
  ├── astra_scan.launch.py
  │   ├── astraproplus.launch.py
  │   │   ├── yahboomcar_description
  │   │   └── yahboomcar_msgs
  │   ├── yahboomcar_description
  │   └── yahboomcar_msgs
  ├── orbbec_camera.launch.py
  │   ├── yahboomcar_description
  │   └── yahboomcar_msgs
  ├── yahboomcar_description
  └── yahboomcar_msgs
  ```

- Circular Dependencies: None
  - Dependencies form a tree structure
  - No circular references
  - Note: astraproplus.launch.py is included twice, but this is not a circular dependency

## Dependency Analysis Summary

### Launch File Hierarchy
```
KCFTracker.launch.py
├── yahboomcar.launch.py
│   ├── yahboomcar_description
│   └── yahboomcar_msgs
├── yahboom_joy.launch.py
│   ├── yahboomcar.launch.py
│   └── yahboomcar_msgs
└── astraproplus.launch.py
    ├── yahboomcar_description
    └── yahboomcar_msgs

astra_scan.launch.py
├── astraproplus.launch.py
│   ├── yahboomcar_description
│   └── yahboomcar_msgs
├── yahboomcar_description
└── yahboomcar_msgs

astra_scan_filter.launch
├── astra_scan.launch.py
│   ├── astraproplus.launch.py
│   │   ├── yahboomcar_description
│   │   └── yahboomcar_msgs
│   ├── yahboomcar_description
│   └── yahboomcar_msgs
├── orbbec_camera.launch.py
│   ├── yahboomcar_description
│   └── yahboomcar_msgs
├── yahboomcar_description
└── yahboomcar_msgs
```

### Key Findings
1. No circular dependencies exist in the launch files
2. All dependencies form directed acyclic graphs
3. yahboomcar_description and yahboomcar_msgs are fundamental dependencies
4. Base launch files (astraproplus.launch.py, yahboomcar.launch.py) depend on fundamental packages
5. Complex launch files have multiple paths to fundamental dependencies
6. Some launch files include the same dependencies multiple times

### Best Practices
1. Always ensure yahboomcar_description and yahboomcar_msgs are installed
2. Start with fundamental packages before launching other nodes
3. Launch files can be run independently if their dependencies are already running
4. Multiple inclusions of the same launch file are handled by ROS
5. Dependencies are automatically managed by ROS launch system
6. Check package.xml for all required dependencies
7. Monitor system resources when running multiple nodes

## Usage Examples

### Basic Camera Setup
```bash
roslaunch yahboomcar_astra astra.launch
```

### Object Tracking
```bash
roslaunch yahboomcar_astra KCFTracker.launch.py
```

### Point Cloud Generation
```bash
roslaunch yahboomcar_astra astra_scan.launch.py
```

### Filtered Point Cloud
```bash
roslaunch yahboomcar_astra astra_scan_filter.launch
```

## Dependencies
- OpenCV
- PCL (Point Cloud Library)
- Astra SDK
- ROS2 Humble

## Configuration
- Camera parameters can be adjusted in the launch files
- KCF tracking parameters can be tuned using ROS2 parameter system
- Point cloud filter parameters can be modified in the launch files

## Troubleshooting
1. Camera not detected:
   - Check USB connection
   - Verify camera permissions
   - Restart the camera node

2. Tracking issues:
   - Adjust lighting conditions
   - Check object contrast
   - Tune PID parameters

3. Point cloud problems:
   - Verify depth sensor calibration
   - Check filter parameters
   - Ensure proper lighting conditions

## General Tips
- For best results, use in well-lit environments
- Make sure the Astra camera is properly connected
- Use `rqt_image_view` to verify camera input
- Check ROS logs for errors if nodes do not start
- Monitor CPU usage when running tracking nodes
- Use `rostopic list` to verify all required topics
- Use `rosnode info` to check node status
- All launch files automatically handle their dependencies
- Use `roslaunch --screen` to see detailed output
- Check package.xml for all required dependencies
- Keep tracking area clear of obstacles
- Follow tracking procedures carefully
- Monitor system logs for errors
- Use appropriate launch file for specific needs
- Test tracking in safe environment
- Verify all hardware connections
- Check tracking parameters
- Monitor visualization output

