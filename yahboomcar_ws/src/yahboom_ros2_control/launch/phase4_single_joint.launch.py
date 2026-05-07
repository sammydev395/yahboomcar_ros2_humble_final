"""D7.4 / Phase 4 single-joint live arm test launch — LIVE MODE.

FIRST time the X3PLUS arm sees torque under the ros2_control stack. Brings
up arm-only:
  - robot_state_publisher (URDF expanded with dry_run:=false)
  - controller_manager with YahboomSystem in LIVE mode
  - joint_state_broadcaster
  - arm_controller (forward_command_controller, position interface)

NOT spawned: chassis_controller (no wheel command flow during arm Phase 4),
imu_sensor_broadcaster (out of scope), arm_teleop_node (operator drives via
direct topic publishes — no gamepad path), joy_node.

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

Operator workflow (after launch):
  Step 1. Verify activation succeeded:
      ros2 control list_controllers
        # joint_state_broadcaster: active
        # arm_controller:          active
      ros2 topic echo --once /joint_states
        # Note the SIX arm position values. Write them down. These are
        # the seeded pose YahboomSystem read from hardware.

  Step 2. Read out the active_joint argument from this launch (printed in
      banner above; default is arm_joint1). All commands hold the OTHER
      five joints at their seeded value, only the active joint moves.

  Step 3. Build the initial command (cmd == seeded pose, will be no-op):
      JOINTS=[arm_joint1, arm_joint2, arm_joint3, arm_joint4, arm_joint5, grip_joint]
      Use the values from /joint_states. Publish:
        ros2 topic pub --once /arm_controller/commands \\
            std_msgs/msg/Float64MultiArray \\
            "{data: [V1, V2, V3, V4, V5, V6]}"
      Verify: NO MOTION (sub-1° lurch is the PASS bar — TELEOP_PHASE4
      lessons 1-4). If the arm jumps more than 1° on this first cmd,
      ABORT — the seed code didn't work.

  Step 4. Nudge ONLY the active joint by +0.05 rad (the per-tick clamp from
      arm_teleop's Phase 3 safety, applied here as a manual self-discipline
      cap). Re-publish with the active joint's value bumped by +0.05;
      hold the rest. Verify motion is smooth. Pause. Read /joint_states
      to confirm new state matches the new command (within 0.01 rad).

  Step 5. Repeat in the opposite direction (-0.05 rad). Then a couple more
      nudges in each direction. PASS bar: each step settles within 1°,
      no overshoot, no oscillation. Lurch-on-release < 1° per
      TELEOP_PHASE4_LESSONS.md.

  Step 6. Mark joint PASS in the architecture doc (D7.5 row); deactivate
      controllers and ros2 lifecycle, then re-launch with the next
      active_joint per the test order:
          joint1 → joint5 → joint6 → joint4 → joint3 → joint2

Abort criteria (any of these = STOP):
  - Lurch on first publish > 1° (seed didn't work — investigate)
  - on_configure ERROR (joint outside URDF range — re-pose and retry)
  - on_activate ERROR (operator back-drove between configure & activate
    such that a joint is now outside range — re-pose and retry)
  - Step doesn't settle within 1° → controller tuning issue
  - Any unexpected motion of a non-active joint
  - Any servo audible distress (whining, clicking) → deactivate, --torque-off

Recovery from any abort:
  ros2 lifecycle set /controller_manager deactivate
  ros2 run yahboom_ros2_control smoke_serial /dev/myserial --torque-off
  # Re-pose by hand, --query-arm to verify, then start over.

Default active_joint = arm_joint1 (highest test priority — base yaw, simplest
kinematics, lowest risk of self-collision).
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
        description=('Joint under test for this Phase 4 session. Operator '
                     'commands MUST move ONLY this joint; the other five '
                     'must hold their seeded value. Test order across '
                     'launches: arm_joint1 → arm_joint5 → grip_joint → '
                     'arm_joint4 → arm_joint3 → arm_joint2.'))

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
            '  active_joint = ', LaunchConfiguration('active_joint'), '\n',
            '  PASS bar     = lurch-on-release sub-1°\n',
            '                 (TELEOP_PHASE4_LESSONS.md lessons 1-4)\n',
            '  per-tick cap = 0.05 rad (manual self-discipline)\n',
            '  speed cap    = 0.10 rad/s\n',
            '─────────────────────────────────────────────────────────────\n',
            '  ABORT if first cmd lurches > 1° (seed code failure)\n',
            '  ABORT if any non-active joint moves\n',
            '  ABORT on servo distress (whine/click)\n',
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

    return LaunchDescription([
        urdf_arg,
        controllers_arg,
        active_joint_arg,
        pre_flight_banner,
        robot_state_publisher,
        controller_manager,
        spawn_jsb,
        delay_arm_after_jsb,
    ])
