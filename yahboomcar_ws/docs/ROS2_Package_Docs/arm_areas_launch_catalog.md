# Arm-Related Areas Launch File Catalog

---

## arm_autopilot/launch/arm_autopilot.launch
**Purpose:** Arm autopilot demo with camera, remote control, and web video streaming.

**Starts:**
- Joystick/remote control (via yahboomcar_ctrl/yahboom_joy.launch)
- Low-level driver node (via yahboomcar_bringup/yahboomcar.launch)
- Web video server
- Arm autopilot node (`autopilot_main.py`)

**Use:** For autonomous arm control demos with camera and remote control.

---

## arm_color_transport/launch/transport_base.launch
**Purpose:** Arm color transport demo with navigation, mapping, and web video streaming.

**Starts:**
- Map server
- Lidar and robot base bringup (via yahboomcar_nav/laser_bringup.launch)
- AMCL localization
- Move base for navigation
- Web video server
- Marker drawing node (`DrawMarker.py`)
- Color transport node (`transport_main.py`)

**Use:** For color-based object transport demos with navigation and visualization.

---

## arm_color_transport/launch/transport_rviz.launch
**Purpose:** RViz visualization for arm color transport.

**Starts:**
- RViz with arm color transport config

**Use:** For visualizing arm color transport in RViz.

---

## arm_mediapipe/launch/01_HandCtrlArm.launch
**Purpose:** Hand gesture control for the arm using MediaPipe.

**Starts:**
- Hand control node (`01_HandCtrlArm.py`)

**Use:** For controlling the arm with hand gestures.

---

## arm_mediapipe/launch/02_PoseCtrlArm.launch
**Purpose:** Pose-based control for the arm using MediaPipe.

**Starts:**
- Pose control node (`02_PoseCtrlArm.py`)

**Use:** For controlling the arm with body pose estimation.

---

## arm_mediapipe/launch/mediaArm.launch
**Purpose:** MediaPipe-based arm demo with remote control and web video streaming.

**Starts:**
- Joystick/remote control (via yahboomcar_ctrl/yahboom_joy.launch)
- Low-level driver node (via yahboomcar_bringup/yahboomcar.launch)
- Web video server
- Message-to-image node (`msgToimg.py`)

**Use:** For MediaPipe-based arm demos with visualization and remote control.

# Yahboomcar Controller Arm Movement Guide

## How It Should Work Now
When you press arm buttons, you should see:
- **Button Detection:** `Arm buttons pressed: [X, B, A, Y, etc.]`
- **Command Sending:** `ARM COMMAND SENT - Joint X: target=Y -> sent=Z`

## Test the Arm Movement
Try pressing these buttons and watch the logs:
- **Joint 1 (Base):** X button (decrease), B button (increase)
- **Joint 2 (Shoulder):** A button (decrease), Y button (increase)
- **Joint 3 (Elbow):** Left D-pad left/right
- **Joint 4 (Wrist):** Left D-pad up/down
- **Joint 5 (Wrist rotation):** L2 (decrease), L1 (increase) - when gripper mode is OFF
- **Joint 6 (Gripper):** L2 (close), L1 (open) - when gripper mode is ON

---

## Button Mapping Reference

### Face Buttons (Right Side)
- **X (bottom, blue)** → `buttons[0]` (A in code)
- **B (right, red)**   → `buttons[1]` (B in code)
- **A (left, green)**  → `buttons[2]` (X in code)
- **Y (top, pink)**    → `buttons[3]` (Y in code)

### Shoulder Buttons (Top Edge)
- **L1 (top left, front row)**  → `buttons[4]`
- **R1 (top right, front row)** → `buttons[5]`
- **L2 (bottom left, back row, trigger)**  → `axes[2]` (usually, value -1 when pressed)
- **R2 (bottom right, back row, trigger)** → `axes[5]` (usually, value -1 when pressed)

### Middle Buttons
- **SELECT (left)** → `buttons[6]`
- **MODE (center)** → `buttons[7]`
- **START (right)** → `buttons[8]`

### Sticks (Press Down)
- **Left Stick Press**  → `buttons[9]`
- **Right Stick Press** → `buttons[10]`

### D-Pad
- **D-Pad Left/Right** → `axes[6]`
- **D-Pad Up/Down**    → `axes[7]`

---

## Special Notes for Your Controller
- The **L1** and **R1** are the topmost shoulder buttons (not triggers).
- The **L2** and **R2** are the lower, trigger-style buttons.
- The **MODE** button is not used for gripper/wrist switching in your code.
- **Gripper/wrist mode** is toggled by the **SELECT** button (`buttons[6]`).

---

