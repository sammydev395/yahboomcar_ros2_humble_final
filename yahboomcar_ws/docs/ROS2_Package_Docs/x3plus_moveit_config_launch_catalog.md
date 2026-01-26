# x3plus_moveit_config/launch & arm_moveit_demo/launch Launch File Catalog

---

## x3plus_moveit_config/launch/demo.launch
**Purpose:** Main MoveIt demo for the X3Plus arm, with optional RViz and fake or real controllers.

**Starts:**
- Joint state publisher (GUI or non-GUI)
- Robot state publisher
- MoveIt move_group node (via move_group.launch)
- RViz (optional)
- MongoDB warehouse (optional)

**Use:** For MoveIt simulation, planning, and visualization with the X3Plus arm.

---

## x3plus_moveit_config/launch/demo_gazebo.launch
**Purpose:** MoveIt demo integrated with Gazebo simulation.

**Starts:**
- Gazebo simulation (via gazebo.launch)
- MoveIt demo (via demo.launch, with Gazebo controllers)

**Use:** For simulating the X3Plus arm in Gazebo with MoveIt.

---

## x3plus_moveit_config/launch/joystick_control.launch
**Purpose:** Joystick control for MoveIt and the X3Plus arm.

**Starts:**
- Joystick node (`joy_node`)
- MoveIt joystick interface (`moveit_joy.py`)

**Use:** For controlling the arm with a joystick in MoveIt.

---

## arm_moveit_demo/launch/x3plus_moveit_demo.launch
**Purpose:** Comprehensive MoveIt demo for the X3Plus arm, with simulation, RViz, and optional real robot interface.

**Starts:**
- Joint state publisher (GUI or non-GUI, if sim enabled)
- Robot state publisher (if sim enabled)
- Real robot interface node (`SimulationToMachine.py`, if sim disabled)
- MoveIt move_group node (via x3plus_moveit_config/move_group.launch)
- RViz (optional)
- MongoDB warehouse (optional)

**Use:** For full-featured MoveIt simulation, planning, and visualization with the X3Plus arm, or for interfacing with the real robot. 