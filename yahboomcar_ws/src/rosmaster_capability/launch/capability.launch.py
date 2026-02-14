from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        Node(
            package='rosmaster_capability',
            executable='capability_node',
            name='capability_node',
            namespace='rosmaster',
            output='screen',
        ),
    ])
