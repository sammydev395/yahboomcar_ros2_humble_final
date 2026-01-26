# yahboomcar_linefollw/launch Launch File Catalog

---

## yahboomcar_linefollw/launch/follow_line.launch
**Purpose:** Line following demo with optional camera and remote control.

**Starts:**
- Joystick/remote control (via yahboomcar_ctrl/yahboom_joy.launch)
- Low-level driver node (via yahboomcar_bringup/yahboomcar.launch)
- USB camera node (via usb_cam-test.launch, unless VideoSwitch is true)
- Line following node (`follow_line.py`, if VideoSwitch is true)

**Use:** For line following demos using the camera and remote control.

---

## yahboomcar_linefollw/launch/line.launch
**Purpose:** Basic line following demo with camera input.

**Starts:**
- Line following node (`follow_line.py`)

**Use:** For simple line following using the camera. 