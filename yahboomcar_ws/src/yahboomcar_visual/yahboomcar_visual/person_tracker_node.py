#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image
from cv_bridge import CvBridge
import cv2
import os
from ultralytics import YOLO
from yahboomcar_msgs.msg import TrackedPerson, TrackedPersons
import time

class PersonTrackerNode(Node):
    def __init__(self):
        super().__init__('person_tracker_node')
        
        # Parameters
        self.declare_parameter('model_path', '/root/ultralytics/ultralytics/yolo11n.engine')
        self.declare_parameter('camera_topic', '/camera/color/image_raw')
        self.declare_parameter('conf_threshold', 0.25)
        self.declare_parameter('iou_threshold', 0.45)
        self.declare_parameter('publish_annotated_image', True)
        
        model_path = self.get_parameter('model_path').get_parameter_value().string_value
        camera_topic = self.get_parameter('camera_topic').get_parameter_value().string_value
        self.conf_threshold = self.get_parameter('conf_threshold').get_parameter_value().double_value
        self.iou_threshold = self.get_parameter('iou_threshold').get_parameter_value().double_value
        self.publish_annotated = self.get_parameter('publish_annotated_image').get_parameter_value().bool_value
        
        self.get_logger().info(f'Loading model from {model_path}...')
        self.model = YOLO(model_path)
        self.get_logger().info('Model loaded successfully.')
        
        self.bridge = CvBridge()
        
        # Subscription
        self.image_sub = self.create_subscription(
            Image,
            camera_topic,
            self.image_callback,
            10
        )
        
        # Publishers
        self.person_pub = self.create_publisher(TrackedPersons, '/person_tracker/tracked_persons', 10)
        
        if self.publish_annotated:
            self.annotated_image_pub = self.create_publisher(Image, '/person_tracker/annotated_image', 10)
            
        self.last_time = time.time()
        self.frame_count = 0
        
        self.get_logger().info(f'Person Tracker Node started. Subscribing to {camera_topic}')

    def image_callback(self, msg):
        try:
            # Convert ROS Image message to OpenCV image
            cv_image = self.bridge.imgmsg_to_cv2(msg, desired_encoding='bgr8')
            
            # Run YOLOv11 tracking
            # classes=[0] is for person in COCO dataset
            results = self.model.track(
                source=cv_image,
                persist=True,
                classes=[0],
                conf=self.conf_threshold,
                iou=self.iou_threshold,
                verbose=False
            )
            
            tracked_persons_msg = TrackedPersons()
            tracked_persons_msg.header = msg.header
            
            if results[0].boxes.id is not None:
                boxes = results[0].boxes.xyxy.cpu().numpy()
                track_ids = results[0].boxes.id.int().cpu().numpy()
                confidences = results[0].boxes.conf.cpu().numpy()
                
                for box, track_id, conf in zip(boxes, track_ids, confidences):
                    person = TrackedPerson()
                    person.track_id = int(track_id)
                    person.bbox = [float(box[0]), float(box[1]), float(box[2]), float(box[3])]
                    person.confidence = float(conf)
                    person.center_x = float((box[0] + box[2]) / 2.0)
                    person.center_y = float((box[1] + box[3]) / 2.0)
                    tracked_persons_msg.persons.append(person)
            
            # Publish tracked persons
            self.person_pub.publish(tracked_persons_msg)
            
            # Publish annotated image
            if self.publish_annotated:
                annotated_frame = results[0].plot()
                annotated_msg = self.bridge.cv2_to_imgmsg(annotated_frame, encoding='bgr8')
                annotated_msg.header = msg.header
                self.annotated_image_pub.publish(annotated_msg)
                
            # Calculate FPS
            self.frame_count += 1
            current_time = time.time()
            if current_time - self.last_time >= 1.0:
                fps = self.frame_count / (current_time - self.last_time)
                self.get_logger().info(f'FPS: {fps:.2f}, Persons tracked: {len(tracked_persons_msg.persons)}')
                self.frame_count = 0
                self.last_time = current_time
                
        except Exception as e:
            self.get_logger().error(f'Error in image_callback: {str(e)}')

def main(args=None):
    rclpy.init(args=args)
    node = PersonTrackerNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()
