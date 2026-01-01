#!/usr/bin/env python3
# coding: utf-8
"""
Garbage Identification and Transport Node for ROS2.
Uses YOLO11 for garbage detection and controls the robot arm to sort garbage.
Migrated from ROS1 to ROS2 Humble.
"""

import rclpy
from rclpy.node import Node
from time import sleep

from std_msgs.msg import String

from .transport_common import ROSNav
from .garbage_library import GarbageTransport


# Garbage classification categories
RECYCLABLE_WASTE = ['newspaper', 'zip-top_can', 'book', 'old_school_bag']
TOXIC_WASTE = ['syringe', 'expired_cosmetics', 'used_batteries', 'expired_tablets']
WET_WASTE = ['fish_bone', 'egg_shell', 'apple_core', 'watermelon_rind']
DRY_WASTE = ['toilet_paper', 'peach_pit', 'cigarette_butts', 'disposable_chopsticks']


class GarbageIdentifyNode(Node):
    """
    ROS2 Node for garbage identification and transport.
    Subscribes to detection messages and controls the garbage sorting process.
    """

    def __init__(self):
        super().__init__('garbage_identify_node')
        
        self.img = None
        self.garbage_index = 0
        self.name = None
        self.garbage_result = 999
        
        # Initialize ROS navigation and transport
        self.ros_nav = ROSNav(self)
        self.garbage_transbot = GarbageTransport(self.ros_nav)
        
        # Subscribe to detection messages
        self.create_subscription(
            String, 'DetectMsg', self.detect_msg_callback, 10
        )
        
        # Timer for processing
        self.create_timer(0.05, self.process_timer_callback)
        
        self.get_logger().info('Garbage Identify Node initialized')

    def get_garbage_category(self, name: str) -> int:
        """
        Determine garbage category from name.
        
        Args:
            name: Detected garbage name
            
        Returns:
            Category number (1=recyclable, 2=toxic, 3=wet, 4=dry, 0=unknown)
        """
        name_lower = name.lower()
        
        if name_lower in [n.lower() for n in RECYCLABLE_WASTE]:
            return 1  # Red bin - Recyclable
        elif name_lower in [n.lower() for n in TOXIC_WASTE]:
            return 2  # Green bin - Toxic
        elif name_lower in [n.lower() for n in WET_WASTE]:
            return 3  # Blue bin - Wet waste
        elif name_lower in [n.lower() for n in DRY_WASTE]:
            return 4  # Yellow bin - Dry waste
        else:
            return 0  # Unknown

    def detect_msg_callback(self, msg: String):
        """Callback for detection messages."""
        if msg.data:
            self.name = msg.data
            self.get_logger().info(f'Detected garbage: {self.name}')
            self.process_garbage()

    def process_timer_callback(self):
        """Timer callback for continuous processing."""
        # Continue processing if in mid-transport
        if self.garbage_transbot.model != "init_point" and \
           self.garbage_transbot.model != "Grip":
            category = self.get_garbage_category(self.name) if self.name else 0
            if category > 0:
                self.garbage_transbot.process(category)

    def process_garbage(self):
        """Process detected garbage."""
        if self.name is None:
            return
            
        category = self.get_garbage_category(self.name)
        
        if category == 0:
            self.get_logger().warn(f'Unknown garbage type: {self.name}')
            return
        
        category_names = {1: 'Recyclable', 2: 'Toxic', 3: 'Wet', 4: 'Dry'}
        self.get_logger().info(
            f'Garbage "{self.name}" classified as: {category_names[category]}'
        )
        
        # Start transport process
        self.garbage_transbot.model = "Grip"
        
        # Process until complete
        while self.garbage_transbot.model != "init_point":
            self.garbage_transbot.process(category)
            sleep(0.1)
            
            # Allow ROS2 to process callbacks
            rclpy.spin_once(self, timeout_sec=0.01)
        
        self.get_logger().info('Transport complete, resetting')
        self.name = None
        self.garbage_transbot.Reset()


def main(args=None):
    rclpy.init(args=args)
    node = GarbageIdentifyNode()
    
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

