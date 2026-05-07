"""D7.4 / Phase 4 single-joint live arm test launch — LIVE MODE.

FIRST time the X3PLUS arm sees torque under the ros2_control stack, with
the gamepad in the loop. Mirrors Ultra's `phase4_single_joint.launch.py`
pattern: arm_teleop_node sits between the joy_node and the
arm_controller, gating motion behind the deadman button and capping
per-tick deltas via Phase 3 safety nets (already in arm_teleop_node.py
since D6).

Brings up:
  - robot_state_publisher (URDF expanded with dry_run:=false)
  - controller_manager with YahboomSystem in LIVE mode
  - joint_state_broadcaster
  - arm_controller (forward_command_controller, position interface)
  - joy_node — gamepad → /joy
  - arm_teleop_node — /joy + /joint_states → /arm_controller/commands
    with active_joint gating + phase4_jog_rate cap

NOT spawned: chassis_controller (no wheel command flow during arm
Phase 4), imu_sensor_broadcaster (out of scope), teleop_twist_joy_node
(no chassis Twist needed).

Why arm_teleop_node is REQUIRED here (not optional):
  forward_command_controller's update() is a no-op if no
  Float64MultiArray has been received on /arm_controller/commands. But
  the moment the controller sees its FIRST commanded value, it begins
  writing to its command interfaces every cycle. arm_teleop_node now
  publishes the seeded target on EVERY tick (50 Hz) regardless of
  deadman state — so /arm_controller/commands always has fresh data
  matching state, YahboomSystem.write() emits no-op heartbeats, and
  the arm holds. Without arm_teleop_node in the launch (or any other
  publisher), running phase4 with just JSB + arm_controller produces
  joint-2 oscillation as the controller's command interfaces drift
  toward default-zero (D7.5 first-launch failure 2026-05-07).

Pre-flight (do BEFORE running this launch):
  1. Verify all six arm joints inside URDF range:
       ros2 run yahboom_ros2_control smoke_serial /dev/myserial --query-arm
     If ANY joint reads OUTSIDE [kArmUrdfLo, kArmUrdfHi]: disable torque
     ('smoke_serial /dev/myserial --torque-off'), back-drive into range,
     re-query, then proceed. YahboomSystem on_configure refuses to seed
     otherwise — launch will ERROR cleanly.
  2. Confirm the arm is in a SAFE physical pose (not at hard mechanical
     limits, no obstructions). Operator hand on the arm. Power easy to
     reach for E-stop.
  3. Confirm vendor stack is NOT running (no rosmaster_main.py, no
     Mcnamu_driver_X3plus). lsof /dev/myserial must return nothing.
  4. Gamepad receiver plugged in; /dev/input/js0 enumerates as the
     DragonRise (vid:pid 0079:181c) per yahboom_gamepad_map.yaml.

Operator workflow (after launch comes up):
  Step 1. Verify activation succeeded (logs):
      "arm seeded: cmd = state from hardware (first write == no-op)"
      "arm torque ENABLED, holding configure-seeded pose: deg=[...]"
      "arm_controller activate successful"
      "arm_teleop ... seeded target from /joint_states: [...]"
    Watch for `[FAULT]` lines (none expected).

  Step 2. WITHOUT pressing deadman: confirm the arm is rigid (torque on,
    holding seeded pose) and `/arm_controller/commands` is publishing at
    50 Hz from arm_teleop. NO joint should move.

      ros2 topic hz /arm_controller/commands     # ~50 Hz expected
      ros2 topic echo --once /joint_states       # arm_jointN positions

  Step 3. Press and hold B (deadman, button 1). Push the active_joint's
    stick a SHORT amount in one direction. PASS bar: lurch-on-release
    sub-1° per TELEOP_PHASE4_LESSONS.md. Release deadman; arm holds at
    new position. JOINT_MAP from yahboom arm_teleop_node:
      - arm_joint1 → LS LR    (axis 0)
      - arm_joint2 → LS UD    (axis 1)
      - arm_joint3 → RS UD    (axis 3)
      - arm_joint4 → D-pad LR (axis 6)
      - arm_joint5 → RS LR    (axis 2)  — asymmetric range -π/2..+π
      - grip_joint → D-pad UD (axis 7)  — range entirely negative
    See yahboom_gamepad_map.yaml for the full gamepad layout.

  Step 4. Repeat in opposite direction. Then a couple more nudges in
    each direction. PASS bar: each step settles within 1°, no overshoot,
    no oscillation. NO non-active joint should move (Phase 3 active_joint
    gating).

  Step 5. Press SELECT (button 10) to E-stop. arm_teleop calls ~/freeze;
    target snaps to current /joint_states; /arm_controller/commands
    holds at frozen pose; chassis-stop TwistStamped published (no chassis
    here, but the publish should be visible in `ros2 topic echo`).

  Step 6. Mark joint PASS in the architecture doc (D7.5 row); Ctrl+C
    the launch (on_deactivate fires, leaves torque ON), re-launch with
    the next active_joint per the test order:
        arm_joint1 → arm_joint5 → grip_joint → arm_joint4 → arm_joint3
        → arm_joint2 (last and most cautious — shoulder)

Abort criteria (any of these = STOP, release deadman, hit SELECT):
  - Lurch on activation > 1° (seed didn't work — investigate)
  - on_configure ERROR (joint outside URDF range — re-pose and retry)
  - on_activate ERROR (operator back-drove between configure & activate
    such that a joint is now outside range — re-pose and retry)
  - Step doesn't settle within 1° → controller tuning issue
  - Any unexpected motion of a non-active joint
  - Any servo audible distress (whining, clicking) → deactivate, --torque-off
  - [FAULT] lines in arm_teleop log (discontinuity reject)

Recovery from any abort:
  Ctrl+C the launch (on_deactivate runs, torque stays ON)
  ros2 run yahboom_ros2_control smoke_serial /dev/myserial --torque-off
  # Re-pose by hand, --query-arm to verify, then start over.

Default active_joint = arm_joint1 (highest test priority — base yaw,
simplest kinematics, lowest risk of self-collision).
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
    active_joint_arg = DeclareLaunchArgument(
        'active_joint', default_value='arm_joint1',
        description=('Joint under test for this Phase 4 session. arm_teleop '
                     'gates stick input so ONLY this joint moves; the other '
                     'five hold their seeded value. Test order across '
                     'launches: arm_joint1 → arm_joint5 → grip_joint → '
                     'arm_joint4 → arm_joint3 → arm_joint2.'))
    phase4_jog_rate_arg = DeclareLaunchArgument(
        'phase4_jog_rate', default_value='0.10',
        description=('Jog rate cap (rad/s). Phase 4 default 0.10 rad/s '
                     'forces a slow uniform cap regardless of JOINT_MAP '
                     'per-joint rates. Set to 0.0 to use the per-joint '
                     'rates instead (Phase 5+ only).'))
    device_id_arg = DeclareLaunchArgument(
        'device_id', default_value='0',
        description=('Joystick index. 0 = /dev/input/js0 (= /dev/yahboom_joy '
                     'udev symlink — DragonRise vid:pid 0079:181c).'))

    # LIVE mode — dry_run BAKED IN to false so accidental re-source of an
    # older launch can't put us back into dry-run silently. The xacro arg
    # forces YahboomSystem to open /dev/myserial and run the full
    # configure/activate sequence (query → validate → seed → torque-on).
    robot_description_content = ParameterValue(
        Command(['xacro ', LaunchConfiguration('urdf'), ' dry_run:=false']),
        value_type=str,
    )

    pre_flight_banner = LogInfo(
        msg=[
            '\n',
            '═════════════════════════════════════════════════════════════\n',
            '  PHASE 4 SINGLE-JOINT LIVE TEST — LIVE MODE (dry_run=false)\n',
            '─────────────────────────────────────────────────────────────\n',
            '  active_joint    = ', LaunchConfiguration('active_joint'), '\n',
            '  phase4_jog_rate = ', LaunchConfiguration('phase4_jog_rate'), ' rad/s\n',
            '  PASS bar        = lurch-on-release sub-1°\n',
            '                    (TELEOP_PHASE4_LESSONS.md lessons 1-4)\n',
            '─────────────────────────────────────────────────────────────\n',
            '  Hold B (button 1) deadman + active joint stick to jog.\n',
            '  Press SELECT (button 10) for E-stop / freeze.\n',
            '─────────────────────────────────────────────────────────────\n',
            '  ABORT if first cmd lurches > 1° (seed code failure)\n',
            '  ABORT if any non-active joint moves\n',
            '  ABORT on servo distress (whine/click)\n',
            '  ABORT on any [FAULT] line in arm_teleop log\n',
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

    delay_arm_after_jsb = RegisterEventHandler(
        event_handler=OnProcessExit(
            target_action=spawn_jsb,
            on_exit=[spawn_arm],
        )
    )

    # joy_node — gamepad → /joy. Same configuration as
    # phase2_dry_run.launch.py (autorepeat 50 Hz so /joy stays fresh
    # while a stick is held; coalesce 1 ms for responsiveness).
    joy_node = Node(
        package='joy',
        executable='joy_node',
        name='joy_node',
        output='screen',
        parameters=[{
            'device_id': LaunchConfiguration('device_id'),
            'deadzone': 0.0,            # arm_teleop applies INPUT_DEADZONE
            'autorepeat_rate': 50.0,
            'sticky_buttons': False,
            'coalesce_interval_ms': 1,
        }],
    )

    # arm_teleop — gamepad → Float64MultiArray on /arm_controller/commands.
    # LIVE (dry_run=false default; explicit). active_joint and
    # phase4_jog_rate flow through from launch args. The node:
    #   - On first /joint_states message, seeds target from real arm pose.
    #   - On EVERY tick (50 Hz), publishes target — even when deadman is
    #     not held. This keeps forward_command_controller's command
    #     interfaces continuously synced to our target so YahboomSystem.
    #     write() emits no-op heartbeats and the arm holds. (Without this
    #     idle-republish, observed joint2 oscillation 2026-05-07.)
    #   - When deadman IS held + active_joint stick has input, target
    #     advances by jog_rate × dt × stick_value, capped at 0.05 rad/tick
    #     and gated by SOFT_LIMITS. Non-active joints don't accept input.
    #   - SELECT (button 10) calls ~/freeze: target snaps to current
    #     /joint_states + chassis Twist zero published.
    arm_teleop = Node(
        package='yahboom_ros2_control',
        executable='arm_teleop_node.py',
        name='arm_teleop',
        output='screen',
        parameters=[{
            'active_joint': LaunchConfiguration('active_joint'),
            'phase4_jog_rate': LaunchConfiguration('phase4_jog_rate'),
            'dry_run': False,
        }],
    )

    return LaunchDescription([
        urdf_arg,
        controllers_arg,
        active_joint_arg,
        phase4_jog_rate_arg,
        device_id_arg,
        pre_flight_banner,
        robot_state_publisher,
        controller_manager,
        spawn_jsb,
        delay_arm_after_jsb,
        joy_node,
        arm_teleop,
    ])
