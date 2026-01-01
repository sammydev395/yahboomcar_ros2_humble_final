#!/usr/bin/env python3
# coding: utf-8
"""
Set Joint Plan Demo for X3plus Arm using MoveIt2
Plans and executes motion to target joint positions.
"""

import rclpy
from rclpy.node import Node
from rclpy.action import ActionClient
from time import sleep
import math

from moveit_msgs.action import MoveGroup
from moveit_msgs.msg import (
    MotionPlanRequest,
    PlanningOptions,
    Constraints,
    JointConstraint,
)

# Degree to radian conversion
DE2RA = math.pi / 180.0


class SetJointPlanNode(Node):
    """Node that moves the arm to target joint positions using MoveIt2."""

    def __init__(self):
        super().__init__('set_joint_plan_node')
        
        # MoveGroup action client
        self._action_client = ActionClient(self, MoveGroup, 'move_action')
        
        # Joint names for the arm
        self.joint_names = [
            'arm_joint1', 'arm_joint2', 'arm_joint3', 
            'arm_joint4', 'arm_joint5'
        ]
        
        # Planning parameters
        self.planning_group = 'arm_group'
        self.planning_time = 5.0
        self.num_planning_attempts = 10
        self.goal_joint_tolerance = 0.001
        self.velocity_scaling = 1.0
        self.acceleration_scaling = 1.0
        
        self.get_logger().info('Set Joint Plan Node initialized')
        self.get_logger().info('Waiting for MoveGroup action server...')
        
        if not self._action_client.wait_for_server(timeout_sec=10.0):
            self.get_logger().error('MoveGroup action server not available!')
            return

        self.get_logger().info('MoveGroup action server available')

    def create_joint_goal(self, joint_values):
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
            joint_constraint.tolerance_above = self.goal_joint_tolerance
            joint_constraint.tolerance_below = self.goal_joint_tolerance
            joint_constraint.weight = 1.0
            constraints.joint_constraints.append(joint_constraint)
        
        goal.request.goal_constraints.append(constraints)
        
        # Planning options
        goal.planning_options.plan_only = False
        goal.planning_options.replan = True
        goal.planning_options.replan_attempts = 5
        
        return goal

    def send_joint_goal(self, joint_values):
        """Send a joint goal to the MoveGroup action server."""
        goal = self.create_joint_goal(joint_values)
        
        self.get_logger().info(f'Sending joint goal: {joint_values}')
        
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

    def send_named_target(self, name):
        """Send a named target (e.g., 'up', 'down')."""
        named_targets = {
            'up': [0.0, 0.0, 0.0, 0.0, 0.0],
            'down': [0.0, -1.5708, 0.0, 0.0, 0.0],
            'int': [0.0, 0.7854, -1.5708, -1.5708, 0.0],
        }
        
        if name not in named_targets:
            self.get_logger().error(f'Unknown named target: {name}')
            return False
        
        self.get_logger().info(f'Moving to named target: {name}')
        return self.send_joint_goal(named_targets[name])

    def run(self):
        """Main execution - move to target joint positions."""
        self.get_logger().info('Starting set joint plan demo...')
        
        # First, go to 'down' position
        self.send_named_target('down')
        sleep(0.5)
        
        # Target joint values (radians): [0, 0.79, -1.57, -1.57, 0]
        target_joints = [0.0, 0.79, -1.57, -1.57, 0.0]
        
        # Try multiple times
        for i in range(5):
            self.get_logger().info(f'Planning attempt {i+1}/5')
            success = self.send_joint_goal(target_joints)
            if success:
                break
            sleep(0.5)
        
        self.get_logger().info('Set joint plan demo completed')


def main(args=None):
    rclpy.init(args=args)
    node = SetJointPlanNode()
    
    try:
        node.run()
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()

