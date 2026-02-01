#!/usr/bin/env python3
# encoding: utf-8
"""
ROS2 Face Detection Node for Yahboom ROSMASTER X3PLUS
Subscribes to Astra Pro camera (/color/image_raw) and publishes face detections
"""

import rclpy
from rclpy.node import Node
from cv_bridge import CvBridge
from sensor_msgs.msg import Image
import mediapipe as mp
import cv2 as cv
import time


class FaceDetectionNode(Node):
    def __init__(self):
        super().__init__('face_detection_node')
        
        # MediaPipe Face Detection setup
        self.mp_face_detection = mp.solutions.face_detection
        self.mp_draw = mp.solutions.drawing_utils
        self.face_detection = self.mp_face_detection.FaceDetection(
            min_detection_confidence=0.5
        )
        
        # CV Bridge for image conversion
        self.bridge = CvBridge()
        
        # Declare parameters
        self.declare_parameter('camera_topic', '/color/image_raw')
        self.declare_parameter('min_detection_confidence', 0.5)
        self.declare_parameter('publish_annotated_image', True)
        
        # Get parameters
        camera_topic = self.get_parameter('camera_topic').value
        min_conf = self.get_parameter('min_detection_confidence').value
        self.publish_annotated = self.get_parameter('publish_annotated_image').value
        
        # Update detection confidence if parameter changed
        if min_conf != 0.5:
            self.face_detection = self.mp_face_detection.FaceDetection(
                min_detection_confidence=min_conf
            )
        
        # Subscribers
        self.image_sub = self.create_subscription(
            Image,
            camera_topic,
            self.image_callback,
            10
        )
        
        # Publishers
        self.annotated_image_pub = self.create_publisher(
            Image,
            '/face_detection/annotated_image',
            10
        )
        
        # Statistics
        self.frame_count = 0
        self.face_count = 0
        self.last_time = time.time()
        self.fps = 0.0
        
        self.get_logger().info(f'Face Detection Node initialized')
        self.get_logger().info(f'Subscribing to: {camera_topic}')
        self.get_logger().info(f'Min detection confidence: {min_conf}')
    
    def image_callback(self, msg):
        """Process incoming image and detect faces"""
        try:
            # Convert ROS Image to OpenCV format
            frame = self.bridge.imgmsg_to_cv2(msg, "bgr8")
            
            # Detect faces
            frame, bboxes = self.find_faces(frame)
            
            # Calculate FPS
            self.frame_count += 1
            current_time = time.time()
            if current_time - self.last_time >= 1.0:
                self.fps = self.frame_count / (current_time - self.last_time)
                self.frame_count = 0
                self.last_time = current_time
                self.get_logger().info(
                    f'FPS: {self.fps:.1f}, Faces detected: {len(bboxes)}'
                )
            
            # Publish annotated image if enabled
            if self.publish_annotated:
                annotated_msg = self.bridge.cv2_to_imgmsg(frame, "bgr8")
                annotated_msg.header = msg.header
                self.annotated_image_pub.publish(annotated_msg)
                
        except Exception as e:
            self.get_logger().error(f'Error processing image: {str(e)}')
    
    def find_faces(self, frame):
        """Detect faces in frame using MediaPipe"""
        img_rgb = cv.cvtColor(frame, cv.COLOR_BGR2RGB)
        results = self.face_detection.process(img_rgb)
        bboxes = []
        
        if results.detections:
            for detection in results.detections:
                # Get bounding box
                bbox_c = detection.location_data.relative_bounding_box
                ih, iw, _ = frame.shape
                
                x = int(bbox_c.xmin * iw)
                y = int(bbox_c.ymin * ih)
                w = int(bbox_c.width * iw)
                h = int(bbox_c.height * ih)
                
                bbox = (x, y, w, h)
                confidence = detection.score[0]
                bboxes.append([bbox, confidence])
                
                # Draw face detection
                frame = self.fancy_draw(frame, bbox)
                cv.putText(
                    frame,
                    f'{int(confidence * 100)}%',
                    (x, y - 20),
                    cv.FONT_HERSHEY_PLAIN,
                    2,
                    (255, 0, 255),
                    2
                )
                
                # Draw key points (eyes, nose, mouth)
                if detection.location_data.relative_keypoints:
                    for keypoint in detection.location_data.relative_keypoints:
                        kx = int(keypoint.x * iw)
                        ky = int(keypoint.y * ih)
                        cv.circle(frame, (kx, ky), 5, (0, 255, 0), -1)
        
        return frame, bboxes
    
    def fancy_draw(self, frame, bbox, l=30, t=10):
        """Draw fancy bounding box around face"""
        x, y, w, h = bbox
        x1, y1 = x + w, y + h
        
        # Main rectangle
        cv.rectangle(frame, (x, y), (x1, y1), (255, 0, 255), 2)
        
        # Corner decorations
        # Top left
        cv.line(frame, (x, y), (x + l, y), (255, 0, 255), t)
        cv.line(frame, (x, y), (x, y + l), (255, 0, 255), t)
        # Top right
        cv.line(frame, (x1, y), (x1 - l, y), (255, 0, 255), t)
        cv.line(frame, (x1, y), (x1, y + l), (255, 0, 255), t)
        # Bottom left
        cv.line(frame, (x, y1), (x + l, y1), (255, 0, 255), t)
        cv.line(frame, (x, y1), (x, y1 - l), (255, 0, 255), t)
        # Bottom right
        cv.line(frame, (x1, y1), (x1 - l, y1), (255, 0, 255), t)
        cv.line(frame, (x1, y1), (x1, y1 - l), (255, 0, 255), t)
        
        return frame


def main(args=None):
    rclpy.init(args=args)
    face_detection_node = FaceDetectionNode()
    
    try:
        rclpy.spin(face_detection_node)
    except KeyboardInterrupt:
        pass
    finally:
        face_detection_node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
