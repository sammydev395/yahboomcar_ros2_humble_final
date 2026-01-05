# ArmPi FPV Menu System Migration Plan to Yahboom Rosmaster X3 Plus

**Date**: January 2025  
**Source**: `/home/jetson/mnt/armpi_fpv/armpi_fpv` (mounted via sshfs from `/home/sammydev295` on armpi_fpv)  
**Target Workspace**: `/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws`  
**Target Platform**: Jetson Nano Super (Orin-based) Rosmaster X3 Plus  
**Current Camera**: Microdia USB Camera (ID 0c45:6340) - will be replaced with Arducam/OAK-D-Lite  
**ROS2 Distribution**: Both systems use **Ubuntu 22.04 with ROS2 Humble** (no version migration needed)

---

## Quick Summary: What's Needed vs Not Needed

### ✅ **FILES TO MIGRATE**:
- ✅ `launch_armpi_fpv_tuning.sh` → Migrate and adapt (update paths, use Yahboom nodes)
- ❌ `launch_armpi_fpv.sh` → **WILL BE DELETED** (not needed, only tuning script is migrated)
- ✅ `turn_off_leds.py` → Adapt for Yahboom strip LEDs (different message types)
- ✅ `get_ups_status.py` → **REWRITE** for Waveshare UPS Power Module (C) (different from Lumi-Electronics)
- ✅ `test_voltage_monitor.sh` → May work as-is (uses standard RPi commands)
- ✅ `config/color_ranges.yaml` → Copy as-is
- ✅ `config/home_pose.json` → Copy as-is
- ✅ `tracking_*.yaml` → Copy as-is (if tracking is used)
- ✅ `test/arducam_to_usbcam_bridge.py` → Update for Arducam integration
- ✅ `src/lab_config/` → **NEEDED** for camera calibration (no existing tools in yahboom code)

### ❌ **FILES NOT NEEDED** (Hiwonder-Specific):
- ❌ `scripts/jog_keys.py` → Uses `hiwonder_servo_msgs` (not relevant for Yahboom)
- ❌ `scripts/jog_publisher.py` → Uses `hiwonder_servo_msgs` (not relevant for Yahboom)
- ❌ `src/armpi_fpv_utils/` → Uses `hiwonder_servo_msgs` (not relevant for Yahboom)

### 🔄 **KEY ADAPTATIONS REQUIRED**:
1. **Menu Script**: Replace Hiwonder nodes with Yahboom equivalents
2. **UPS Module**: Rewrite `get_ups_status.py` for Waveshare module (I2C, INA219)
3. **LED Control**: Adapt `turn_off_leds.py` for Yahboom strip LEDs
4. **Camera Integration**: Support current Microdia USB → Arducam/OAK-D-Lite migration
5. **Camera Calibration**: Use `lab_config` package (no existing calibration in yahboom code)

### 📋 **SYSTEM DIFFERENCES**:
- **Platform**: Hiwonder ArmPi FPV → Yahboom Rosmaster X3 Plus
- **ROS2**: Both use Humble (no version migration needed)
- **Python**: Both use 3.10 (Ubuntu 22.04)
- **Servo System**: Hiwonder → Yahboom (different messages/controllers)
- **UPS Module**: Lumi-Electronics → Waveshare UPS Power Module (C)
- **LEDs**: Hiwonder RGB LEDs → Yahboom strip LEDs

---

## Executive Summary

This document outlines the migration plan for bringing the ArmPi FPV menu system (`launch_armpi_fpv_tuning.sh`) and associated Python scripts to the Yahboom Rosmaster X3 Plus platform. The migration includes:

1. **Menu System**: Interactive bash menu script for controlling ROS2 nodes
2. **Python Scripts**: Utility scripts for LED control (strip LEDs), UPS monitoring (Waveshare UPS Module C)
3. **Configuration Files**: Color calibration (lab_config may be needed for camera calibration)
4. **Camera Support**: 
   - **Arm Camera**: Current inexpensive USB camera (on arm/gripper) - will be replaced with Arducam or OAK-D-Lite
   - **Astra Camera**: Fixed mount, powerful depth camera - already working, no migration needed
   - **Arducam IMX477 CSI Camera** - to be integrated (replaces current USB camera)
   - **OAK-D-Lite HD Camera** - future integration option
