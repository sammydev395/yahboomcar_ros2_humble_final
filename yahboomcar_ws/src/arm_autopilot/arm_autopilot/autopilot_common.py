#!/usr/bin/env python3
# encoding: utf-8
"""
Arm Autopilot Common Utilities - ROS2 Version
Migrated from ROS1 to ROS2 Humble
"""
import threading
import yaml
import time
import math
import cv2 as cv
import numpy as np
from numpy import pi

import rclpy
from rclpy.node import Node
from cv_bridge import CvBridge
from ament_index_python.packages import get_package_share_directory

from yahboomcar_msgs.msg import ArmJoint
from geometry_msgs.msg import Twist
from std_msgs.msg import Bool, Int32
from sensor_msgs.msg import LaserScan, Image

RAD2DEG = 180 / pi


def linear(pos1, pos2):
    """
    Linear formula: y = ax + b
    Calculate a and b
    """
    x1, y1 = pos1
    x2, y2 = pos2
    a = (y2 - y1) / (x2 - x1)
    b = y1 - (y2 - y1) * x1 / (x2 - x1)
    return a, b


def cacl_oblique_angle(point1, point2):
    x1, y1 = point1
    x2, y2 = point2
    x = (x1 - x2)
    y = (y1 - y2)
    angle = np.arctan(x / y) * RAD2DEG
    return angle


def ManyImgs(scale, imgarray):
    """
    Display multiple images in a single window
    """
    rows = len(imgarray)
    cols = len(imgarray[0])
    rowsAvailable = isinstance(imgarray[0], list)
    width = imgarray[0][0].shape[1]
    height = imgarray[0][0].shape[0]
    
    if rowsAvailable:
        for x in range(0, rows):
            for y in range(0, cols):
                if imgarray[x][y].shape[:2] == imgarray[0][0].shape[:2]:
                    imgarray[x][y] = cv.resize(imgarray[x][y], (0, 0), None, scale, scale)
                else:
                    imgarray[x][y] = cv.resize(imgarray[x][y], (imgarray[0][0].shape[1], imgarray[0][0].shape[0]), None, scale, scale)
                if len(imgarray[x][y].shape) == 2:
                    imgarray[x][y] = cv.cvtColor(imgarray[x][y], cv.COLOR_GRAY2BGR)
        imgBlank = np.zeros((height, width, 3), np.uint8)
        hor = [imgBlank] * rows
        for x in range(0, rows):
            hor[x] = np.hstack(imgarray[x])
        ver = np.vstack(hor)
    else:
        for x in range(0, rows):
            if imgarray[x].shape[:2] == imgarray[0].shape[:2]:
                imgarray[x] = cv.resize(imgarray[x], (0, 0), None, scale, scale)
            else:
                imgarray[x] = cv.resize(imgarray[x], (imgarray[0].shape[1], imgarray[0].shape[0]), None, scale, scale)
            if len(imgarray[x].shape) == 2:
                imgarray[x] = cv.cvtColor(imgarray[x], cv.COLOR_GRAY2BGR)
        hor = np.hstack(imgarray)
        ver = hor
    return ver


class HSVYaml:
    def __init__(self, config_path=None):
        if config_path is None:
            try:
                pkg_share = get_package_share_directory('arm_autopilot')
                self.hsv_text = pkg_share + "/config/HSV.yaml"
            except Exception:
                # Fallback for development
                self.hsv_text = "/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws/src/arm_autopilot/config/HSV.yaml"
        else:
            self.hsv_text = config_path

    def write_hsv(self, color, hsv):
        try:
            with open(self.hsv_text, 'r') as f:
                result = yaml.safe_load(f)
                if result is None:
                    result = {}
                result[color] = hsv
            with open(self.hsv_text, 'w') as f:
                yaml.safe_dump(result, f)
        except Exception as e:
            print(f"Error writing HSV: {e}")

    def read_hsv(self, name):
        try:
            with open(self.hsv_text, 'r') as f:
                color_hsv = yaml.safe_load(f)
            if color_hsv and name in color_hsv:
                return color_hsv[name]
            else:
                # Return default values
                defaults = {
                    "red": ((0, 85, 126), (9, 253, 255)),
                    "green": ((55, 128, 146), (125, 253, 255)),
                    "blue": ((55, 232, 245), (125, 253, 255)),
                    "yellow": ((24, 98, 245), (125, 253, 255)),
                }
                return defaults.get(name, ((0, 0, 0), (180, 255, 255)))
        except Exception as e:
            print(f"Error reading HSV: {e}")
            return ((0, 0, 0), (180, 255, 255))


