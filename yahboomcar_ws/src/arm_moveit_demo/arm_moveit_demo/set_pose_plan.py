#!/usr/bin/env python3
# coding: utf-8
"""
Set Pose Plan Demo for X3plus Arm using MoveIt2
Plans and executes motion to a target pose (position + orientation).
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
    PositionConstraint,
    OrientationConstraint,
    BoundingVolume,
)
from geometry_msgs.msg import Pose, PoseStamped
from shape_msgs.msg import SolidPrimitive
from tf_transformations import quaternion_from_euler

# Degree to radian conversion
DE2RA = math.pi / 180.0


class SetPosePlanNode(Node):
    """Node that moves the arm to a target pose using MoveIt2."""

    def __init__(self):
        super().__init__('set_pose_plan_node')
        
        # MoveGroup action client
        self._action_client = ActionClient(self, MoveGroup, 'move_action')
        
        # Planning parameters
        self.planning_group = 'arm_group'
        self.end_effector_link = 'arm_link5'
        self.reference_frame = 'base_link'
        self.planning_time = 5.0
        self.num_planning_attempts = 10
        self.goal_position_tolerance = 0.01
        self.goal_orientation_tolerance = 0.01
        self.velocity_scaling = 1.0
        self.acceleration_scaling = 1.0
        
        self.get_logger().info('Set Pose Plan Node initialized')
        self.get_logger().info('Waiting for MoveGroup action server...')
        
        if not self._action_client.wait_for_server(timeout_sec=10.0):
            self.get_logger().error('MoveGroup action server not available!')
            return

        self.get_logger().info('MoveGroup action server available')

    def create_pose_goal(self, target_pose):
        """Create a MoveGroup goal for the given target pose."""
        goal = MoveGroup.Goal()
        
        # Motion plan request
        goal.request.group_name = self.planning_group
        goal.request.num_planning_attempts = self.num_planning_attempts
        goal.request.allowed_planning_time = self.planning_time
        goal.request.max_velocity_scaling_factor = self.velocity_scaling
        goal.request.max_acceleration_scaling_factor = self.acceleration_scaling
        
        # Goal constraints - position
        constraints = Constraints()
        
        # Position constraint
        position_constraint = PositionConstraint()
        position_constraint.header.frame_id = self.reference_frame
        position_constraint.link_name = self.end_effector_link
        position_constraint.target_point_offset.x = 0.0
        position_constraint.target_point_offset.y = 0.0
        position_constraint.target_point_offset.z = 0.0
        
        # Bounding volume (sphere around target)
        bounding_volume = BoundingVolume()
        primitive = SolidPrimitive()
        primitive.type = SolidPrimitive.SPHERE
        primitive.dimensions = [self.goal_position_tolerance]
        bounding_volume.primitives.append(primitive)
        
        primitive_pose = Pose()
        primitive_pose.position = target_pose.position
        primitive_pose.orientation.w = 1.0
        bounding_volume.primitive_poses.append(primitive_pose)
        
        position_constraint.constraint_region = bounding_volume
        position_constraint.weight = 1.0
        constraints.position_constraints.append(position_constraint)
        
        # Orientation constraint
        orientation_constraint = OrientationConstraint()
        orientation_constraint.header.frame_id = self.reference_frame
        orientation_constraint.link_name = self.end_effector_link
        orientation_constraint.orientation = target_pose.orientation
        orientation_constraint.absolute_x_axis_tolerance = self.goal_orientation_tolerance
        orientation_constraint.absolute_y_axis_tolerance = self.goal_orientation_tolerance
        orientation_constraint.absolute_z_axis_tolerance = self.goal_orientation_tolerance
        orientation_constraint.weight = 1.0
        constraints.orientation_constraints.append(orientation_constraint)
        
        goal.request.goal_constraints.append(constraints)
        
        # Planning options
        goal.planning_options.plan_only = False
        goal.planning_options.replan = True
        goal.planning_options.replan_attempts = 5
        
        return goal

    def send_pose_goal(self, target_pose):
        """Send a pose goal to the MoveGroup action server."""
        goal = self.create_pose_goal(target_pose)
        
        self.get_logger().info(
            f'Sending pose goal: pos=({target_pose.position.x:.3f}, '
            f'{target_pose.position.y:.3f}, {target_pose.position.z:.3f})'
        )
        
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
        """Main execution - move to target pose."""
        self.get_logger().info('Starting set pose plan demo...')
        
        # Create target pose
        target_pose = Pose()
        target_pose.position.x = 0.15
        target_pose.position.y = 0.0
        target_pose.position.z = 0.2
        
        # Set orientation using RPY (roll, pitch, yaw) -> quaternion
        roll = 0.0
        pitch = 45.0  # degrees
        yaw = 0.0
        q = quaternion_from_euler(
            roll * DE2RA, 
            pitch * DE2RA, 
            yaw * DE2RA
        )
        target_pose.orientation.x = q[0]
        target_pose.orientation.y = q[1]
        target_pose.orientation.z = q[2]
        target_pose.orientation.w = q[3]
        
        # Try multiple times
        for i in range(5):
            self.get_logger().info(f'Planning attempt {i+1}/5')
            success = self.send_pose_goal(target_pose)
            if success:
                break
            sleep(0.5)
        
        self.get_logger().info('Set pose plan demo completed')


def main(args=None):
    rclpy.init(args=args)
    node = SetPosePlanNode()
    
    try:
        node.run()
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()

