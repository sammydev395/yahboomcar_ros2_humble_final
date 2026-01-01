#!/usr/bin/env python3
# coding: utf-8
"""
Simulation to Machine Bridge for X3plus Arm
Subscribes to MoveIt2 joint states and publishes to the real arm via ArmJoint messages.
"""

import rclpy
from rclpy.node import Node
import numpy as np
import math

from sensor_msgs.msg import JointState
from yahboomcar_msgs.msg import ArmJoint


class SimulationToMachineNode(Node):
    """
    Bridge node that converts MoveIt2 simulated joint states
    to real robot arm commands.
    """

    def __init__(self):
        super().__init__('simulation_to_machine_node')
        
        # Initial joint positions (degrees) - neutral position
        self.joints = [90.0, 90.0, 90.0, 90.0, 90.0, 30.0]
        
        # Publisher for arm commands
        self.pub_arm = self.create_publisher(ArmJoint, 'TargetAngle', 10)
        
        # Subscriber for MoveIt2 joint states
        # In MoveIt2, the topic may vary based on controller configuration
        self.create_subscription(
            JointState,
            '/joint_states',
            self.joint_state_callback,
            10
        )
        
        # Also subscribe to the fake controller topic if using simulation
        self.create_subscription(
            JointState,
            '/move_group/fake_controller_joint_states',
            self.joint_state_callback,
            10
        )
        
        # Publish initial position
        self.create_timer(0.1, self.initial_publish_once)
        self._initial_published = False
        
        self.get_logger().info('Simulation to Machine Node initialized')
        self.get_logger().info('Listening for joint states from MoveIt2...')

    def initial_publish_once(self):
        """Publish initial position once."""
        if not self._initial_published:
            self.pub_arm_joints(self.joints)
            self._initial_published = True

    def joint_state_callback(self, msg: JointState):
        """
        Callback for joint state messages.
        Converts radians to degrees and maps to arm servo range.
        """
        if len(msg.position) == 0:
            return
        
        arm_rad = np.array(msg.position)
        RAD2DEG = 180.0 / math.pi
        arm_deg = arm_rad * RAD2DEG
        
        if len(msg.position) == 5:
            # 5 arm joints
            mid = np.array([90, 90, 90, 90, 90])
            arm_array = arm_deg + mid
            for i in range(5):
                self.joints[i] = float(arm_array[i])
                
        elif len(msg.position) == 1:
            # Gripper joint only
            # arm_deg: -88~0 -> arm_array: 91~180
            arm_array = arm_deg + np.array([180])
            self.joints[5] = float(np.interp(arm_array[0], [90, 180], [30, 180]))
            
        elif len(msg.position) == 6:
            # All joints including gripper
            mid = np.array([90, 90, 90, 90, 90, 0])
            arm_array = arm_deg + mid
            for i in range(5):
                self.joints[i] = float(arm_array[i])
            # Map gripper
            gripper_val = arm_array[5] + 180
            self.joints[5] = float(np.interp(gripper_val, [90, 180], [30, 180]))
        
        self.pub_arm_joints(self.joints)

    def pub_arm_joints(self, joints, run_time=1000):
        """Publish arm joint command."""
        arm_joint = ArmJoint()
        arm_joint.joints = [float(j) for j in joints]
        arm_joint.run_time = run_time
        self.pub_arm.publish(arm_joint)
        
        self.get_logger().debug(
            f'Published joints: {[f"{j:.1f}" for j in joints]}'
        )


def main(args=None):
    rclpy.init(args=args)
    node = SimulationToMachineNode()
    
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()