class ROSCtrl:
    """ROS2 Control class for robot and arm control"""
    
    def __init__(self, node: Node):
        self.node = node
        self.RobotRun_status = True
        self.Buzzer_state = False
        self.Joy_active = False
        self.LaserAngle = 30
        self.ResponseDist = 0.55
        self.warning = 1
        self.joy_action = 0
        self.bridge = CvBridge()
        
        # Declare and get parameters
        self.node.declare_parameter('img_flip', False)
        self.node.declare_parameter('VideoSwitch', False)
        self.img_flip = self.node.get_parameter('img_flip').get_parameter_value().bool_value
        self.VideoSwitch = self.node.get_parameter('VideoSwitch').get_parameter_value().bool_value
        
        # Subscribers
        self.sub_JoyState = self.node.create_subscription(
            Bool, '/JoyState', self.JoyStateCallback, 10)
        self.sub_scan = self.node.create_subscription(
            LaserScan, '/scan', self.registerScan, 1)
        self.sub_RGBLight = self.node.create_subscription(
            Int32, 'RGBLight', self.RGBLightcallback, 100)
        
        # Publishers
        self.pub_CmdVel = self.node.create_publisher(Twist, '/cmd_vel', 10)
        self.pub_buzzer = self.node.create_publisher(Bool, '/Buzzer', 1)
        self.pub_rgb = self.node.create_publisher(Image, '/linefollw/rgb', 1)
        self.pub_Arm = self.node.create_publisher(ArmJoint, 'TargetAngle', 1000)

    def RGBLightcallback(self, msg):
        if not isinstance(msg, Int32):
            return
        threading.Thread(target=self.joy_action_update).start()

    def joy_action_update(self):
        self.joy_action += 1
        time.sleep(0.5)
        self.joy_action = 0

    def pubArm(self, joints, id=10, angle=90, run_time=500):
        armjoint = ArmJoint()
        armjoint.run_time = run_time
        if len(joints) != 0:
            armjoint.joints = [float(j) for j in joints]
        else:
            armjoint.id = id
            armjoint.angle = float(angle)
        self.pub_Arm.publish(armjoint)

    def pubVel(self, x, y, z=0.0):
        twist = Twist()
        twist.linear.x = float(x)
        twist.linear.y = float(y)
        twist.angular.z = float(z)
        self.pub_CmdVel.publish(twist)
        self.RobotRun_status = False

    def pubImg(self, rgb_img):
        self.pub_rgb.publish(self.bridge.cv2_to_imgmsg(rgb_img, "bgr8"))

    def pubBuzzer(self, status):
        msg = Bool()
        msg.data = status
        self.pub_buzzer.publish(msg)
        self.Buzzer_state = False

    def JoyStateCallback(self, msg):
        if not isinstance(msg, Bool):
            return
        self.Joy_active = msg.data
        self.pubVel(0, 0)

    def cancel(self):
        self.pub_CmdVel.publish(Twist())

    def registerScan(self, scan_data):
        self.warning = 1
        if not isinstance(scan_data, LaserScan):
            return
        if self.Joy_active:
            return
        
        ranges = np.array(scan_data.ranges)
        for i in range(len(ranges)):
            angle = (scan_data.angle_min + scan_data.angle_increment * i) * RAD2DEG
            if abs(angle) > (180 - self.LaserAngle):
                if ranges[i] < self.ResponseDist:
                    self.warning += 1


