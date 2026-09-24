"""D2 chassis-only launch for the X3PLUS — namespaced under /rosmaster.

Brings up (all under the /rosmaster namespace):
  - robot_state_publisher (from x3plus_chassis.urdf.xacro)
  - controller_manager (ros2_control_node) with YahboomSystem plugin
  - joint_state_broadcaster (spawned)
  - imu_sensor_broadcaster (spawned)
  - chassis_controller / mecanum_drive_controller (spawned)

Namespacing rationale (2026-09-24): Rosmaster + Ultra share
ROS_DOMAIN_ID=100. Unnamespaced controller topics
(/chassis_controller/reference_unstamped, /arm_controller/commands)
collide fleet-wide — a command meant for one robot lands on both.
Verified incidents: Rosmaster gamepad B moved Ultra's arm; menu arm
command moved Ultra's arm instead of Rosmaster's. Everything here now
lives under /rosmaster.

Test after launch:
  ros2 topic pub --rate 10 /rosmaster/chassis_controller/reference_unstamped \\
      geometry_msgs/msg/Twist '{linear: {x: 0.1}}'
  ros2 topic echo /rosmaster/joint_states --once
"""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import Command, LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.substitutions import FindPackageShare

ROBOT_NS = 'rosmaster'


def generate_launch_description():
    pkg_share = FindPackageShare('yahboom_ros2_control')

    urdf_path = PathJoinSubstitution(
        [pkg_share, 'description', 'x3plus_chassis.urdf.xacro'])

    controllers_yaml = PathJoinSubstitution(
        [pkg_share, 'config', 'ros2_controllers.yaml'])

    urdf_arg = DeclareLaunchArgument(
        'urdf',
        default_value=urdf_path,
        description='Path to URDF/xacro for the X3PLUS chassis.',
    )
    controllers_arg = DeclareLaunchArgument(
        'controllers',
        default_value=controllers_yaml,
        description='Path to ros2_controllers.yaml.',
    )

    robot_description_content = ParameterValue(
        Command(['xacro ', LaunchConfiguration('urdf')]),
        value_type=str,
    )

    robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        namespace=ROBOT_NS,
        output='screen',
        parameters=[{'robot_description': robot_description_content}],
    )

    controller_manager = Node(
        package='controller_manager',
        executable='ros2_control_node',
        namespace=ROBOT_NS,
        output='screen',
        parameters=[
            {'robot_description': robot_description_content},
            LaunchConfiguration('controllers'),
        ],
    )

    # NOTE: spawners are launched all-at-once (no OnProcessExit chaining).
    # The 2.53.1 spawner is unreliable on this stack anyway (silent
    # load/configure failures when stale DDS graph entries exist), so
    # start_teleop_stack.sh force-configures each controller after launch;
    # the spawners here are best-effort first pass.
    cm_fqn = f'/{ROBOT_NS}/controller_manager'
    spawn_jsb = Node(
        package='controller_manager',
        executable='spawner',
        namespace=ROBOT_NS,
        arguments=['joint_state_broadcaster', '--controller-manager', cm_fqn],
    )
    spawn_imu = Node(
        package='controller_manager',
        executable='spawner',
        namespace=ROBOT_NS,
        arguments=['imu_sensor_broadcaster', '--controller-manager', cm_fqn],
    )
    spawn_chassis = Node(
        package='controller_manager',
        executable='spawner',
        namespace=ROBOT_NS,
        arguments=['chassis_controller', '--controller-manager', cm_fqn],
    )

    return LaunchDescription([
        urdf_arg,
        controllers_arg,
        robot_state_publisher,
        controller_manager,
        spawn_jsb,
        spawn_imu,
        spawn_chassis,
    ])
