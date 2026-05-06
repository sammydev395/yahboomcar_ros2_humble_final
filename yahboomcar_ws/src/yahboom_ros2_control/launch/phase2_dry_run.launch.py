"""D5/D6 Phase 2 dry-run launch — combined gamepad → arm + chassis WITHOUT
any actuator authority anywhere.

THREE-LAYER DRY-RUN GUARANTEE:
  1. ros2_control_node started with dry_run:=true via xacro arg →
     YahboomSystem on_configure logs "DRY RUN: not opening /dev/myserial".
     No serial bytes for chassis FUNC_MOTION or arm FUNC_ARM_CTRL.
  2. arm_teleop_node started with dry_run:=true → logs
     "[DRY-RUN] WOULD publish Float64MultiArray ..." instead of publishing
     to /arm_controller/commands.
  3. teleop_twist_joy_node has /cmd_vel remapped to /dry_run/twist (NOT
     /chassis_controller/reference_unstamped) — no controller subscribes,
     and twist_logger_node makes the messages observable for verification.

NOT spawned: chassis_controller, arm_controller, imu_sensor_broadcaster.
ONLY spawned: joint_state_broadcaster (read-only, makes /joint_states
visible so arm_teleop_node can seed target from it).

Test procedure (per docs/YAHBOOM_GAMEPAD_INTEGRATION_PLAN.md Phase 2):
  $ ros2 launch yahboom_ros2_control phase2_dry_run.launch.py
  Watch the launch log for:
    - "DRY RUN: not opening /dev/myserial" from YahboomSystem
    - "arm_teleop READY [DRY-RUN ...]" from arm_teleop_node
    - "twist_logger listening on /dry_run/twist" from twist_logger_node
  Then exercise the gamepad:
    - Hold A + push left stick → expect [DRY-RUN] Twist lx=... ly=...
    - Hold A + push right stick LR → expect Twist wz=...
    - Hold B + push sticks/D-pad → expect [DRY-RUN] WOULD publish ...
    - Press SELECT → expect [E-STOP] gamepad button 10 pressed → freeze
    - Verify lsof /dev/myserial stays empty for the entire session

Pass criteria: every input → expected log line, with NO serial activity
on /dev/myserial and NO publishes to /chassis_controller/* or
/arm_controller/*.
"""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, RegisterEventHandler
from launch.event_handlers import OnProcessExit
from launch.substitutions import Command, LaunchConfiguration, PathJoinSubstitution
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
    device_id_arg = DeclareLaunchArgument(
        'device_id', default_value='0',
        description='Joystick index. 0 = /dev/input/js0 (= /dev/yahboom_joy).')

    # Force dry_run=true at xacro expansion time.
    robot_description_content = ParameterValue(
        Command(['xacro ', LaunchConfiguration('urdf'), ' dry_run:=true']),
        value_type=str,
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

    # ONLY joint_state_broadcaster — no arm_controller, no chassis_controller.
    spawn_jsb = Node(
        package='controller_manager',
        executable='spawner',
        arguments=['joint_state_broadcaster', '--controller-manager', '/controller_manager'],
    )

    # Joy publisher — same config as d5_joy_test.launch.py.
    joy = Node(
        package='joy',
        executable='joy_node',
        name='joy_node',
        output='screen',
        parameters=[{
            'device_id': LaunchConfiguration('device_id'),
            'deadzone': 0.0,            # raw values for D5 mapping verification
            'autorepeat_rate': 50.0,
            'sticky_buttons': False,
            'coalesce_interval_ms': 1,
        }],
    )

    # teleop_twist_joy — chassis Twist from gamepad. Output remapped to
    # /dry_run/twist so no controller subscribes (chassis_controller isn't
    # spawned anyway, but remap makes Phase 2 isolation explicit).
    # Axis assignment per yahboom_gamepad_map.yaml chassis_with_a_held:
    #   linear.x ← LS UD (axis 1)
    #   linear.y ← LS LR (axis 0)
    #   angular.z ← RS LR (axis 2)
    # Deadman = A (button 0). Turbo = Y (button 4).
    # Phase 4 caps on the scales; vendor full-gear is 0.7 / 3.2.
    teleop_twist = Node(
        package='teleop_twist_joy',
        executable='teleop_node',
        name='teleop_twist_joy_node',
        output='screen',
        parameters=[{
            'enable_button': 0,            # A = chassis deadman
            'enable_turbo_button': 4,      # Y = chassis turbo
            'axis_linear.x': 1,            # LS UD → forward/back
            'axis_linear.y': 0,            # LS LR → strafe
            'axis_angular.yaw': 2,         # RS LR → yaw
            'scale_linear.x': 0.10,        # m/s — Phase 4 cap
            'scale_linear.y': 0.10,
            'scale_angular.yaw': 0.30,     # rad/s — Phase 4 cap
            'scale_linear_turbo.x': 0.30,
            'scale_linear_turbo.y': 0.30,
            'scale_angular_turbo.yaw': 1.00,
            'require_enable_button': True,
        }],
        remappings=[
            ('/cmd_vel', '/dry_run/twist'),
        ],
    )

    # Arm teleop — gamepad → Float64MultiArray, but dry_run=true so it
    # logs WOULD publish instead of publishing.
    arm_teleop = Node(
        package='yahboom_ros2_control',
        executable='arm_teleop_node.py',
        name='arm_teleop',
        output='screen',
        parameters=[{
            'dry_run': True,
        }],
    )

    # Twist logger — subscribes /dry_run/twist, prints non-zero Twist
    # messages in human-readable form.
    twist_logger = Node(
        package='yahboom_ros2_control',
        executable='twist_logger_node.py',
        name='twist_logger',
        output='screen',
    )

    return LaunchDescription([
        urdf_arg,
        controllers_arg,
        device_id_arg,
        robot_state_publisher,
        controller_manager,
        spawn_jsb,
        joy,
        teleop_twist,
        arm_teleop,
        twist_logger,
    ])
