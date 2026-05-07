"""D8.2 / Phase 5 combined chassis + arm live launch — LIVE MODE.

First time the X3PLUS chassis (mecanum_drive_controller) and arm
(forward_command_controller) run simultaneously under our ros2_control
stack with shared serial bus to the STM32. Mirrors the design of
phase4_single_joint.launch.py, adding:

  - chassis_controller (mecanum_drive_controller/MecanumDriveController)
  - teleop_twist_joy_node (joy → /chassis_controller/reference TwistStamped)

Brings up:
  - robot_state_publisher (URDF expanded with dry_run:=false)
  - controller_manager with YahboomSystem in LIVE mode
  - joint_state_broadcaster
  - arm_controller (forward_command_controller, position interface)
  - chassis_controller (mecanum_drive_controller)
  - joy_node — gamepad → /joy
  - arm_teleop_node — /joy + /joint_states → /arm_controller/commands
    (active_joint=all by default for combined ops, jog rate from launch arg)
  - teleop_twist_joy_node — /joy → /chassis_controller/reference

Deadman convention (cross-subsystem, one stick layout):
  - B (button 1) = ARM deadman. Hold to allow stick → arm motion.
    + X (button 3) = arm turbo (TURBO_FACTOR=2.0)
  - A (button 0) = CHASSIS deadman. Hold to allow stick → chassis Twist.
    + Y (button 4) = chassis turbo (linear/angular separate scale_turbo).
  - SELECT (button 10) = E-stop / freeze. arm_teleop calls ~/freeze
    (snaps arm target to current state, publishes zero TwistStamped to
    /chassis_controller/reference to also stop the wheels).

Sticks are physically shared:
  - LS LR (axis 0) drives arm_joint1 OR chassis linear.y, depending
    on which deadman is held.
  - LS UD (axis 1) drives arm_joint2 OR chassis linear.x.
  - RS LR (axis 2) drives arm_joint5 OR chassis angular.yaw.
  - RS UD (axis 3) drives arm_joint3 (no chassis equivalent).
  - D-pad LR (axis 6) drives arm_joint4.
  - D-pad UD (axis 7) drives grip_joint.
Hold both deadmen at once = drive both subsystems simultaneously.

Why arm_teleop_node is REQUIRED here:
  same reason as phase4_single_joint — forward_command_controller's
  command interfaces drift toward default-zero without a continuous
  publisher. arm_teleop's 50 Hz idle-republish keeps them at the
  configure-time seed when no operator input is present.

Pre-flight (do BEFORE running this launch):
  1. Verify all six arm joints inside URDF range:
       ros2 run yahboom_ros2_control smoke_serial /dev/myserial --query-arm
     If ANY joint reads OUTSIDE [kArmUrdfLo, kArmUrdfHi]: disable torque
     ('smoke_serial /dev/myserial --torque-off'), back-drive into range,
     re-query, then proceed.
  2. **WHEELS**: lift the chassis off the ground OR clear the floor for
     ~2 m around. At default conservative caps the chassis moves
     ~0.10 m/s; at turbo it can hit 0.30 m/s. With angular turbo it can
     spin at 1.0 rad/s — clear obstacles.
  3. Confirm vendor stack is NOT running (no rosmaster_main.py, no
     Mcnamu_driver_X3plus). lsof /dev/myserial must return nothing.
  4. Operator hand near power kill switch + SELECT button.

Operator workflow (after launch comes up):
  Step 1. Verify activation logs:
      "arm seeded: cmd = state from hardware (first write == no-op)"
      "arm torque ENABLED — first write() will seed servos via
       6× FUNC_UART_SERVO at configure pose"
      "joint_state_broadcaster ... activated"
      "arm_controller ... activated"
      "chassis_controller ... activated"
      "arm_teleop ... seeded target from /joint_states: [...]"
    NO movement should occur yet (no deadman held).

  Step 2. **Chassis only** test (don't touch B):
      Hold A + LS UD (forward/back) — wheels should spin slowly.
      Hold A + LS LR (strafe) — mecanum strafe.
      Hold A + RS LR (yaw) — chassis spins in place.
      Release A — wheels stop within 1 control cycle (FUNC_MOTION
      heartbeat dedupe sends a zero-Twist on cmd_vel timeout).

  Step 3. **Arm only** test (don't touch A):
      Hold B + arm sticks per JOINT_MAP — same as D7.5 / D8.1 PASS.
      Confirm arm still works as it did at 0.40 rad/s.

  Step 4. **Combined** — hold A + B simultaneously, drive both.
    PASS bar: no servo distress, no [FAULT] arm_teleop log lines, no
    chassis stutter, no DDS lag visible in the controller logs.

  Step 5. **Turbo combined** (caps depend on launch args):
      A + Y → chassis at scale_*_turbo (default 0.30 m/s linear,
              1.0 rad/s angular).
      B + X → arm at arm_jog_rate × TURBO_FACTOR (default 0.40 × 2 =
              0.80 rad/s = PHYSICAL_MAX_RATE_DEFAULT).
      A + B + Y + X → both at full physical caps.

  Step 6. **E-stop drill** while both subsystems are active:
      With at least one deadman held, press SELECT. Confirm:
        - arm_teleop logs "[E-STOP] gamepad button 10 pressed"
        - arm freezes at current /joint_states
        - chassis stops within 1 cycle (zero TwistStamped on
          /chassis_controller/reference)
        - Both subsystems stay stopped until SELECT is released and
          the next deadman press.

Abort criteria (any of these = STOP, release deadman, hit SELECT):
  - Self-collision risk during simultaneous shoulder + elbow + chassis
    motion (the arm extends differently as the chassis turns, possibly
    altering self-collision geometry).
  - Servo distress (whine/click/shudder) — most likely under load when
    the chassis pulls/pushes against arm inertia at turbo.
  - [FAULT] line in arm_teleop log (discontinuity reject).
  - Wheel slip / motor stall (chassis caps too high; lower
    linear_scale).
  - Controller_manager update_rate dropping below 100 Hz under
    combined load.

Recovery from any abort:
  Ctrl+C the launch (on_deactivate fires, leaves arm torque ON).
  ros2 run yahboom_ros2_control smoke_serial /dev/myserial --torque-off
  to back-drive arm by hand if needed; --query-arm to verify before
  re-launch.

Default caps: arm_jog_rate=0.40 (D8.1 50% — proven at turbo 100%),
chassis linear=0.10/0.30 m/s, chassis angular=0.30/1.0 rad/s. Override
via launch args to scale up after each PASS (Phase 5 25 → 50 → 100%).
"""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, LogInfo, RegisterEventHandler
from launch.event_handlers import OnProcessExit
from launch.substitutions import (
    Command, LaunchConfiguration, PathJoinSubstitution,
)
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    pkg_share = FindPackageShare('yahboom_ros2_control')

    urdf_path = PathJoinSubstitution(
        [pkg_share, 'description', 'x3plus_chassis.urdf.xacro'])
    controllers_yaml = PathJoinSubstitution(
        [pkg_share, 'config', 'ros2_controllers.yaml'])

    urdf_arg = DeclareLaunchArgument(
        'urdf', default_value=urdf_path,
        description='Path to URDF/xacro for the X3PLUS.')
    controllers_arg = DeclareLaunchArgument(
        'controllers', default_value=controllers_yaml,
        description='Path to ros2_controllers.yaml.')
    arm_jog_rate_arg = DeclareLaunchArgument(
        'arm_jog_rate', default_value='0.40',
        description=('Arm jog rate cap (rad/s) for arm_teleop. D8.1 '
                     'validated 0.40 rad/s × B+X turbo = 0.80 rad/s = '
                     'PHYSICAL_MAX_RATE_DEFAULT. Set to 0.10 for the '
                     'Phase-4 conservative cap; 0.0 to use JOINT_MAP '
                     'per-joint rates.'))
    linear_scale_arg = DeclareLaunchArgument(
        'linear_scale', default_value='0.10',
        description=('Chassis linear cap (m/s) for teleop_twist_joy. '
                     'Phase 5 25% = 0.10. Vendor full-gear is 0.7.'))
    linear_turbo_scale_arg = DeclareLaunchArgument(
        'linear_turbo_scale', default_value='0.30',
        description=('Chassis linear cap (m/s) when Y (turbo) is held.'))
    angular_scale_arg = DeclareLaunchArgument(
        'angular_scale', default_value='0.30',
        description=('Chassis angular cap (rad/s) for teleop_twist_joy. '
                     'Phase 5 25% = 0.30. Vendor full-gear is 3.2.'))
    angular_turbo_scale_arg = DeclareLaunchArgument(
        'angular_turbo_scale', default_value='1.00',
        description=('Chassis angular cap (rad/s) when Y (turbo) is held.'))
    active_joint_arg = DeclareLaunchArgument(
        'active_joint', default_value='all',
        description=('arm_teleop active_joint. Default "all" — every '
                     'joint live per JOINT_MAP. Use a specific joint '
                     'name (e.g. arm_joint2) to lock the arm to a '
                     'single-joint while the chassis remains free.'))
    device_id_arg = DeclareLaunchArgument(
        'device_id', default_value='0',
        description=('Joystick index. 0 = /dev/input/js0 '
                     '(= /dev/yahboom_joy DragonRise 0079:181c).'))

    # LIVE mode — dry_run BAKED IN to false.
    robot_description_content = ParameterValue(
        Command(['xacro ', LaunchConfiguration('urdf'), ' dry_run:=false']),
        value_type=str,
    )

    pre_flight_banner = LogInfo(
        msg=[
            '\n',
            '═════════════════════════════════════════════════════════════\n',
            '  PHASE 5 COMBINED LIVE TEST — LIVE MODE (dry_run=false)\n',
            '─────────────────────────────────────────────────────────────\n',
            '  active_joint        = ', LaunchConfiguration('active_joint'), '\n',
            '  arm_jog_rate        = ', LaunchConfiguration('arm_jog_rate'), ' rad/s (×2 with X turbo)\n',
            '  chassis linear      = ', LaunchConfiguration('linear_scale'), ' m/s '
            '(turbo: ', LaunchConfiguration('linear_turbo_scale'), ')\n',
            '  chassis angular     = ', LaunchConfiguration('angular_scale'), ' rad/s '
            '(turbo: ', LaunchConfiguration('angular_turbo_scale'), ')\n',
            '─────────────────────────────────────────────────────────────\n',
            '  ARM:     B (button 1) = deadman | X (3) = turbo\n',
            '  CHASSIS: A (button 0) = deadman | Y (4) = turbo\n',
            '  E-STOP:  SELECT (button 10) — freezes BOTH arm + chassis\n',
            '─────────────────────────────────────────────────────────────\n',
            '  ⚠  Wheels are LIVE. Lift chassis or clear 2m floor.\n',
            '  ABORT on servo distress, [FAULT], wheel slip, or update_rate\n',
            '  drop below 100 Hz under combined load.\n',
            '═════════════════════════════════════════════════════════════\n',
        ],
    )

    robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='screen',
        parameters=[{'robot_description': robot_description_content}],
    )

    controller_manager = Node(
        package='controller_manager',
        executable='ros2_control_node',
        output='screen',
        parameters=[
            {'robot_description': robot_description_content},
            LaunchConfiguration('controllers'),
        ],
    )

    spawn_jsb = Node(
        package='controller_manager',
        executable='spawner',
        arguments=['joint_state_broadcaster',
                   '--controller-manager', '/controller_manager'],
    )

    spawn_arm = Node(
        package='controller_manager',
        executable='spawner',
        arguments=['arm_controller',
                   '--controller-manager', '/controller_manager'],
    )

    spawn_chassis = Node(
        package='controller_manager',
        executable='spawner',
        arguments=['chassis_controller',
                   '--controller-manager', '/controller_manager'],
    )

    # Spawn arm + chassis controllers AFTER JSB has activated, so
    # /joint_states is flowing by the time controllers come up.
    delay_after_jsb = RegisterEventHandler(
        event_handler=OnProcessExit(
            target_action=spawn_jsb,
            on_exit=[spawn_arm, spawn_chassis],
        )
    )

    joy_node = Node(
        package='joy',
        executable='joy_node',
        name='joy_node',
        output='screen',
        parameters=[{
            'device_id': LaunchConfiguration('device_id'),
            'deadzone': 0.0,           # arm_teleop + teleop_twist_joy
                                       # apply their own deadzones
            'autorepeat_rate': 50.0,
            'sticky_buttons': False,
            'coalesce_interval_ms': 1,
        }],
    )

    arm_teleop = Node(
        package='yahboom_ros2_control',
        executable='arm_teleop_node.py',
        name='arm_teleop',
        output='screen',
        parameters=[{
            'active_joint': LaunchConfiguration('active_joint'),
            'phase4_jog_rate': LaunchConfiguration('arm_jog_rate'),
            'dry_run': False,
        }],
    )

    # teleop_twist_joy — gamepad → TwistStamped on /chassis_controller/reference.
    # Axis layout per yahboom_gamepad_map.yaml chassis_with_a_held:
    #   linear.x   ← LS UD (axis 1) — forward/back
    #   linear.y   ← LS LR (axis 0) — strafe
    #   angular.yaw ← RS LR (axis 2) — yaw
    # Deadman = A (button 0). Turbo = Y (button 4).
    # `publish_stamped_twist=True` → emit TwistStamped (which is what
    # mecanum_drive_controller subscribes to in Humble).
    teleop_twist = Node(
        package='teleop_twist_joy',
        executable='teleop_node',
        name='teleop_twist_joy_node',
        output='screen',
        parameters=[{
            'enable_button': 0,            # A = chassis deadman
            'enable_turbo_button': 4,      # Y = chassis turbo
            'axis_linear.x': 1,            # LS UD
            'axis_linear.y': 0,            # LS LR
            'axis_angular.yaw': 2,         # RS LR
            'scale_linear.x':  LaunchConfiguration('linear_scale'),
            'scale_linear.y':  LaunchConfiguration('linear_scale'),
            'scale_angular.yaw': LaunchConfiguration('angular_scale'),
            'scale_linear_turbo.x': LaunchConfiguration('linear_turbo_scale'),
            'scale_linear_turbo.y': LaunchConfiguration('linear_turbo_scale'),
            'scale_angular_turbo.yaw': LaunchConfiguration('angular_turbo_scale'),
            'require_enable_button': True,
            'publish_stamped_twist': True,
        }],
        remappings=[
            ('/cmd_vel', '/chassis_controller/reference'),
        ],
    )

    return LaunchDescription([
        urdf_arg,
        controllers_arg,
        arm_jog_rate_arg,
        linear_scale_arg,
        linear_turbo_scale_arg,
        angular_scale_arg,
        angular_turbo_scale_arg,
        active_joint_arg,
        device_id_arg,
        pre_flight_banner,
        robot_state_publisher,
        controller_manager,
        spawn_jsb,
        delay_after_jsb,
        joy_node,
        arm_teleop,
        teleop_twist,
    ])
