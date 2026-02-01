from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    # Launch arguments
    camera_topic_arg = DeclareLaunchArgument(
        'camera_topic',
        default_value='/color/image_raw',
        description='Camera topic to subscribe to'
    )
    
    min_confidence_arg = DeclareLaunchArgument(
        'min_detection_confidence',
        default_value='0.5',
        description='Minimum detection confidence for face detection (0.0-1.0)'
    )
    
    publish_annotated_arg = DeclareLaunchArgument(
        'publish_annotated_image',
        default_value='true',
        description='Whether to publish annotated image with face detections'
    )
    
    # Face Detection Node
    face_detection_node = Node(
        package='yahboomcar_mediapipe',
        executable='face_detection_node',
        name='face_detection_node',
        output='screen',
        parameters=[{
            'camera_topic': LaunchConfiguration('camera_topic'),
            'min_detection_confidence': LaunchConfiguration('min_detection_confidence'),
            'publish_annotated_image': LaunchConfiguration('publish_annotated_image'),
        }],
        remappings=[
            # Remap camera topic if needed
            ('/color/image_raw', LaunchConfiguration('camera_topic')),
        ]
    )
    
    return LaunchDescription([
        camera_topic_arg,
        min_confidence_arg,
        publish_annotated_arg,
        face_detection_node,
    ])