## In Your Code (`user_pc` function):
- **L1** (`buttons[4]`): Used for wrist or gripper open (depending on mode)
- **L2** (`axes[2] == -1`): Used for wrist or gripper close (depending on mode)
- **SELECT** (`buttons[6]`): Toggles between gripper and wrist rotation mode

---

## Summary Table
| Physical Button | Code Name | Index/Axis | Function (in your code)         |
|-----------------|-----------|------------|---------------------------------|
| X (pink)        | A         | buttons[0] | Arm joint 1 anti-clockwise      |
| B (red)         | B         | buttons[1] | Arm joint 1 clockwise           |
| A (blue)        | X         | buttons[2] | Arm joint 2 down                |
| Y (green)       | Y         | buttons[3] | Arm joint 2 up                  |
| L1              | L1        | buttons[4] | Wrist/Gripper open              |
| R1              | R1        | buttons[5] | RGB light effect                |
| L2 (trigger)    | L2        | axes[2]    | Wrist/Gripper close             |
| R2 (trigger)    | R2        | axes[5]    | Cancel navigation               |
| SELECT          | SELECT    | buttons[6] | Toggle gripper/wrist mode       |
| MODE            | MODE      | buttons[7] | Not used in your code           |
| START           | START     | buttons[8] | This is the safety              |

---

## To Switch Gripper/Wrist Mode
- Press the **SELECT** button (left of MODE) to toggle between controlling the gripper and wrist rotation with L1/L2.

If you want to use the MODE button for something, let us know!
Otherwise, use SELECT for gripper/wrist mode switching as per your current code.
If you want a custom mapping or want to use the MODE button for mode switching, we can help you change the code accordingly!

---

## Troubleshooting: No Arm Movement

### What's Working
- Joystick node starts and synchronizes arm state (you see the correct angles in the logs).
- No errors or crashes in the logs.
- No arm movement occurs when you press arm buttons.

### What's Missing
- No log lines like:
  - `ARM COMMAND SENT - Joint X: target=Y -> sent=Z`
  - `Arm buttons pressed: [...]`
  appear in your logs.

### What This Means
- The joystick node is not detecting your button presses for arm movement, or the code is not reaching the arm command publishing section.

### What to Check and Try
1. **Are Arm Button Presses Detected?**
   - When you press any of the arm control buttons (X, B, A, Y, D-pad, L1, L2), do you see log lines like:
     - `Arm buttons pressed: ['X']`
   - If you do **not** see these, the joystick mapping may not match your controller.
2. **Check Joystick Button/Axes Mapping**
   - Run the following command in a new terminal to see your controller's live mapping:
     ```bash
     ros2 topic echo /joy
     ```
   - Press each button and trigger, and note which `buttons[]` or `axes[]` index changes.
   - Compare this to the mapping in your code (see above for mapping).
3. **Test for Any Button Press**
   - Try pressing **all** face buttons, D-pad, L1/L2, and see if **any** "Arm buttons pressed" log appears.
   - If not, your controller mapping is different from the code's assumptions.
4. **If No Logs Appear for Arm Buttons**
   - Your controller may use a different mapping (common for generic PS2/PS3/USB controllers).
   - You may need to adjust the `user_pc` function to match your actual mapping.

---

## How to Fix

### A. Print All Button and Axis States
Add a log to print all button and axis states on every joystick message. This will help you map your controller:
```python
self.get_logger().info(f"RAW BUTTONS: {joy_data.buttons}")
self.get_logger().info(f"RAW AXES: {joy_data.axes}")
```
Add these lines to your `user_pc` function.

### B. Remap According to Your Controller
After you see which index changes for each button, update the code so that, for example, if your "X" button is `buttons[1]` instead of `buttons[0]`, change the code accordingly.

---

## Debugging Table
| Physical Button | What to Watch For in /joy | Code Index (default) | What to Change If Different |
|-----------------|--------------------------|----------------------|-----------------------------|
| X (blue)        | buttons[0]               | buttons[0]           | Change to match your index  |
| B (red)         | buttons[1]               | buttons[1]           | ...                         |
| A (green)       | buttons[2]               | buttons[2]           | ...                         |
| Y (pink)        | buttons[3]               | buttons[3]           | ...                         |
| L1              | buttons[4]               | buttons[4]           | ...                         |
| L2 (trigger)    | axes[2]                  | axes[2]              | ...                         |

---

## Next Steps
1. Add the raw print statements to your `user_pc` function.
2. Restart the joystick node and press each button/trigger.
3. Note which index changes for each button.
4. Update the code to match your controller's mapping.

Let us know the output of the raw button/axes logs when you press your controller buttons, and we'll help you remap the code for your controller! 