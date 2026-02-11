#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

def generate_launch_description():
    # Declare launch arguments
    model_path_arg = DeclareLaunchArgument(
        'model_path',
        default_value='/root/ultralytics/ultralytics/yolo11n.engine',
        description='Path to YOLO model file (.pt, .engine, etc.)'
    )
    
    camera_topic_arg = DeclareLaunchArgument(
        'camera_topic',
        default_value='/color/image_raw',
        description='Camera image topic to subscribe to'
    )
    
    conf_threshold_arg = DeclareLaunchArgument(
        'conf_threshold',
        default_value='0.25',
        description='Confidence threshold for detections'
    )
    
    iou_threshold_arg = DeclareLaunchArgument(
        'iou_threshold',
        default_value='0.45',
        description='IOU threshold for NMS'
    )
    
    publish_annotated_arg = DeclareLaunchArgument(
        'publish_annotated_image',
        default_value='true',
        description='Whether to publish annotated images'
    )
    
    # Person tracker node
    person_tracker_node = Node(
        package='yahboomcar_visual',
        executable='person_tracker_node',
        name='person_tracker_node',
        output='screen',
        parameters=[{
            'model_path': LaunchConfiguration('model_path'),
            'camera_topic': LaunchConfiguration('camera_topic'),
            'conf_threshold': LaunchConfiguration('conf_threshold'),
            'iou_threshold': LaunchConfiguration('iou_threshold'),
            'publish_annotated_image': LaunchConfiguration('publish_annotated_image')
        }]
    )
    
    return LaunchDescription([
        model_path_arg,
        camera_topic_arg,
        conf_threshold_arg,
        iou_threshold_arg,
        publish_annotated_arg,
        person_tracker_node
    ])
