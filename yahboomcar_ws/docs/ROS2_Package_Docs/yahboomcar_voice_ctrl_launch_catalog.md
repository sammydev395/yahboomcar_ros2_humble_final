# yahboomcar_voice_ctrl/launch Launch File Catalog

---

## yahboomcar_voice_ctrl/launch/voice_transport_base.launch
**Purpose:** Voice-controlled transport demo with navigation, mapping, and web video streaming.

**Starts:**
- Map server
- Lidar and robot base bringup (via yahboomcar_nav/laser_bringup.launch)
- AMCL localization
- Move base for navigation
- Web video server
- ROSBridge server
- Marker drawing node (`DrawMarker.py`)

**Use:** For voice-controlled object transport demos with navigation and visualization.

---

## yahboomcar_voice_ctrl/launch/voice_ctrl_arm.launch
**Purpose:** Voice control for the arm.

**Starts:**
- Voice control node for the arm (`voice_ctrl_arm.py`)
- Joystick/remote control (via yahboomcar_ctrl/yahboom_joy.launch)

**Use:** For controlling the arm with voice commands.

---

## yahboomcar_voice_ctrl/launch/voice_ctrl_arm_study.launch
**Purpose:** Voice control for arm study/demo.

**Starts:**
- Voice control study node for the arm (`voice_arm_study.py`)
- Joystick/remote control (via yahboomcar_ctrl/yahboom_joy.launch)

**Use:** For studying or demonstrating voice control of the arm.

---

## yahboomcar_voice_ctrl/launch/voice_ctrl_colorTracker.launch
**Purpose:** Voice control for color tracking with camera and remote control.

**Starts:**
- Joystick/remote control (via yahboomcar_ctrl/yahboom_joy.launch)
- Astra camera node (via orbbec_camera/astraproplus.launch)
- Low-level driver node (via yahboomcar_bringup/yahboomcar.launch)
- USB camera node (via usb_cam-test.launch, unless VideoSwitch is true)
- Color tracking node (`colorTracker.py`)

**Use:** For color tracking demos with voice and remote control.

---

## yahboomcar_voice_ctrl/launch/voice_ctrl_followline.launch
**Purpose:** Voice control for line following with lidar and remote control.

**Starts:**
- Low-level driver node (via yahboomcar_bringup/yahboomcar.launch)
- Joystick/remote control (via yahboomcar_ctrl/yahboom_joy.launch)
- Lidar node (via ydlidar_ros_driver/TG.launch)

**Use:** For line following demos with voice and remote control.

---

## yahboomcar_voice_ctrl/launch/voice_ctrl_takethings.launch
**Purpose:** Voice-controlled object transport demo with navigation, mapping, and web video streaming.

**Starts:**
- Map server
- Lidar and robot base bringup (via yahboomcar_nav/laser_bringup.launch)
- AMCL localization
- Move base for navigation
- Web video server
- Marker drawing node (`DrawMarker.py`)

**Use:** For voice-controlled object transport demos with navigation and visualization.

---

## yahboomcar_voice_ctrl/launch/voice_ctrl_yahboomcar.launch
**Purpose:** Voice control for the robot base.

**Starts:**
- Voice control node for the base (`voice_Ctrl_Mcnamu_driver.py`)
- Joystick/remote control (via yahboomcar_ctrl/yahboom_joy.launch)

**Use:** For controlling the robot base with voice commands. 