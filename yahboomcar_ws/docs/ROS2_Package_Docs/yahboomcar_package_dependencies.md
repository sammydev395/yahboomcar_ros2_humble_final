# Yahboom ROSMASTER X3Plus Package Dependencies and Conflicts

## Core System Packages (Must Run Together)
These packages form the basic robot functionality and should typically run together:

1. **Base System**
   - `yahboomcar_bringup` - Core driver and hardware interface
     - CPU Usage: ~5-10%
     - Memory: ~100MB
     - Topics: 10-15
     - Critical for all other packages
   - `yahboomcar_description` - Robot description (URDF)
     - CPU Usage: ~1-2%
     - Memory: ~50MB
     - Static package, no runtime overhead
   - `yahboomcar_msgs` - Custom message definitions
     - No runtime overhead
     - Required by all custom packages
   - `yahboomcar_ctrl` - Basic control interface
     - CPU Usage: ~2-5%
     - Memory: ~50MB
     - Topics: 5-8

2. **Sensor Processing**
   - `yahboomcar_laser` - Lidar processing
     - CPU Usage: ~10-15%
     - Memory: ~200MB
     - Topics: 8-12
     - Bandwidth: ~5-10MB/s
   - `yahboomcar_astra` - Camera processing
     - CPU Usage: ~15-20%
     - Memory: ~300MB
     - Topics: 6-10
     - Bandwidth: ~20-30MB/s

## Independent Functionality Packages
These packages can run independently but may have resource conflicts:

### Navigation Stack
- `yahboomcar_nav` - Navigation stack
  - CPU Usage: ~20-30%
  - Memory: ~400MB
  - Topics: 15-20
  - Dependencies: lidar, odometry, tf
- `yahboomcar_slam` - SLAM functionality
  - CPU Usage: ~30-40%
  - Memory: ~500MB
  - Topics: 20-25
  - Dependencies: lidar, odometry, tf
- **Potential Conflicts**:
  - Both use the same sensor data (lidar, odometry)
  - Can't run SLAM and navigation simultaneously
  - May conflict with other packages using the same sensors

### Vision Processing
- `yahboomcar_visual` - General vision processing
  - CPU Usage: ~25-35%
  - Memory: ~400MB
  - Topics: 10-15
  - Dependencies: camera
- `yahboomcar_mediapipe` - MediaPipe-based vision
  - CPU Usage: ~40-50%
  - Memory: ~600MB
  - Topics: 12-18
  - Dependencies: camera, MediaPipe
- `arm_mediapipe` - Arm-specific MediaPipe processing
  - CPU Usage: ~35-45%
  - Memory: ~500MB
  - Topics: 10-15
  - Dependencies: camera, MediaPipe, arm
- **Potential Conflicts**:
  - All use camera resources
  - Can't run multiple vision processing nodes simultaneously
  - May conflict with `yahboomcar_astra` camera processing

### Arm Control
- `arm_moveit_demo` - MoveIt-based arm control
  - CPU Usage: ~15-25%
  - Memory: ~300MB
  - Topics: 20-30
  - Dependencies: MoveIt, tf
- `x3plus_moveit_config` - MoveIt configuration
  - CPU Usage: ~5-10%
  - Memory: ~100MB
  - Topics: 5-10
  - Dependencies: MoveIt
- `arm_autopilot` - Autonomous arm control
  - CPU Usage: ~20-30%
  - Memory: ~250MB
  - Topics: 15-20
  - Dependencies: camera, arm
- `arm_color_transport` - Color-based arm control
  - CPU Usage: ~15-25%
  - Memory: ~200MB
  - Topics: 10-15
  - Dependencies: camera, arm
- **Potential Conflicts**:
  - All control the same physical arm
  - Can't run multiple arm control systems simultaneously
  - May conflict with base movement if not properly coordinated

### Specialized Functionality
- `yahboomcar_linefollw` - Line following
  - CPU Usage: ~10-15%
  - Memory: ~150MB
  - Topics: 8-12
  - Dependencies: camera
- `yahboomcar_voice_ctrl` - Voice control
  - CPU Usage: ~5-10%
  - Memory: ~100MB
  - Topics: 5-8
  - Dependencies: microphone
- `yahboomcar_multi` - Multi-robot control
  - CPU Usage: ~10-20%
  - Memory: ~200MB
  - Topics: 15-25
  - Dependencies: network
- **Potential Conflicts**:
  - Line following may conflict with navigation
  - Voice control may conflict with other control systems
  - Multi-robot control needs careful coordination

## Detailed Safe Package Combinations

### 1. Basic Navigation Setup
```bash
# Start base system
roslaunch yahboomcar_bringup bringup.launch
# Start navigation
roslaunch yahboomcar_nav nav.launch
```
- Total CPU Usage: ~35-45%
- Total Memory: ~700MB
- Safe to run with: voice control, basic arm control