5. **ROS2 Package Migration**: `lab_config` package (for camera calibration - no existing calibration tools in yahboom code)

---

## 1. Files to Copy from Source

### 1.1 Root Level Scripts

**Source Location**: `/home/jetson/mnt/armpi_fpv/armpi_fpv/`

**Files to Copy**:
- ✅ `launch_armpi_fpv_tuning.sh` - Main launch script (migrate and adapt for Yahboom)
- ❌ `launch_armpi_fpv.sh` - **NOT NEEDED** (will be deleted, only tuning script is migrated)
- ✅ `test_voltage_monitor.sh` - Voltage monitoring script
- ✅ `turn_off_leds.py` - LED control script
- ✅ `get_ups_status.py` - UPS status reader (needs path updates)
- ✅ `tracking_distant_params.yaml` - Tracking parameters for distant objects
- ✅ `tracking_smooth_params.yaml` - Tracking parameters for smooth tracking

**Target Location**: `/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws/`

### 1.2 Scripts Folder

**Source**: `/home/jetson/mnt/armpi_fpv/armpi_fpv/scripts/`

**Files to Copy**:
- ❌ `scripts/jog_keys.py` - **NOT NEEDED** (uses hiwonder_servo_msgs, not relevant for Yahboom)
- ❌ `scripts/jog_publisher.py` - **NOT NEEDED** (uses hiwonder_servo_msgs, not relevant for Yahboom)

**Note**: These scripts use Hiwonder-specific servo messages which are not compatible with Yahboom's servo system. Yahboom has its own message definitions.

### 1.3 Config Folder

**Source**: `/home/jetson/mnt/armpi_fpv/armpi_fpv/config/`

**Files to Copy**:
- ✅ `config/color_ranges.yaml` - LAB color calibration ranges
- ✅ `config/home_pose.json` - Saved home pose configuration

**Target Location**: `/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws/config/`

### 1.4 Test Folder (Camera Integration)

**Source**: `/home/jetson/mnt/armpi_fpv/armpi_fpv/test/`

**Files to Copy**:
- ✅ `test/arducam_to_usbcam_bridge.py` - **KEY FILE**: Bridge from Arducam CSI to USB camera topics
- ✅ `test/arducam_csi_test.py` - Arducam CSI camera test script
- ✅ `test/hdmi_camera_test.py` - HDMI display camera test
- ✅ `test/hdmi_camera_test.sh` - Shell script for HDMI testing
- ✅ `test/quick_hdmi_test.py` - Quick HDMI test
- ✅ `test/ros2_camera_publisher.py` - ROS2 camera publisher
- ✅ `test/simple_camera_publisher.py` - Simple camera publisher
- ✅ `test/test_picamera.py` - PiCamera test
- ✅ `test/README.md` - Test documentation

**Target Location**: `/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws/test/`

### 1.5 ROS2 Packages

**Source**: `/home/jetson/mnt/armpi_fpv/armpi_fpv/src/`

**Packages to Copy**:
- ✅ `src/lab_config/` - Lab color configuration package (needed for camera calibration - no existing calibration tools in yahboom code)
- ❌ `src/armpi_fpv_utils/` - **NOT NEEDED** (uses hiwonder_servo_msgs, not relevant for Yahboom robot)

**Target Location**: `/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws/src/`

**Note**: 
- `lab_config` is needed because there are no camera calibration tools in the current yahboom codebase
- `armpi_fpv_utils` is not needed since we're migrating from Hiwonder to Yahboom (different servo systems)
- Both systems use ROS2 Humble, so no version migration needed

---

## 2. Path Mappings and Updates Required

### 2.1 Workspace Path Updates

| **Old Path (FPV)** | **New Path (Ultra)** |
|-------------------|---------------------|
| `/home/sammydev295/armpi_fpv` | `/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws` |
| `/home/sammydev295/ros2_iron_ws` | `/opt/ros/humble` or workspace setup |
| Python 3.8 paths | Python 3.10 paths |

