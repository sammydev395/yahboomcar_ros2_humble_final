#!/usr/bin/env python3
# encoding: utf-8
"""
Garbage Transport Library for ROS2.
Handles the garbage gripping and transport state machine.
"""

import threading
from time import sleep
from .transport_common import ROSNav


class GarbageTransport:
    """
    State machine for garbage transport operations.
    Manages gripping, navigation, and drop-off sequences.
    """

    def __init__(self, ros_nav: ROSNav):
        self.ros_nav = ros_nav
        self.model = "Grip"
        self.Grip_status = True
        self.color_name = {}
        self.index = 0
        self.joints = []
        self.point = 0

    def process(self, point: int):
        """
        Process the garbage transport state machine.
        
        Args:
            point: Target bin (1=red, 2=green, 3=blue, 4=yellow)
        """
        self.point = point
        
        if self.model == "Grip":
            if self.Grip_status is True:
                threading.Thread(target=self.Grip_Target).start()
                
        elif self.model == "Transport":
            if self.ros_nav.Transport_status is True:
                color_map = {1: 'red', 2: 'green', 3: 'blue', 4: 'yellow'}
                if point in color_map and color_map[point] in self.ros_nav.color_pose:
                    self.ros_nav.PubTargetPoint(
                        self.ros_nav.color_pose[color_map[point]]
                    )
                
                self.model = "Grip_down"
                self.ros_nav.goal_result = 0
                self.buzzer_loop()
                
        elif self.model == "Grip_down":
            if self.ros_nav.goal_result == 3:
                threading.Thread(target=self.Grip_down).start()
                
        elif self.model == "come_back":
            if self.ros_nav.goal_result == 3:
                self.buzzer_loop()
                sleep(1)
                self.Reset()
                self.model = "init_point"

    def comeback(self):
        """Navigate back to start position."""
        self.ros_nav.PubTargetPoint(self.ros_nav.start_point)
        self.model = "come_back"
        if self.ros_nav.goal_result == 3:
            self.buzzer_loop()

    def Reset(self):
        """Reset all states to initial values."""
        self.ros_nav.goal_result = 0
        self.model = "Grip"
        self.Grip_status = True
        self.ros_nav.Transport_status = False
        self.color_name = {}
        self.index = 0

    def Grip_down(self):
        """Execute the drop-off sequence."""
        self.model = "Grip_down"
        self.ros_nav.goal_result = 0
        
        # Lower arm to drop position
        self.joints = [90.0, 2.0, 60.0, 40.0, 90.0, 140.0]
        self.ros_nav.pubArm(self.joints, run_time=1000)
        sleep(1)
        
        # Open gripper
        self.ros_nav.pubArm([], 6, 30)
        sleep(0.5)
        
        # Return to neutral position
        self.joints = [90.0, 145.0, 0.0, 45.0, 90.0, 30.0]
        self.ros_nav.pubArm(self.joints, run_time=1000)
        sleep(1)
        
        self.buzzer_loop()
        self.comeback()

    def Grip_Target(self):
        """Execute the grip sequence."""
        self.model = "Grip_Target"
        self.Grip_status = True
        self.buzzer_loop()
        
        # Move arm to grip position
        self.joints = [90.0, 145.0, 0.0, 45.0, 90.0, 30.0]
        self.ros_nav.pubArm(self.joints, run_time=1000)
        sleep(0.5)
        
        # Close gripper
        self.ros_nav.pubArm([], 6, 149)
        self.ros_nav.Transport_status = True
        sleep(1)
        
        self.buzzer_loop()
        self.model = "Transport"

    def buzzer_loop(self):
        """Sound buzzer once."""
        self.ros_nav.pubBuzzer(True)
        sleep(1)
        self.ros_nav.pubBuzzer(False)
        sleep(1)

