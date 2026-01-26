# yahboomcar_ctrl/launch Launch File Catalog

---

## Joystick Control Reference

### Supported Joystick Types
- **Jetson Controller**: 15 buttons, 8 axes (Vendor=0079 Product=181c)
- **PC Controller**: 11 buttons, 8 axes (different mapping)

### Jetson Controller Mapping

#### Axes (8 total)
| Axis | Name | Description | Function |
|------|------|-------------|----------|
| 0 | X | Left stick horizontal | Robot Y-axis movement (left/right) |
| 1 | Y | Left stick vertical | Robot X-axis movement (forward/backward) |
| 2 | Z | Right stick horizontal | Robot angular rotation (left/right) |
| 3 | Rz | Right stick vertical | Robot Y-axis movement (alternative) |
| 4 | Gas | Right trigger | **Safety override** - Toggles `/JoyState` for KCF tracker and other systems |
| 5 | Brake | Left trigger | Arm joint control (Joint 5 or 6 depending on gripper mode) |
| 6 | Hat0X | Left D-pad horizontal | Arm joint 3 control |
| 7 | Hat0Y | Left D-pad vertical | Arm joint 4 control |

#### Buttons (15 total)
| Button | Name | Function |
|--------|------|----------|
| 0 | A | Arm joint 2 decrease |
| 1 | B | Arm joint 1 increase |
| 2 | C | (Unused) |
| 3 | X | Arm joint 1 decrease |
| 4 | Y | Arm joint 2 increase |
| 5 | Z | (Unused) |
| 6 | L1 | Arm joint 5/6 increase (depending on gripper mode) |
| 7 | R1 | RGB light cycle |
| 8 | TL | (Unused) |
| 9 | TR | (Unused) |
| 10 | Select | Toggle gripper mode |
| 11 | Start | **Safety override** - Must be held for robot movement |
| 12 | Mode | (Unused) |
| 13 | Left Thumb | Linear speed gear control (1.0 → 1/3 → 2/3 → 1.0) |
| 14 | Right Thumb | Angular speed gear control (1.0 → 1/4 → 1/2 → 3/4 → 1.0) |

### PC Controller Mapping

#### Axes (8 total)
| Axis | Name | Description | Function |
|------|------|-------------|----------|
| 0 | X | Left stick horizontal | Robot Y-axis movement (left/right) |
| 1 | Y | Left stick vertical | Robot X-axis movement (forward/backward) |
| 2 | L2 | Left trigger | Arm joint control (Joint 5 or 6) |
| 3 | Z | Right stick horizontal | Robot angular rotation |
| 4 | Rz | Right stick vertical | (Unused) |
| 5 | R2 | Right trigger | **Safety override** - Toggles `/JoyState` |
| 6 | Hat0X | Left D-pad horizontal | Arm joint 3 control |
| 7 | Hat0Y | Left D-pad vertical | Arm joint 4 control |

#### Buttons (11 total)
| Button | Name | Function |
|--------|------|----------|
| 0 | A | Arm joint 2 decrease |
| 1 | B | Arm joint 1 increase |
| 2 | X | Arm joint 1 decrease |
| 3 | Y | Arm joint 2 increase |
| 4 | L1 | Arm joint 5/6 increase |
| 5 | R1 | RGB light cycle |
| 6 | Select | Toggle gripper mode |
| 7 | Mode | Buzzer toggle |
| 8 | Start | (Unused) |
| 9 | Left Thumb | Linear speed gear control |
| 10 | Right Thumb | Angular speed gear control |

### Published Topics

#### `/cmd_vel` (geometry_msgs/Twist)
- **Linear.x**: Forward/backward movement
- **Linear.y**: Left/right movement  
- **Angular.z**: Rotation (left/right)

#### `/JoyState` (std_msgs/Bool)
- **True**: Enable movement for KCF tracker and other systems
- **False**: Disable movement (safety stop)
- **Triggered by**: Gas trigger (Axis 4) on Jetson, Brake trigger (Axis 5) on PC

#### `/TargetAngle` (yahboomcar_msgs/ArmJoint)
- **id**: Joint number (1-6)
- **angle**: Target angle in degrees
- **run_time**: Movement duration

#### `/RGBLight` (std_msgs/Int32)
- Cycles through 6 different RGB light patterns
- **Triggered by**: R1 button

#### `/Buzzer` (std_msgs/Bool)
- **True**: Turn on buzzer
- **False**: Turn off buzzer
- **Triggered by**: Start button (Jetson) or Mode button (PC)

### Safety Features

1. **Dual Safety Override**:
   - Gas trigger (Axis 4) on Jetson controller
   - Brake trigger (Axis 5) on PC controller
   - Both toggle `/JoyState` topic

2. **Movement Requirements**:
   - Start button (11) must be held for robot movement on Jetson
   - `/JoyState` must be `true` for KCF tracker movement

3. **Automatic Stops**:
   - Arm movement stops when no buttons/axes are pressed
   - Robot movement stops when safety override is released

### Usage Examples

#### Basic Robot Control
1. Hold **Start button** (11) for movement enable
2. Use **left stick** (Axes 0,1) for forward/backward/left/right
3. Use **right stick** (Axis 2) for rotation

#### Arm Control
1. Use **A/B/X/Y buttons** for joints 1-2
2. Use **D-pad** (Axes 6,7) for joints 3-4
3. Use **L1 + L2** for joints 5-6
4. Press **Select** to toggle gripper mode

#### KCF Tracker Control
1. Draw bounding box with mouse in KCF window
2. Press **Gas trigger** (Axis 4) to enable movement
3. Robot will automatically track selected object

---

## yahboomcar_ctrl/launch/yahboom_joy.launch
**Purpose:** Main joystick/remote control for the robot base.

**Starts:**
- Joystick node (`joy_node`)
- Yahboom joystick control node (`yahboom_joy.py`)

**Use:** For controlling the robot base with a joystick or remote.

---

## yahboomcar_ctrl/launch/yahboom_keyboard.launch
**Purpose:** Keyboard control for the robot base.

**Starts:**
- Yahboom keyboard control node (`yahboom_keyboard.py`)

**Use:** For controlling the robot base with a keyboard.

---

## yahboomcar_ctrl/launch/twist_joy.launch
**Purpose:** Joystick control for turtlesim and twist-based robots.

**Starts:**
- Turtlesim node
- Joystick node (`joy_node`)
- Twist joystick control node (`twist_joy.py`)

**Use:** For controlling turtlesim or twist-based robots with a joystick.

---

## yahboomcar_ctrl/launch/turtlebot_joy.launch
**Purpose:** Joystick control for Turtlebot robots.

**Starts:**
- Joystick node (`joy_node`)
- Turtlebot joystick control node (`turtlebot_joy.py`)

**Use:** For controlling Turtlebot robots with a joystick. 