### 2.2 Python Version Updates

**Changes Required**:
- Both systems use **Python 3.10** (Ubuntu 22.04 with ROS2 Humble)
- Remove any Python 3.8 detection logic if present
- Update shebangs: `#!/usr/bin/env python3`
- Update PYTHONPATH: `python3.8/site-packages` → `python3.10/site-packages` (if any hardcoded paths exist)

### 2.3 ROS2 Distribution Updates

**Changes Required**:
- ❌ **Old**: `source /home/sammydev295/ros2_iron_ws/install/setup.bash`
- ✅ **New**: `source /opt/ros/humble/setup.bash` and `source install/setup.bash`
- Both systems use ROS2 Humble, so API compatibility should be good
- Update all ROS2 source commands in scripts

---

## 3. Launch Script Migration (`launch_armpi_fpv_tuning.sh`)

### 3.1 Key Changes Needed

**File**: `launch_armpi_fpv_tuning.sh` (migrate from FPV, adapt for Yahboom)

**Critical Updates**:

1. **Workspace Path** (Line 28):
   ```bash
   # OLD:
   cd /home/sammydev295/armpi_fpv
   source /home/sammydev295/ros2_iron_ws/install/setup.bash
   
   # NEW:
   cd /home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws
   source /opt/ros/humble/setup.bash
   source install/setup.bash
   ```

2. **PYTHONPATH** (Line 31):
   ```bash
   # OLD:
   export PYTHONPATH="/home/sammydev295/armpi_fpv/install/hiwonder_servo_controllers/lib/python3.8/site-packages:..."
   
   # NEW:
   export PYTHONPATH="/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws/install/hiwonder_servo_controllers/lib/python3.10/site-packages:..."
   ```

3. **Node Execution Paths**:
   - Update all `ros2 run` commands to use correct package names
   - Verify package names exist in Ultra workspace
   - Update parameter file paths

4. **Camera Integration**:
   - **Astra Camera**: Already working on fixed mount, no changes needed
   - **Arm Camera**: Support current USB camera → Arducam CSI bridge (replacement)
   - Add support for OAK-D-Lite (future option)
   - Update topic remappings for arm camera interface

### 3.2 Dependencies Identified

The launch script references:
- `scripts/jog_keys.py` - Referenced in jog_mode function (if exists)
- `config/` folder - Referenced for home_pose.json and tracking params
- `turn_off_leds.py` - Referenced in stop_tracking function
- `get_ups_status.py` - Referenced in get_system_status function (if exists)

### 3.3 Node Package Mappings

**NOT RELEVANT TO YAHBOOM ROBOT** (Hiwonder-specific nodes):
- ❌ `ros_robot_controller` - Hiwonder robot controller (not used on Yahboom)
- ❌ `hiwonder_servo_controllers` - Hiwonder servo system (Yahboom has its own)
- ❌ `armpi_fpv_kinematics` - Hiwonder kinematics (Yahboom has its own)
- ❌ `camera_ros` - Hiwonder camera node (Yahboom uses different camera packages)
- ❌ `object_tracking` - Hiwonder tracking node (Yahboom has different tracking)

**Yahboom-specific packages to use instead**:
- `yahboomcar_base_node` - Base robot controller
- `yahboomcar_astra` - Astra camera node (fixed mount, already working - no changes needed)
- `yahboomcar_KCFTracker` - Object tracking (if needed)
- `image_tools` - Image viewer (standard ROS2 package)
- `usb_cam` - USB camera node (for arm camera)

**Action**: Adapt menu script to use Yahboom-specific nodes and packages instead of Hiwonder ones.

---

## 4. Camera Integration Strategy

### 4.1 Camera Support Overview

The Rosmaster X3 Plus has **two camera systems**:

1. **Astra Depth Camera** ✅ **ALREADY WORKING - NO MIGRATION NEEDED**
   - **Location**: Fixed mount, elevated position, looks ahead
   - **Package**: `yahboomcar_astra`
   - **Topics**: `/color/image_raw`, `/depth/image_raw`, `/ir/image_raw`
   - **Status**: Already integrated and working perfectly
   - **Action**: **No work needed** - this camera is already functional