class ColorFollow:
    """Color detection and following class"""
    
    def __init__(self):
        self.frame = None
        self.binary = ()
        self.Center_x = 0
        self.Center_y = 0
        self.Center_r = 0
        self.msg_box = {}
        self.msg_circle = {}
        self.target_color_name = 'red'
        self.color_hsv_list = {
            "red": ((176, 78, 65), (180, 253, 255)),
            "green": ((46, 37, 71), (99, 234, 255)),
            "blue": ((104, 137, 108), (127, 253, 255)),
            "yellow": ((30, 24, 139), (33, 158, 255)),
        }
        self.hsv_yaml = HSVYaml()

    def line_follow(self, rgb_img, color_name, hsv_msg):
        height, width = rgb_img.shape[:2]
        img = rgb_img.copy()
        if color_name == self.target_color_name:
            img[0:int(height / 2), 0:width] = 0
        
        hsv_img = cv.cvtColor(img, cv.COLOR_BGR2HSV)
        lower = np.array(hsv_msg[0], dtype="uint8")
        upper = np.array(hsv_msg[1], dtype="uint8")
        mask = cv.inRange(hsv_img, lower, upper)
        color_mask = cv.bitwise_and(hsv_img, hsv_img, mask=mask)
        gray_img = cv.cvtColor(color_mask, cv.COLOR_RGB2GRAY)
        kernel = cv.getStructuringElement(cv.MORPH_RECT, (5, 5))
        gray_img = cv.morphologyEx(gray_img, cv.MORPH_CLOSE, kernel)
        ret, binary = cv.threshold(gray_img, 10, 255, cv.THRESH_BINARY)
        
        find_contours = cv.findContours(binary, cv.RETR_EXTERNAL, cv.CHAIN_APPROX_SIMPLE)
        if len(find_contours) == 3:
            contours = find_contours[1]
        else:
            contours = find_contours[0]
        
        box = []
        circle = ()
        if len(contours) != 0:
            areas = []
            for c in range(len(contours)):
                areas.append(cv.contourArea(contours[c]))
            if max(areas) > 150:
                max_id = areas.index(max(areas))
                max_rect = cv.minAreaRect(contours[max_id])
                max_box = cv.boxPoints(max_rect)
                box = np.int0(max_box)
                (color_x, color_y), color_radius = cv.minEnclosingCircle(box)
                circle = int(color_x), int(color_y)
        
        if color_name == self.target_color_name:
            self.binary = binary
        self.msg_box[color_name] = box
        self.msg_circle[color_name] = circle

    def add_box(self, color_name):
        if len(self.msg_box[color_name]) != 0:
            cv.drawContours(self.frame, [self.msg_box[color_name]], 0, (255, 0, 0), 2)
            (color_x, color_y), color_radius = cv.minEnclosingCircle(self.msg_box[color_name])
            Center_x, Center_y, Center_r = int(color_x), int(color_y), int(color_radius)
            cv.circle(self.frame, (Center_x, Center_y), 5, (255, 0, 255), -1)
            cv.putText(self.frame, color_name, (Center_x, Center_y), cv.FONT_HERSHEY_SIMPLEX, 0.6, (100, 200, 200), 1)

    def Roi_hsv(self, img, Roi):
        """Get HSV range in a region of interest"""
        H = []
        S = []
        V = []
        HSV = cv.cvtColor(img, cv.COLOR_BGR2HSV)
        
        for i in range(Roi[0], Roi[2]):
            for j in range(Roi[1], Roi[3]):
                H.append(HSV[j, i][0])
                S.append(HSV[j, i][1])
                V.append(HSV[j, i][2])
        
        H_min = min(H)
        H_max = max(H)
        S_min = min(S)
        S_max = 253
        V_min = min(V)
        V_max = 255
        
        if H_max + 5 > 180:
            H_max = 180
        else:
            H_max += 5
        if H_min - 5 < 0:
            H_min = 0
        else:
            H_min -= 5
        if S_min - 10 < 0:
            S_min = 0
        else:
            S_min -= 10
        if V_min - 10 < 0:
            V_min = 0
        else:
            V_min -= 10
        
        lowerb = f'lowerb : ({H_min}, {S_min}, {V_min})'
        upperb = f'upperb : ({H_max}, {S_max}, {V_max})'
        cv.putText(img, lowerb, (150, 30), cv.FONT_HERSHEY_SIMPLEX, 0.5, (255, 0, 0), 1)
        cv.putText(img, upperb, (150, 50), cv.FONT_HERSHEY_SIMPLEX, 0.5, (255, 0, 0), 1)
        hsv_range = ((int(H_min), int(S_min), int(V_min)), (int(H_max), int(S_max), int(V_max)))
        return img, hsv_range


class SimplePID:
    """Simple discrete PID controller"""

    def __init__(self, target, P, I, D):
        if (not (np.size(P) == np.size(I) == np.size(D)) or 
            ((np.size(target) == 1) and np.size(P) != 1) or 
            (np.size(target) != 1 and (np.size(P) != np.size(target) and (np.size(P) != 1)))):
            raise TypeError('input parameters shape is not compatible')
        
        self.Kp = np.array(P)
        self.Ki = np.array(I)
        self.Kd = np.array(D)
        self.last_error = 0
        self.integrator = 0
        self.timeOfLastCall = None
        self.setPoint = np.array(target)
        self.integrator_max = float('inf')

    def update(self, current_value):
        current_value = np.array(current_value)
        if np.size(current_value) != np.size(self.setPoint):
            raise TypeError('current_value and target do not have the same shape')
        
        if self.timeOfLastCall is None:
            self.timeOfLastCall = time.perf_counter()
            return np.zeros(np.size(current_value))
        
        error = self.setPoint - current_value
        P = error
        currentTime = time.perf_counter()
        deltaT = (currentTime - self.timeOfLastCall)
        self.integrator = self.integrator + (error * deltaT)
        I = self.integrator
        D = (error - self.last_error) / deltaT
        self.last_error = error
        self.timeOfLastCall = currentTime
        
        return self.Kp * P + self.Ki * I + self.Kd * D

