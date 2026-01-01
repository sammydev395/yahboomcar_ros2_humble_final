#!/usr/bin/env python3
# encoding: utf-8
"""
Arm Autopilot Main Node - ROS2 Version
Migrated from ROS1 to ROS2 Humble

This node provides color-based object detection and 6-DOF arm control
for picking up colored objects (red, green, blue, yellow).
"""
import os
import threading
from time import sleep
import time

import cv2 as cv
import numpy as np

import rclpy
from rclpy.node import Node
from rcl_interfaces.msg import ParameterDescriptor, ParameterType
from rcl_interfaces.msg import SetParametersResult

from arm_autopilot.autopilot_common import (
    ROSCtrl, ColorFollow, HSVYaml, SimplePID, ManyImgs, linear
)


class LineDetectNode(Node):
    """ROS2 Node for line detection and arm autopilot control"""
    
    def __init__(self):
        super().__init__('line_detect')
        
        self.get_logger().info('Initializing Arm Autopilot Node (ROS2)...')
        
        # Initialize components
        self.ros_ctrl = ROSCtrl(self)
        self.color = ColorFollow()
        self.hsv_yaml = HSVYaml()
        
        # State variables
        self.dyn_update = False
        self.Calibration = False
        self.select_flags = False
        self.gripper_state = False
        self.location_state = False
        self.Track_state = 'identify'
        self.windows_name = 'frame'
        self.color.target_color_name = 'red'
        self.color_name_list = ['red', 'green', 'blue', 'yellow']
        self.hsv_value = ()
        self.color_cfg_src = self.index = self.cols = self.rows = 0
        self.Mouse_XY = (0, 0)
        self.Roi_init = ()
        
        # Declare ROS2 parameters (replaces dynamic_reconfigure)
        self._declare_parameters()
        
        # Add parameter callback
        self.add_on_set_parameters_callback(self.parameter_callback)
        
        # PID and control settings
        self.scale = 1000.0
        self.FollowLinePID = (30.0, 0.0, 60.0)
        self.linear = 0.10
        self.PID_init()
        
        # Initial arm position
        self.joints_init = [90, 120, 0, 0, 90, 30]
        
        # Load HSV values for all colors
        for i in range(4):
            self.color.color_hsv_list[self.color_name_list[i]] = self.hsv_yaml.read_hsv(self.color_name_list[i])
        
        # Setup OpenCV window
        cv.namedWindow(self.windows_name, cv.WINDOW_AUTOSIZE)
        cv.setMouseCallback(self.windows_name, self.onMouse, 0)
        
        # Move arm to initial position
        self.ros_ctrl.pubArm(self.joints_init)
        
        self.get_logger().info('Arm Autopilot Node initialized successfully!')

    def _declare_parameters(self):
        """Declare all ROS2 parameters (replaces dynamic_reconfigure)"""
        # Calibration mode
        self.declare_parameter('Calibration', False, 
            ParameterDescriptor(description='Color calibration mode'))
        
        # Color selection (0=red, 1=green, 2=blue, 3=yellow)
        self.declare_parameter('Color', 0,
            ParameterDescriptor(description='Target color: 0=red, 1=green, 2=blue, 3=yellow'))
        
        # HSV parameters
        self.declare_parameter('Hmin', 0, ParameterDescriptor(description='H min in HSV'))
        self.declare_parameter('Hmax', 9, ParameterDescriptor(description='H max in HSV'))
        self.declare_parameter('Smin', 85, ParameterDescriptor(description='S min in HSV'))
        self.declare_parameter('Smax', 253, ParameterDescriptor(description='S max in HSV'))
        self.declare_parameter('Vmin', 126, ParameterDescriptor(description='V min in HSV'))
        self.declare_parameter('Vmax', 255, ParameterDescriptor(description='V max in HSV'))
        
        # PID parameters
        self.declare_parameter('scale', 1000, ParameterDescriptor(description='PID scale factor'))
        self.declare_parameter('Kp', 30.0, ParameterDescriptor(description='PID Kp'))
        self.declare_parameter('Ki', 0.0, ParameterDescriptor(description='PID Ki'))
        self.declare_parameter('Kd', 60.0, ParameterDescriptor(description='PID Kd'))
        
        # Motion parameters
        self.declare_parameter('linear', 0.25, ParameterDescriptor(description='Linear velocity'))
        self.declare_parameter('LaserAngle', 30, ParameterDescriptor(description='Laser angle for obstacle detection'))
        self.declare_parameter('ResponseDist', 0.55, ParameterDescriptor(description='Response distance'))

    def parameter_callback(self, params):
        """Handle parameter changes (replaces dynamic_reconfigure callback)"""
        for param in params:
            if param.name == 'scale':
                self.scale = float(param.value)
            elif param.name == 'linear':
                self.linear = float(param.value)
            elif param.name == 'LaserAngle':
                self.ros_ctrl.LaserAngle = int(param.value)
            elif param.name == 'ResponseDist':
                self.ros_ctrl.ResponseDist = float(param.value)
            elif param.name == 'Kp':
                self.FollowLinePID = (float(param.value), self.FollowLinePID[1], self.FollowLinePID[2])
                self.PID_init()
            elif param.name == 'Ki':
                self.FollowLinePID = (self.FollowLinePID[0], float(param.value), self.FollowLinePID[2])
                self.PID_init()
            elif param.name == 'Kd':
                self.FollowLinePID = (self.FollowLinePID[0], self.FollowLinePID[1], float(param.value))
                self.PID_init()
            elif param.name == 'Calibration':
                self.Calibration = bool(param.value)
            elif param.name == 'Color':
                color_idx = int(param.value)
                if 0 <= color_idx < len(self.color_name_list):
                    if self.color_cfg_src != color_idx:
                        self.color.target_color_name = self.color_name_list[color_idx]
                        self.hsv_value = self.hsv_yaml.read_hsv(self.color.target_color_name)
                        self.color_cfg_src = color_idx
            elif param.name in ['Hmin', 'Hmax', 'Smin', 'Smax', 'Vmin', 'Vmax']:
                if self.Track_state != 'mouse':
                    hmin = self.get_parameter('Hmin').value
                    hmax = self.get_parameter('Hmax').value
                    smin = self.get_parameter('Smin').value
                    smax = self.get_parameter('Smax').value
                    vmin = self.get_parameter('Vmin').value
                    vmax = self.get_parameter('Vmax').value
                    self.hsv_value = ((hmin, smin, vmin), (hmax, smax, vmax))
                    self.hsv_yaml.write_hsv(self.color.target_color_name, self.hsv_value)
                    self.color.color_hsv_list[self.color.target_color_name] = self.hsv_value
                else:
                    self.Track_state = 'identify'
        
        return SetParametersResult(successful=True)

    def process(self, rgb_img, action):
        """Process camera frame and control robot/arm"""
        if action == 32 or self.ros_ctrl.joy_action == 2:
            self.Track_state = 'tracking'
            self.Calibration = False
            self.dyn_update = True
            self.ros_ctrl.pubArm(self.joints_init)
        elif action == ord('r') or action == ord('R'):
            self.Reset()
        elif action == ord('q') or action == ord('Q'):
            self.cancel()
        elif action == ord('c') or action == ord('C'):
            self.Calibration = not self.Calibration
            self.dyn_update = True
        elif action == ord('i') or action == ord('I'):
            self.Track_state = "identify"
            self.Calibration = False
            self.dyn_update = True
        elif action == ord('f') or action == ord('F'):
            color_index = self.color_name_list.index(self.color.target_color_name)
            if color_index >= 3:
                color_index = 0
            else:
                color_index += 1
            self.color.target_color_name = self.color_name_list[color_index]
            self.hsv_value = self.hsv_yaml.read_hsv(self.color.target_color_name)
            self.dyn_update = True
        
        if self.Track_state == 'init':
            cv.setMouseCallback(self.windows_name, self.onMouse, 0)
            if self.select_flags:
                cv.line(rgb_img, self.cols, self.rows, (255, 0, 0), 2)
                cv.rectangle(rgb_img, self.cols, self.rows, (0, 255, 0), 2)
                if self.Roi_init[0] != self.Roi_init[2] and self.Roi_init[1] != self.Roi_init[3]:
                    rgb_img, self.hsv_value = self.color.Roi_hsv(rgb_img, self.Roi_init)
                    self.color.color_hsv_list[self.color.target_color_name] = self.hsv_value
                    self.hsv_yaml.write_hsv(self.color.target_color_name, self.hsv_value)
                    self.dyn_update = True
                else:
                    self.Track_state = 'init'
        
        if self.Track_state != 'init' and len(self.hsv_value) != 0:
            if self.Calibration:
                self.color.msg_box = {}
                self.color.line_follow(rgb_img, self.color.target_color_name, self.hsv_value)
            else:
                for i in range(len(self.color_name_list)):
                    threading.Thread(
                        target=self.color.line_follow,
                        args=(rgb_img, self.color_name_list[i],
                              self.color.color_hsv_list[self.color_name_list[i]],)
                    ).start()
        
        if (self.Track_state == 'tracking' and len(self.color.msg_circle) != 0 and
                not self.ros_ctrl.Joy_active and not self.gripper_state):
            for i in self.color.msg_circle.keys():
                if i == self.color.target_color_name and not self.location_state:
                    threading.Thread(
                        target=self.execute,
                        args=(self.color.msg_circle[self.color.target_color_name],)
                    ).start()
            for i in self.color.msg_box.keys():
                if i != self.color.target_color_name and len(self.color.msg_box) != 0 and len(self.color.msg_box[i]) != 0:
                    (point_x, point_y), _ = cv.minEnclosingCircle(self.color.msg_box[i])
                    threading.Thread(target=self.Wrecker, args=(point_x, point_y,)).start()
                else:
                    self.index += 1
                    if self.index >= 20:
                        self.location_state = False
        else:
            if self.ros_ctrl.RobotRun_status:
                self.ros_ctrl.pubVel(0, 0)
        
        if self.dyn_update:
            self.dyn_cfg_update()
        
        return self.color.binary

    def Wrecker(self, point_x, point_y):
        """Handle non-target object - move robot and grab with arm"""
        self.index = 0
        self.location_state = True
        if self.ros_ctrl.Buzzer_state:
            self.ros_ctrl.pubBuzzer(False)
        if abs(point_x - 320) < 40:
            point_x = 320

        self.get_logger().debug(f"cur_y: {point_y}")
        if abs(point_x - 320) < 10 and point_y > 440:
            self.get_logger().info(f"point_x: {point_x}, point_y: {point_y}")
            self.get_logger().info("arm down now")
            if self.ros_ctrl.RobotRun_status:
                self.ros_ctrl.pubVel(0, 0)
            self.gripper_state = True
            sleep(0.3)
            np_array = np.array([linear([320, 90], [343.5, 95])])
            pos1 = np.dot(np_array, np.array([point_x, 1])).squeeze().tolist()
            joints = [pos1, 7.0, 60.0, 38.0, 90]
            if len(joints) != 0:
                self.arm_gripper(joints)
            self.color.msg_box = {}
            self.color.msg_circle = {}
            sleep(2.5)
            self.gripper_state = False
            self.location_state = False
        else:
            self.robot_location(point_x, point_y)

    def robot_location(self, point_x, point_y):
        """Control robot movement to target location"""
        [y, x] = self.PID_controller.update([(point_x - 320) / 10.0, (point_y - 440) / 10.0])
        if x >= 0.10:
            x = 0.10
        elif x <= -0.10:
            x = -0.10
        if y >= 0.10:
            y = 0.10
        elif y <= -0.10:
            y = -0.10
        self.ros_ctrl.pubVel(x, y)
        self.ros_ctrl.RobotRun_status = True

    def arm_gripper(self, joints):
        """Execute arm gripper sequence to pick up object"""
        joints.append(30)
        self.ros_ctrl.pubArm(joints, run_time=8000)
        sleep(2)
        self.ros_ctrl.pubArm([], id=6, angle=150)
        sleep(0.5)
        self.ros_ctrl.pubArm([], id=2, angle=60, run_time=1000)
        sleep(1)
        self.ros_ctrl.pubArm([], id=1, angle=0, run_time=1000)
        sleep(1)
        joints[0] = 0
        joints[5] = 140
        self.ros_ctrl.pubArm(joints, run_time=1000)
        sleep(1)
        self.ros_ctrl.pubArm([], id=6, angle=30)
        sleep(0.5)
        self.ros_ctrl.pubArm([], id=2, angle=90, run_time=1000)
        sleep(1)
        self.ros_ctrl.pubArm([90, 120, 0, 0, 90, 30], run_time=2000)

    def execute(self, circle):
        """Execute line following with obstacle avoidance"""
        self.index = 0
        if len(circle) == 0:
            self.ros_ctrl.pubVel(0, 0)
        else:
            if self.ros_ctrl.warning > 10:
                self.get_logger().warning("Obstacles ahead !!!")
                self.ros_ctrl.pubVel(0, 0)
                self.ros_ctrl.pubBuzzer(True)
                self.ros_ctrl.Buzzer_state = True
            else:
                [z_Pid, _] = self.PID_controller.update([(circle[0] - 320) / 16, 0])
                if self.ros_ctrl.img_flip:
                    z = -z_Pid
                else:
                    z = z_Pid
                x = self.linear
                if self.ros_ctrl.Buzzer_state:
                    self.ros_ctrl.pubBuzzer(False)
                self.ros_ctrl.pubVel(x, 0, z=z)
            self.ros_ctrl.RobotRun_status = True

    def dyn_cfg_update(self):
        """Update parameters based on current HSV values"""
        hsv = self.color.color_hsv_list[self.color.target_color_name]
        try:
            self.set_parameters([
                rclpy.parameter.Parameter('Calibration', rclpy.Parameter.Type.BOOL, self.Calibration),
                rclpy.parameter.Parameter('Color', rclpy.Parameter.Type.INTEGER, 
                                          self.color_name_list.index(self.color.target_color_name)),
                rclpy.parameter.Parameter('Hmin', rclpy.Parameter.Type.INTEGER, hsv[0][0]),
                rclpy.parameter.Parameter('Hmax', rclpy.Parameter.Type.INTEGER, hsv[1][0]),
                rclpy.parameter.Parameter('Smin', rclpy.Parameter.Type.INTEGER, hsv[0][1]),
                rclpy.parameter.Parameter('Smax', rclpy.Parameter.Type.INTEGER, hsv[1][1]),
                rclpy.parameter.Parameter('Vmin', rclpy.Parameter.Type.INTEGER, hsv[0][2]),
                rclpy.parameter.Parameter('Vmax', rclpy.Parameter.Type.INTEGER, hsv[1][2]),
            ])
        except Exception as e:
            self.get_logger().warning(f"Failed to update parameters: {e}")
        self.dyn_update = False

    def putText_img(self, frame):
        """Add text overlay to frame"""
        if self.Calibration:
            cv.putText(frame, "Calibration", (500, 30), cv.FONT_HERSHEY_SIMPLEX, 0.6, (100, 200, 200), 1)
        cv.putText(frame, self.color.target_color_name, (300, 30), cv.FONT_HERSHEY_SIMPLEX, 0.6, (100, 200, 200), 1)
        msg_index = len(self.color.msg_box.keys())
        if msg_index != 0:
            for i in self.color.msg_box.keys():
                try:
                    self.color.add_box(i)
                except Exception as e:
                    self.get_logger().debug(f"Error adding box: {e}")
        self.ros_ctrl.pubImg(frame)
        return frame

    def onMouse(self, event, x, y, flags, param):
        """Handle mouse events for ROI selection"""
        if x > 640 or y > 480:
            return
        if event == 1:
            self.Track_state = 'init'
            self.select_flags = True
            self.Calibration = True
            self.Mouse_XY = (x, y)
        if event == 4:
            self.select_flags = False
            self.Track_state = 'mouse'
        if self.select_flags:
            self.cols = min(self.Mouse_XY[0], x), min(self.Mouse_XY[1], y)
            self.rows = max(self.Mouse_XY[0], x), max(self.Mouse_XY[1], y)
            self.Roi_init = (self.cols[0], self.cols[1], self.rows[0], self.rows[1])

    def Reset(self):
        """Reset all states"""
        self.PID_init()
        self.color.binary = ()
        self.color.msg_box = {}
        self.Track_state = 'init'
        self.color.msg_circle = {}
        self.gripper_state = False
        self.ros_ctrl.Joy_active = False
        self.Mouse_XY = (0, 0)
        self.ros_ctrl.pubVel(0, 0)
        self.ros_ctrl.pubBuzzer(False)
        self.get_logger().info("Reset success!!!")

    def PID_init(self):
        """Initialize PID controller"""
        self.PID_controller = SimplePID(
            [0, 0],
            [self.FollowLinePID[0] / self.scale, self.FollowLinePID[0] / self.scale],
            [self.FollowLinePID[1] / self.scale, self.FollowLinePID[1] / self.scale],
            [self.FollowLinePID[2] / self.scale, self.FollowLinePID[2] / self.scale]
        )

    def cancel(self):
        """Shutdown handler"""
        self.Reset()
        self.ros_ctrl.cancel()
        self.get_logger().info("Shutting down this node.")


