#!/usr/bin/env python3
# coding: utf-8
"""
Random Move Demo for X3plus Arm using MoveIt2
Moves the arm to random target positions continuously.
"""

import rclpy
from rclpy.node import Node
from rclpy.action import ActionClient
from time import sleep
import sys

from moveit_msgs.action import MoveGroup
from moveit_msgs.msg import (
    MotionPlanRequest,
    PlanningOptions,
    Constraints,
    JointConstraint,
    RobotState,
)
from sensor_msgs.msg import JointState
from std_msgs.msg import Header
import random
import math


class RandomMoveNode(Node):
    """Node that moves the arm to random positions using MoveIt2."""

    def __init__(self):
        super().__init__('random_move_node')
        
        # MoveGroup action client
        self._action_client = ActionClient(self, MoveGroup, 'move_action')
        
        # Joint names for the arm
        self.joint_names = [
            'arm_joint1', 'arm_joint2', 'arm_joint3', 
            'arm_joint4', 'arm_joint5'
        ]
        
        # Joint limits (radians) - approximately ±1.57 rad (90 degrees)
        self.joint_limits = {
            'arm_joint1': (-1.57, 1.57),
            'arm_joint2': (-1.57, 1.57),
            'arm_joint3': (-1.57, 1.57),
            'arm_joint4': (-1.57, 1.57),
            'arm_joint5': (-1.57, 1.57),
        }
        
        # Planning parameters
        self.planning_group = 'arm_group'
        self.planning_time = 5.0
        self.num_planning_attempts = 10
        self.goal_tolerance = 0.01
        self.velocity_scaling = 1.0
        self.acceleration_scaling = 1.0
        
        self.get_logger().info('Random Move Node initialized')
        self.get_logger().info('Waiting for MoveGroup action server...')
        
        if not self._action_client.wait_for_server(timeout_sec=10.0):
            self.get_logger().error('MoveGroup action server not available!')
            self.get_logger().info(
                'Please ensure MoveIt2 is running: '
                'ros2 launch x3plus_moveit_config demo.launch.py'
            )
            return

        self.get_logger().info('MoveGroup action server available')

    def get_random_joint_values(self):
        """Generate random joint values within limits."""
        joint_values = []
        for joint in self.joint_names:
            low, high = self.joint_limits[joint]
            value = random.uniform(low, high)
            joint_values.append(value)
        return joint_values

    def create_motion_plan_request(self, joint_values):
        """Create a MoveGroup goal for the given joint values."""
        goal = MoveGroup.Goal()
        
        # Motion plan request
        goal.request.group_name = self.planning_group
        goal.request.num_planning_attempts = self.num_planning_attempts
        goal.request.allowed_planning_time = self.planning_time
        goal.request.max_velocity_scaling_factor = self.velocity_scaling
        goal.request.max_acceleration_scaling_factor = self.acceleration_scaling
        
        # Goal constraints
        constraints = Constraints()
        for i, joint_name in enumerate(self.joint_names):
            joint_constraint = JointConstraint()
            joint_constraint.joint_name = joint_name
            joint_constraint.position = joint_values[i]
            joint_constraint.tolerance_above = self.goal_tolerance
            joint_constraint.tolerance_below = self.goal_tolerance
            joint_constraint.weight = 1.0
            constraints.joint_constraints.append(joint_constraint)
        
        goal.request.goal_constraints.append(constraints)
        
        # Planning options
        goal.planning_options.plan_only = False
        goal.planning_options.replan = True
        goal.planning_options.replan_attempts = 5
        
        return goal

    def send_goal(self, joint_values):
        """Send a goal to the MoveGroup action server."""
        goal = self.create_motion_plan_request(joint_values)
        
        self.get_logger().info(f'Sending goal: {joint_values}')
        
        future = self._action_client.send_goal_async(goal)
        rclpy.spin_until_future_complete(self, future)
        
        goal_handle = future.result()
        if not goal_handle.accepted:
            self.get_logger().warn('Goal rejected')
            return False
        
        self.get_logger().info('Goal accepted, waiting for result...')
        
        result_future = goal_handle.get_result_async()
        rclpy.spin_until_future_complete(self, result_future)
        
        result = result_future.result()
        if result.result.error_code.val == 1:  # SUCCESS
            self.get_logger().info('Motion executed successfully!')
            return True
        else:
            self.get_logger().warn(
                f'Motion failed with error code: {result.result.error_code.val}'
            )
            return False

    def run(self):
        """Main loop - continuously move to random positions."""
        self.get_logger().info('Starting random move demo...')
        
        while rclpy.ok():
            # Generate random joint values
            joint_values = self.get_random_joint_values()
            
            # Send goal
            success = self.send_goal(joint_values)
            
            # Wait before next move
            sleep(0.5)


def main(args=None):
    rclpy.init(args=args)
    node = RandomMoveNode()
    
    try:
        node.run()
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()