2. **Arm Camera (Gripper Camera)** ⚠️ **TO BE REPLACED**
   - **Current**: Inexpensive USB camera mounted on arm/gripper
   - **Device**: Generic USB webcam (accessed via `/dev/camera_usb` or `/dev/video0`)
   - **Status**: Currently working but will be replaced for better quality
   - **Replacement Options**:
     - **Arducam IMX477 CSI Camera** ⚠️ **TO BE INTEGRATED**
       - 12.3MP, 1/2.3", 6mm CS Lens
       - CSI interface
       - Bridge script: `test/arducam_to_usbcam_bridge.py`
       - **Action**: Update bridge to publish to arm camera topic
       - **Purpose**: Replace current USB camera for better quality
     - **OAK-D-Lite HD Camera** ⚠️ **FUTURE INTEGRATION OPTION**
       - OpenCV AI Machine Vision Kit
       - USB 3.0 interface
       - AI acceleration on-device
       - **Action**: Install DepthAI ROS2 driver
       - **Purpose**: Advanced AI vision capabilities on arm

### 4.2 Camera Integration Tasks

#### 4.2.1 Astra Depth Camera (Fixed Mount)

**Status**: ✅ **ALREADY WORKING - NO MIGRATION NEEDED**

**Current Topics**:
- `/color/image_raw` - RGB image
- `/color/camera_info` - RGB camera info
- `/depth/image_raw` - Depth image
- `/depth/camera_info` - Depth camera info
- `/ir/image_raw` - IR image
- `/ir/camera_info` - IR camera info
- `/depth/points` - Point cloud

**Package**: `yahboomcar_astra`
**Launch File**: `yahboomcar_astra/launch/astra.launch.py`

**Action**: **No work needed** - this camera is already integrated and working. It's on a fixed mount and looks ahead, separate from the arm camera system.

#### 4.2.2 Arm Camera - Current USB Camera

**Status**: ⚠️ Currently working but will be replaced

**Current Implementation**:
- Generic USB webcam mounted on arm/gripper
- Accessed via `/dev/camera_usb` or `/dev/video0`
- Used by arm control nodes (e.g., `arm_autopilot`, `arm_mediapipe`)

**Action**: Will be replaced with Arducam or OAK-D-Lite for better quality.

#### 4.2.3 Arducam IMX477 CSI Camera (Arm Camera Replacement)

**Status**: ⚠️ Requires bridge script update

**Current Implementation**:
- Bridge script: `test/arducam_to_usbcam_bridge.py`
- Publishes to `/usb_cam/image_raw` topic
- Uses `libcamera-vid` for capture

**Migration Tasks**:
1. Update `arducam_to_usbcam_bridge.py`:
   - Update topic to match arm camera topic (e.g., `/arm_camera/image_raw` or keep `/usb_cam/image_raw`)
   - Add launch parameter for topic remapping
   - Keep CSI capture using `libcamera-vid`
2. Create ROS2 launch file for Arducam bridge
3. Test CSI camera integration on arm
4. Calibrate camera using `lab_config` package if needed

**Key File**: `test/arducam_to_usbcam_bridge.py`
- **Action**: Update to publish to arm camera topic (replaces current USB camera)

#### 4.2.4 OAK-D-Lite Camera (Future Arm Camera Option)

**Status**: ⚠️ New integration required

**Integration Tasks**:
1. Install DepthAI ROS2 driver:
   ```bash
   cd ~/yahboomcar_ros2_ws_new/yahboomcar_ws/src
   git clone https://github.com/luxonis/depthai-ros.git
   cd ~/yahboomcar_ros2_ws_new/yahboomcar_ws
   colcon build --packages-select depthai_ros
   ```
2. Create launch file for OAK-D-Lite (arm camera)
3. Map topics to arm camera namespace
4. Test AI features on arm

---

## 5. Python Scripts Migration

### 5.1 `scripts/jog_keys.py`

**Status**: ❌ **NOT NEEDED**