### 2. Vision Processing Setup
```bash
# Start base system
roslaunch yahboomcar_bringup bringup.launch
# Start vision processing
roslaunch yahboomcar_mediapipe mediapipe.launch
```
- Total CPU Usage: ~50-60%
- Total Memory: ~900MB
- Safe to run with: voice control, basic arm control

### 3. Arm Control Setup
```bash
# Start base system
roslaunch yahboomcar_bringup bringup.launch
# Start MoveIt
roslaunch arm_moveit_demo moveit.launch
```
- Total CPU Usage: ~30-40%
- Total Memory: ~500MB
- Safe to run with: voice control, basic navigation

### 4. Voice Control Setup
```bash
# Start base system
roslaunch yahboomcar_bringup bringup.launch
# Start voice control
roslaunch yahboomcar_voice_ctrl voice_ctrl.launch
```
- Total CPU Usage: ~15-25%
- Total Memory: ~250MB
- Safe to run with: most other packages

## Detailed Unsafe Combinations

### 1. Navigation Conflicts
```bash
# UNSAFE - Don't run these together
roslaunch yahboomcar_nav nav.launch
roslaunch yahboomcar_slam slam.launch
```
- Reason: Both try to use lidar and odometry data
- Symptoms: 
  - High CPU usage (>80%)
  - Memory spikes
  - Navigation instability
  - TF tree conflicts

### 2. Vision Processing Conflicts
```bash
# UNSAFE - Don't run these together
roslaunch yahboomcar_visual visual.launch
roslaunch yahboomcar_mediapipe mediapipe.launch
```
- Reason: Both try to access camera simultaneously
- Symptoms:
  - Camera access errors
  - High CPU usage (>90%)
  - Memory spikes
  - Frame drops

### 3. Arm Control Conflicts
```bash
# UNSAFE - Don't run these together
roslaunch arm_moveit_demo moveit.launch
roslaunch arm_autopilot autopilot.launch
```
- Reason: Both try to control the same physical arm
- Symptoms:
  - Arm movement conflicts
  - Joint limit errors
  - Safety system triggers
  - Unpredictable arm behavior

### 4. Control System Conflicts
```bash
# UNSAFE - Don't run these together
roslaunch yahboomcar_voice_ctrl voice_ctrl.launch
roslaunch yahboomcar_ctrl yahboom_joy.launch
```
- Reason: Both try to control robot movement
- Symptoms:
  - Conflicting velocity commands
  - Erratic robot movement
  - Control system instability

## Resource Management Guidelines

### CPU Usage Thresholds
- Safe: <70% total CPU usage
- Warning: 70-85% total CPU usage
- Critical: >85% total CPU usage

### Memory Usage Thresholds
- Safe: <1.5GB total memory
- Warning: 1.5-2GB total memory
- Critical: >2GB total memory

### Bandwidth Usage
- Camera: ~20-30MB/s
- Lidar: ~5-10MB/s
- Total safe bandwidth: <50MB/s

## Troubleshooting Common Issues

### 1. High CPU Usage
```bash
# Check CPU usage
top -b -n 1 | grep ros
# Check specific node
rosnode info /node_name
```

### 2. Memory Issues
```bash
# Check memory usage
free -h
# Check specific process
ps aux | grep ros
```

### 3. Topic Conflicts
```bash
# Check topic connections
rostopic info /topic_name
# Check topic frequency
rostopic hz /topic_name
```

### 4. TF Tree Issues
```bash
# Check TF tree
rosrun tf view_frames
# Check specific transform
rosrun tf tf_echo frame1 frame2
```

## Best Practices

1. **Always start with base system**:
   ```bash
   roslaunch yahboomcar_bringup bringup.launch
   ```

2. **Add functionality incrementally**:
   - Start with one major functionality (navigation, vision, or arm)
   - Test thoroughly before adding another
   - Monitor system resources during testing

3. **Monitor system resources**:
   - Watch CPU and memory usage
   - Check for topic conflicts using `rostopic info`
   - Monitor TF tree for conflicts
   - Use `htop` or `top` for real-time monitoring

4. **Use namespaces when possible**:
   - Helps avoid topic and parameter conflicts
   - Makes it easier to run multiple instances
   - Example:
     ```bash
     roslaunch yahboomcar_nav nav.launch ns:=nav1
     roslaunch yahboomcar_nav nav.launch ns:=nav2
     ```

5. **Check launch file parameters**:
   - Review remappings
   - Check for conflicting parameters
   - Verify hardware resource usage
   - Use `roslaunch --screen` for better debugging

6. **System Health Monitoring**:
   ```bash
   # Monitor all ROS nodes
   rosnode list
   # Check node statistics
   rosnode info /node_name
   # Monitor topic statistics
   rostopic hz /topic_name
   # Check system diagnostics
   rosrun rqt_robot_monitor rqt_robot_monitor
   ``` 