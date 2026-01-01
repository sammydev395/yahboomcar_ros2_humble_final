#!/usr/bin/env python3
# encoding: utf-8
"""
Voice Control Garbage Transport Node for ROS2.
Handles voice-controlled navigation to color-coded destinations.
Migrated from ROS1 to ROS2 Humble.
"""

import rclpy
from rclpy.node import Node
import cv2 as cv
import threading
from time import sleep

from .transport_common import ROSNav


class ColorTransportNode(Node):
    """
    ROS2 Node for voice-controlled color transport.
    Responds to voice commands to navigate to color-coded destinations.
    """

    def __init__(self):
        super().__init__('color_transport_node')
        
        self.ros_nav = ROSNav(self)
        self.model = "Init"
        self.Grip_status = False
        self.color_name = {}
        self.index = 0
        
        # Voice command codes (from Speech_Lib)
        self.voice_commands = {
            19: 'red',
            20: 'yellow',
            21: 'green',
            32: 'blue'
        }
        
        # Timer for processing
        self.create_timer(0.05, self.process_callback)
        
        self.get_logger().info('Color Transport Node initialized')

    def get_color(self, img):
        """
        Detect color in the center region of an image.
        
        Args:
            img: BGR image
            
        Returns:
            Tuple of (annotated image, color name)
        """
        H = []
        color_name = ""
        
        # Convert to HSV
        HSV = cv.cvtColor(img, cv.COLOR_BGR2HSV)
        
        # Draw detection rectangle
        cv.rectangle(img, (280, 180), (360, 260), (0, 255, 0), 2)
        
        # Collect H values from region
        for i in range(280, 360):
            for j in range(180, 260):
                H.append(HSV[j, i][0])
        
        # Get min/max H values
        H_min = min(H)
        H_max = max(H)
        
        # Determine color
        if (H_min >= 0 and H_max <= 10) or (H_min >= 156 and H_max <= 180):
            color_name = 'red'
        elif H_min >= 26 and H_max <= 34:
            color_name = 'yellow'
        elif H_min >= 35 and H_max <= 78:
            color_name = 'green'
        elif H_min >= 100 and H_max <= 124:
            color_name = 'blue'
        
        txt_H = f'Hmin: {H_min} Hmax: {H_max}'
        cv.putText(img, txt_H, (270, 30), cv.FONT_HERSHEY_SIMPLEX, 
                   0.6, (0, 0, 255), 1)
        
        return img, color_name

    def process_callback(self):
        """Main processing callback."""
        # Note: Voice command integration would require Speech_Lib migration
        # This is a placeholder for the voice command processing
        
        if self.model == "Grip_Target":
            if self.ros_nav.goal_result == 3:
                threading.Thread(target=self.Grip_Target).start()
                
        elif self.model == "come_back":
            self.comeback()
            
        elif self.model == "Grip_down":
            if self.ros_nav.goal_result == 3:
                threading.Thread(target=self.Grip_down).start()

    def navigate_to_color(self, color: str):
        """
        Navigate to a color-coded destination.
        
        Args:
            color: Color name ('red', 'yellow', 'green', 'blue')
        """
        if color in self.ros_nav.color_pose:
            self.ros_nav.PubTargetPoint(self.ros_nav.color_pose[color])
            self.model = "Grip_Target"
            self.get_logger().info(f'Navigating to {color} destination')
        else:
            self.get_logger().warn(f'Color pose not found: {color}')

    def comeback(self):
        """Navigate back to start position."""
        self.ros_nav.PubTargetPoint(self.ros_nav.start_point)
        self.model = "Grip_down"

    def Reset(self):
        """Reset all states."""
        self.ros_nav.goal_result = 0
        self.model = "Init"
        self.Grip_status = False
        self.ros_nav.Transport_status = False
        self.color_name = {}
        self.index = 0

    def Grip_down(self):
        """Execute drop-off sequence."""
        self.model = "Grip_down"
        self.ros_nav.goal_result = 0
        
        joints = [90.0, 2.0, 60.0, 40.0, 90.0, 140.0]
        self.ros_nav.pubArm(joints, run_time=1000)
        sleep(1)
        
        self.ros_nav.pubArm([], 6, 30)
        sleep(0.5)
        
        joints = [90.0, 145.0, 0.0, 45.0, 90.0, 30.0]
        self.ros_nav.pubArm(joints, run_time=1000)
        sleep(1)
        
        self.get_logger().info('Drop-off complete')

    def Grip_Target(self):
        """Execute grip sequence."""
        self.model = "Grip_Target"
        self.ros_nav.goal_result = 0
        self.Grip_status = True
        
        self.buzzer_loop()
        
        joints = [90.0, 145.0, 0.0, 45.0, 90.0, 30.0]
        self.ros_nav.pubArm(joints, run_time=1000)
        sleep(0.5)
        
        self.buzzer_loop()
        
        self.ros_nav.pubArm([], 6, 140)
        sleep(1)
        
        self.get_logger().info('Grip complete, returning')
        self.model = "come_back"

    def buzzer_loop(self):
        """Sound buzzer once."""
        self.ros_nav.pubBuzzer(True)
        sleep(1)
        self.ros_nav.pubBuzzer(False)
        sleep(1)


def main(args=None):
    rclpy.init(args=args)
    node = ColorTransportNode()
    
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.ros_nav.cancel()
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()