**Reason**: Uses `hiwonder_servo_msgs` which is not relevant for Yahboom robot. Yahboom has its own servo message definitions and control system.

**Action**: Skip this file - not applicable to Yahboom Rosmaster X3 Plus.

### 5.2 `scripts/jog_publisher.py`

**Status**: ❌ **NOT NEEDED**

**Reason**: Uses `hiwonder_servo_msgs` which is not relevant for Yahboom robot.

**Action**: Skip this file - not applicable to Yahboom Rosmaster X3 Plus.

### 5.3 `turn_off_leds.py`

**Status**: ⚠️ Needs adaptation for Yahboom strip LEDs

**Current Implementation**: Controls Hiwonder RGB LEDs via `ros_robot_controller.msg.RGBState`

**Yahboom Adaptation**:
- Yahboom uses **strip LEDs** (not Hiwonder RGB LEDs)
- Need to identify Yahboom LED control topic/service
- May need to adapt to Yahboom's LED control messages

**Migration Tasks**:
1. Identify Yahboom LED control topic/service (check `yahboomcar_msgs` or similar)
2. Adapt script to use Yahboom LED control interface
3. Test LED control functionality with strip LEDs
4. Update workspace paths

### 5.4 `get_ups_status.py`

**Status**: ⚠️ **NEEDS COMPLETE REWRITE - Different UPS Module**

**Current Implementation**: 
- Uses Lumi-Electronics UPS monitor system
- Path: `/home/sammydev295/Lumi-Electronics/ups_monitor_system/src`
- Uses `ina219_sensor` from Lumi-Electronics

