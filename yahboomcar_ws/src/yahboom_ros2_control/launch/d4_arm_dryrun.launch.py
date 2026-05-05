"""D4 arm dry-run launch — chassis NOT spawned, arm in dry-run mode.

Brings up:
  - robot_state_publisher (URDF expanded with dry_run:=true)
  - controller_manager with YahboomSystem in dry_run mode
  - joint_state_broadcaster (no actuator authority)
  - arm_controller (forward_command_controller) — accepts position commands
    on /arm_controller/commands but YahboomSystem.write() logs them as
    [DRY-RUN] WOULD send FUNC_ARM_CTRL ... and never opens serial.

NOT spawned: chassis_controller, imu_sensor_broadcaster (out of scope for
arm pipeline test).

Test after launch:
  ros2 control list_controllers   # arm_controller should be active
  lsof /dev/myserial               # MUST stay empty for whole session
  ros2 topic pub --once /arm_controller/commands std_msgs/msg/Float64MultiArray \\
      "{data: [0.5, 0.0, 0.0, 0.0, 0.0, 0.0]}"
  ros2 topic echo --once /joint_states  # should show arm_joint1=0.5, others=0
  # Look for "[DRY-RUN] WOULD send FUNC_ARM_CTRL" in launch log

D4 PASS criteria:
  - launch comes up cleanly
  - 2 controllers active (jsb + arm_controller)
  - lsof /dev/myserial returns empty (proof YahboomSystem is in dry_run mode)
  - arm position commands → state echoes (perfect-tracker)
  - log shows DRY-RUN frames with correct deg conversions
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

    spawn_jsb = Node(
        package='controller_manager',
        executable='spawner',
        arguments=['joint_state_broadcaster', '--controller-manager', '/controller_manager'],
    )

    spawn_arm = Node(
        package='controller_manager',
        executable='spawner',
        arguments=['arm_controller', '--controller-manager', '/controller_manager'],
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
        robot_state_publisher,
        controller_manager,
        spawn_jsb,
        delay_arm_after_jsb,
    ])