def main(args=None):
    rclpy.init(args=args)
    
    node = LineDetectNode()
    
    # Open camera
    capture = cv.VideoCapture('/dev/camera_usb')
    if not capture.isOpened():
        # Try default camera
        capture = cv.VideoCapture(0)
    
    cv_edition = cv.__version__
    if cv_edition[0] == '3':
        capture.set(cv.CAP_PROP_FOURCC, cv.VideoWriter_fourcc(*'XVID'))
    else:
        capture.set(cv.CAP_PROP_FOURCC, cv.VideoWriter.fourcc('M', 'J', 'P', 'G'))
    capture.set(cv.CAP_PROP_FRAME_WIDTH, 640)
    capture.set(cv.CAP_PROP_FRAME_HEIGHT, 480)
    
    try:
        while capture.isOpened() and rclpy.ok():
            start = time.time()
            ret, frame = capture.read()
            if not ret:
                node.get_logger().warning("Failed to read frame from camera")
                continue
            
            action = cv.waitKey(10) & 0xFF
            if node.ros_ctrl.img_flip:
                frame = cv.flip(frame, 1)
            node.color.frame = frame
            binary = node.process(frame, action)
            end = time.time()
            fps = 1 / (end - start) if (end - start) > 0 else 0
            text = "FPS : " + str(int(fps))
            cv.putText(frame, text, (30, 30), cv.FONT_HERSHEY_SIMPLEX, 0.6, (100, 200, 200), 1)
            frame = node.putText_img(frame)
            
            if len(binary) != 0:
                cv.imshow(node.windows_name, ManyImgs(1, ([frame, binary])))
            else:
                cv.imshow(node.windows_name, frame)
            
            if action == ord('q') or action == 113:
                break
            
            # Process ROS2 callbacks
            rclpy.spin_once(node, timeout_sec=0.001)
    
    except KeyboardInterrupt:
        pass
    finally:
        node.cancel()
        capture.release()
        cv.destroyAllWindows()
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()

