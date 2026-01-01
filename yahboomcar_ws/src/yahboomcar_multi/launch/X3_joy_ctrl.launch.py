from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument 
from launch.substitutions import LaunchConfiguration 

def generate_launch_description():
    first_robot_arg = DeclareLaunchArgument('first_robot1', default_value='robot1',)
    second_robot_arg = DeclareLaunchArgument('second_robot2', default_value='robot2',)
    third_robot_arg = DeclareLaunchArgument('third_robot3', default_value='robot3',)

    first_robot = LaunchConfiguration('first_robot1')
    second_robot = LaunchConfiguration('second_robot2')
    third_robot = LaunchConfiguration('third_robot3')

    joy_node = Node(
        package='joy',
        executable='joy_node',
    )

    robot1_joy_ctrl_node = Node(
        package='yahboomcar_multi',  
        executable='yahboomcar_X3_ctrl',
        name='joy_ctrl_robot1',
        output='screen',
        remappings=[
            ('cmd_vel', [first_robot, '/cmd_vel']),
            ('Buzzer', [first_robot, '/Buzzer']),
            ('RGBLight', [first_robot, '/RGBLight']),
            ('JoyState', [first_robot, '/JoyState']),
        ],
    )

    robot2_joy_ctrl_node = Node(
        package='yahboomcar_multi',
        executable='yahboomcar_X3_ctrl',
        name='joy_ctrl_robot2',
        output='screen',
        remappings=[
            ('cmd_vel', [second_robot, '/cmd_vel']),
            ('Buzzer', [second_robot, '/Buzzer']),
            ('RGBLight', [second_robot, '/RGBLight']),
            ('JoyState', [second_robot, '/JoyState']),
        ],
    )

    robot3_joy_ctrl_node = Node(
        package='yahboomcar_multi',
        executable='yahboomcar_X3_ctrl',
        name='joy_ctrl_robot3',
        output='screen',
        remappings=[
            ('cmd_vel', [third_robot, '/cmd_vel']),
            ('Buzzer', [third_robot, '/Buzzer']),
            ('RGBLight', [third_robot, '/RGBLight']),
            ('JoyState', [third_robot, '/JoyState']),
        ],
    )

    return LaunchDescription([
        joy_node,
        first_robot_arg, 
        second_robot_arg,
        third_robot_arg,
        robot1_joy_ctrl_node,
        robot2_joy_ctrl_node,
        robot3_joy_ctrl_node,
    ])