**Yahboom UPS Module**:
- **Module**: [Waveshare UPS Power Module (C)](https://www.waveshare.com/wiki/UPS_Power_Module_(C))
- **Interface**: I2C
- **Sensor**: INA219 (different implementation)
- **Communication**: Uses `python3-smbus` for I2C communication
- **Demo Code**: Available from Waveshare wiki

**Migration Tasks**:
1. **Rewrite script** to use Waveshare UPS Module C interface:
   ```bash
   sudo apt-get install python3-smbus
   # Download Waveshare demo code
   wget https://files.waveshare.com/wiki/UPS%20Power%20Module%20(C)/UPS_Power_Module_C.zip
   ```
2. Adapt INA219 reading code for Waveshare module
3. I2C address: Typically 0x40 or 0x41 (check Waveshare documentation)
4. Test UPS status reading functionality
5. Update output format if needed

**Reference**: [Waveshare UPS Power Module (C) Wiki](https://www.waveshare.com/wiki/UPS_Power_Module_(C))

---

## 6. Configuration Files Migration

### 6.1 Config Files

**Files**:
- `config/color_ranges.yaml` - Should work as-is
- `config/home_pose.json` - Should work as-is
- `tracking_distant_params.yaml` - Should work as-is
- `tracking_smooth_params.yaml` - Should work as-is

**Migration Tasks**:
1. Copy files to Ultra workspace
2. Update any hardcoded paths in YAML files
3. Verify parameter names match Humble package expectations
4. Test parameter loading in nodes

---

## 7. ROS2 Package Migration

### 7.1 `lab_config` Package

**Status**: ⚠️ **REQUIRES MIGRATION**

**Package Location**: `src/lab_config/`

**Migration Tasks**:
1. Copy package to Ultra workspace
2. Verify ROS2 Humble compatibility:
   - Check all dependencies are available
   - Update Python version (3.8 → 3.10)
   - Review ROS2 API changes (Iron → Humble)
3. Build package:
   ```bash
   cd ~/yahboomcar_ros2_ws_new/yahboomcar_ws
   colcon build --packages-select lab_config
   ```
4. Test lab config manager node
5. Test GUI if applicable

**Files to Review**:
- `src/lab_config/scripts/lab_config_manager_ros2.py`
- `src/lab_config/scripts/lab_config_gui_ros2.py`
- `src/lab_config/package.xml`
- `src/lab_config/CMakeLists.txt`

### 7.2 `armpi_fpv_utils` Package

**Status**: ❌ **NOT NEEDED**

**Reason**: 
- Uses `hiwonder_servo_msgs` which is not relevant for Yahboom robot
- Yahboom has its own servo control system and message definitions
- We're migrating from Hiwonder ArmPi FPV to Yahboom Rosmaster X3 Plus (different hardware)

**Action**: Skip this package - not applicable to Yahboom platform.

---

## 8. Migration Steps

### Phase 1: File Copying and Directory Setup

1. **Create directory structure**:
   ```bash
   cd /home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws
   mkdir -p scripts config test
   ```

2. **Copy root level scripts**:
   ```bash
   cp /home/jetson/mnt/armpi_fpv/armpi_fpv/launch_armpi_fpv_tuning.sh ./
   cp /home/jetson/mnt/armpi_fpv/armpi_fpv/turn_off_leds.py ./
   cp /home/jetson/mnt/armpi_fpv/armpi_fpv/get_ups_status.py ./
   cp /home/jetson/mnt/armpi_fpv/armpi_fpv/test_voltage_monitor.sh ./
   cp /home/jetson/mnt/armpi_fpv/armpi_fpv/tracking_*.yaml ./
   # Note: launch_armpi_fpv.sh is NOT copied (will be deleted, only tuning script is migrated)
   ```

3. **Copy scripts folder** (if any non-Hiwonder scripts exist):
   ```bash
   # Note: jog_keys.py and jog_publisher.py are NOT copied (use hiwonder_servo_msgs)
   # Only copy if there are other utility scripts that don't depend on Hiwonder messages
   ```

4. **Copy config folder**:
   ```bash
   cp -r /home/jetson/mnt/armpi_fpv/armpi_fpv/config/* ./config/
   ```

5. **Copy test folder**:
   ```bash
   cp -r /home/jetson/mnt/armpi_fpv/armpi_fpv/test/* ./test/
   ```

6. **Copy ROS2 packages**:
   ```bash
   cp -r /home/jetson/mnt/armpi_fpv/armpi_fpv/src/lab_config ./src/
   # Note: armpi_fpv_utils is NOT needed (uses hiwonder_servo_msgs)
   ```

### Phase 2: Path and Version Updates

1. **Update `launch_armpi_fpv_tuning.sh`**:
   - Replace all `/home/sammydev295/armpi_fpv` with `/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws`
   - Replace all `/home/sammydev295/ros2_iron_ws` with `/opt/ros/humble`
   - Update Python paths: `python3.8` → `python3.10`
   - Update ROS2 source commands
   - Update node execution paths
   - Update camera integration for Aurora930

2. **Update `turn_off_leds.py`**:
   - Update ROS2 workspace paths
   - Verify message types exist

3. **Update `get_ups_status.py`**:
   - Update UPS monitor system path
   - Or remove if UPS not available

4. **Update Python scripts**:
   - Update shebangs: `#!/usr/bin/env python3`
   - Remove Python 3.8 specific code
   - Update package imports

### Phase 3: Camera Integration

1. **Astra Camera** (Fixed mount):
   - **No work needed** - already working perfectly
   - Verify it's still working: `ros2 launch yahboomcar_astra astra.launch.py`

2. **Arm Camera - Arducam Integration**:
   - Update `test/arducam_to_usbcam_bridge.py`:
     - Update topic to match arm camera (e.g., `/arm_camera/image_raw`)
     - Add launch parameter support
   - Create launch file for Arducam bridge
   - Test CSI camera on arm
   - Calibrate using `lab_config` if needed

3. **Arm Camera - OAK-D-Lite Integration** (Future):
   - Install DepthAI ROS2 driver
   - Create launch configuration for arm
   - Test AI features on arm

### Phase 4: Package Migration

1. **Migrate `lab_config` package**:
   - Update Python version references
   - Review ROS2 API compatibility
   - Build and test

2. **Skip `armpi_fpv_utils` package**:
   - **NOT NEEDED** - uses `hiwonder_servo_msgs` which is not relevant for Yahboom
   - Yahboom has its own servo control system

3. **Verify all dependencies**:
   - Check all required packages exist
   - Copy missing packages if needed

### Phase 5: Testing

1. **Test launch script**:
   - Test each menu function
   - Verify node start/stop
   - Test tracking presets
   - Test jog mode
   - Test lab calibration GUI

2. **Test camera integration**:
   - Test Aurora930 (baseline)
   - Test Arducam bridge
   - Test camera switching

3. **Test Python scripts**:
   - Test `turn_off_leds.py` (Yahboom strip LEDs)
   - Test `get_ups_status.py` (Waveshare UPS Module C)
   - Note: `jog_keys.py` is not needed (Hiwonder-specific)

4. **Integration testing**:
   - End-to-end tracking test
   - Color calibration workflow
   - Servo control integration

---

## 9. Critical Dependencies to Verify

### 9.1 Packages NOT Needed (Hiwonder-Specific)

**These packages are NOT relevant for Yahboom robot**:
- ❌ `hiwonder_servo_msgs` - Hiwonder-specific, not used on Yahboom
- ❌ `hiwonder_servo_controllers` - Hiwonder-specific, not used on Yahboom
- ❌ `hiwonder_servo_driver` - Hiwonder-specific, not used on Yahboom
- ❌ `ros_robot_controller` - Hiwonder-specific, not used on Yahboom
- ❌ `object_tracking` - Hiwonder-specific tracking (Yahboom has different tracking)
- ❌ `armpi_fpv_kinematics` - Hiwonder-specific (Yahboom has its own kinematics)

**Yahboom Equivalent Packages** (already in workspace):
- ✅ `yahboomcar_base_node` - Base robot controller
- ✅ `yahboomcar_astra` - Camera node
- ✅ `yahboomcar_KCFTracker` - Object tracking (if needed)
- ✅ `yahboomcar_msgs` - Yahboom message definitions

**Action**: Adapt menu script to use Yahboom packages instead of Hiwonder ones.

### 9.2 External Dependencies

- **UPS Module**: Waveshare UPS Power Module (C) - **DIFFERENT FROM FPV**
  - **FPV System**: Lumi-Electronics UPS monitor system (I2C, custom implementation)
  - **Yahboom System**: Waveshare UPS Power Module (C) (I2C, INA219 sensor)
  - **Action**: Rewrite `get_ups_status.py` to use Waveshare module interface
  - **Reference**: [Waveshare UPS Power Module (C) Wiki](https://www.waveshare.com/wiki/UPS_Power_Module_(C))
  - **Installation**: `sudo apt-get install python3-smbus`

---

## 10. Testing Checklist

### 10.1 Launch Script Testing

- [ ] Test `setup_ros2_env()` function
- [ ] Test each node start/stop function
- [ ] Test tracking presets (smooth/distant)
- [ ] Test jog mode
- [ ] Test lab calibration GUI
- [ ] Test system status display
- [ ] Test camera switching

### 10.2 Package Testing

- [ ] Build `lab_config` package
- [ ] Test `ros2 run lab_config lab_config_manager_ros2.py`
- [ ] Test lab config GUI (if applicable)
- [ ] Verify all services and topics
- [ ] Note: `armpi_fpv_utils` is NOT needed (Hiwonder-specific)

### 10.3 Camera Testing

- [ ] Verify Astra camera still working (fixed mount - no changes needed)
- [ ] Test current USB arm camera (baseline)
- [ ] Test Arducam CSI camera bridge on arm
- [ ] Test arm camera topic publishing
- [ ] Calibrate arm camera using `lab_config` if needed
- [ ] Test OAK-D-Lite integration (if implemented)

### 10.4 Integration Testing

- [ ] End-to-end tracking test
- [ ] Color calibration workflow
- [ ] Servo control integration
- [ ] Kinematics integration

---

## 11. Known Issues and Solutions

### Issue 1: Python Version Mismatch
**Problem**: Scripts check for Python 3.8, but Humble uses 3.10  
**Solution**: Remove Python 3.8 checks, use system `python3`

### Issue 2: Hardcoded Paths
**Problem**: Many scripts have hardcoded `/home/sammydev295/` paths  
**Solution**: Replace with `/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws` or use environment variables

### Issue 3: Hiwonder vs Yahboom Packages
**Problem**: Menu script references Hiwonder-specific packages that don't exist on Yahboom  
**Solution**: Adapt menu script to use Yahboom packages (`yahboomcar_base_node`, `yahboomcar_astra`, etc.) instead of Hiwonder ones

### Issue 4: Different UPS Modules
**Problem**: FPV uses Lumi-Electronics UPS, Yahboom uses Waveshare UPS Power Module (C)  
**Solution**: Rewrite `get_ups_status.py` to use Waveshare module interface (INA219 via I2C with python3-smbus)

### Issue 5: Camera Topic Mismatch
**Problem**: Different cameras publish to different topics  
**Solution**: Create unified camera abstraction layer with topic remapping

---

## 12. Next Steps

1. **Immediate Actions**:
   - Review this migration plan
   - Identify which packages from FPV repo need to be copied
   - Start Phase 1: File copying

2. **Short-term** (1-2 days):
   - Complete Phase 2: Path and version updates
   - Migrate `lab_config` and `armpi_fpv_utils` packages
   - Test build and basic functionality

3. **Medium-term** (3-5 days):
   - Complete Phase 3: Camera integration
   - Adapt camera bridge for unified interface
   - Test end-to-end workflows

4. **Long-term** (1-2 weeks):
   - Complete integration testing
   - Optimize for Ultra hardware
   - Update documentation

---

## 13. Resources and References

### Source Repository
- **Mounted Location**: `/home/jetson/mnt/armpi_fpv/armpi_fpv`
- **Original Location**: `/home/sammydev295/armpi_fpv` on armpi_fpv machine (Hiwonder ArmPi FPV)
- **Target Location**: `/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws` (Yahboom Rosmaster X3 Plus)
- **ROS2 Distribution**: Both systems use **ROS2 Humble on Ubuntu 22.04** (no version migration needed)

### Camera Documentation
- **Astra Camera**: Fixed mount depth camera - already working via `yahboomcar_astra` package (no migration needed)
- **Arm Camera**: Current inexpensive USB camera on arm/gripper - will be replaced
- **Arducam Product**: https://www.arducam.com/b0240-arducam-imx477-hq-quality-camera.html
- **OAK-D-Lite**: https://www.waveshare.com/oak-d-lite.htm
- **DepthAI Documentation**: https://docs.luxonis.com/
- **DepthAI ROS2**: https://github.com/luxonis/depthai-ros

### UPS Module Documentation
- **Waveshare UPS Power Module (C)**: https://www.waveshare.com/wiki/UPS_Power_Module_(C)
- **Demo Code**: Available from Waveshare wiki
- **I2C Interface**: Uses INA219 sensor via python3-smbus

### Upgrade Plan Reference
- **Document**: `/home/jetson/mnt/armpi_fpv/armpi_fpv/docs/ARMPI_FPV_TO_ULTRA_UPGRADE_PLAN.md`
- Contains detailed camera integration strategy and migration notes

---

**Document Version**: 2.0  
**Last Updated**: January 2025  
**Status**: Updated for Yahboom Rosmaster X3 Plus Migration

**Key Changes from v1.0**:
- Updated for migration from Hiwonder ArmPi FPV to Yahboom Rosmaster X3 Plus
- Removed Hiwonder-specific packages (servo_msgs, servo_controllers, etc.)
- Removed Aurora930 references (not needed - Astra camera is different and already working)
- Updated camera information:
  - Astra camera: Fixed mount, already working, no migration needed
  - Arm camera: Current inexpensive USB camera, will be replaced with Arducam/OAK-D-Lite
- Updated UPS module information (Waveshare UPS Power Module C instead of Lumi-Electronics)
- Clarified that both systems use ROS2 Humble (no version migration needed)
- Identified that lab_config is needed for camera calibration (no existing tools in yahboom code)
- Clarified that `launch_armpi_fpv.sh` will be deleted, only `launch_armpi_fpv_tuning.sh` is migrated
- Removed `armpi_fpv_utils` package (not needed - uses Hiwonder messages)

