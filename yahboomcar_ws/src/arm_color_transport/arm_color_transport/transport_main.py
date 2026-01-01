#!/usr/bin/env python3
# encoding: utf-8
"""
Arm Color Transport Main Node - ROS2 Version
Color-based pick and transport using navigation and arm control
"""
import threading
import cv2 as cv
from time import sleep, time

import rclpy
from rclpy.node import Node

from arm_color_transport.transport_common import ROSNav


class ColorTransportNode(Node):
    """ROS2 Node for color-based transport"""
    
    def __init__(self):
        super().__init__('color_transport')
        self.get_logger().info('Initializing Color Transport Node (ROS2)...')
        
        self.ros_nav = ROSNav(self)
        self.model = "Init"
        self.Grip_status = False
        self.color_name = ""
        self.index = 0

    def get_color(self, img):
        """Detect color in center region of image"""
        H = []
        color_name = ""
        HSV = cv.cvtColor(img, cv.COLOR_BGR2HSV)
        cv.rectangle(img, (280, 180), (360, 260), (0, 255, 0), 2)
        
        for i in range(280, 360):
            for j in range(180, 260):
                H.append(HSV[j, i][0])
        
        H_min = min(H)
        H_max = max(H)
        
        if (H_min >= 0 and H_max <= 10) or (H_min >= 156 and H_max <= 180):
            color_name = 'red'
        elif H_min >= 23 and H_max <= 56:
            color_name = 'yellow'
        elif H_min >= 35 and H_max <= 78:
            color_name = 'green'
        elif H_min >= 100 and H_max <= 124:
            color_name = 'blue'
        
        txt_H = f'Hmin : {H_min} Hmax : {H_max}'
        cv.putText(img, txt_H, (270, 30), cv.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 255), 1)
        return img, color_name

    def process(self, frame, action, text):
        """Main processing loop"""
        if action == 32 or self.ros_nav.joy_action == 2:
            self.model = "Grip"
        elif action == ord('r') or action == ord('R'):
            self.Reset()
        elif action == ord('q') or action == ord('Q'):
            self.ros_nav.cancel()
        
        if self.model == "Grip":
            if not self.Grip_status:
                frame, self.color_name = self.get_color(frame)
                if len(self.color_name) != 0:
                    threading.Thread(target=self.Grip_Target).start()
        elif self.model == "Transport":
            if len(self.ros_nav.color_pose) != 0 and not self.ros_nav.Transport_status:
                if self.color_name in self.ros_nav.color_pose.keys():
                    self.ros_nav.PubTargetPoint(self.ros_nav.color_pose[self.color_name])
                    self.model = "Grip_down"
                    self.ros_nav.Transport_status = True
        elif self.model == "Grip_down":
            if self.ros_nav.goal_result == 3:
                threading.Thread(target=self.Grip_down).start()
        elif self.model == "come_back":
            if self.ros_nav.goal_result == 3:
                threading.Thread(target=self.buzzer_loop).start()
                self.Reset()
        
        cv.putText(frame, text, (30, 30), cv.FONT_HERSHEY_SIMPLEX, 0.6, (100, 200, 200), 1)
        cv.putText(frame, self.model, (30, 450), cv.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 255), 1)
        self.ros_nav.pubImg(frame)
        return frame

    def comeback(self):
        """Navigate back to start position"""
        self.ros_nav.PubTargetPoint(self.ros_nav.start_point)
        self.model = "come_back"

    def Reset(self):
        """Reset all states"""
        self.ros_nav.goal_result = 0
        self.model = "Init"
        self.Grip_status = False
        self.ros_nav.Transport_status = False
        self.color_name = ""
        self.index = 0
        self.get_logger().info("Reset complete")

    def Grip_down(self):
        """Execute arm down and release sequence"""
        self.model = "Grip_down"
        self.ros_nav.goal_result = 0
        joints = [90, 2.0, 60.0, 40.0, 90, 140]
        self.ros_nav.pubArm(joints, run_time=1000)
        sleep(1)
        self.ros_nav.pubArm([], 6, 30)
        sleep(0.5)
        joints = [90, 145, 0, 45, 90, 30]
        self.ros_nav.pubArm(joints, run_time=1000)
        sleep(1)
        self.comeback()

    def Grip_Target(self):
        """Execute arm grip sequence"""
        self.model = "Grip_Target"
        self.Grip_status = True
        self.buzzer_loop()
        joints = [90, 145, 0, 45, 90, 30]
        self.ros_nav.pubArm(joints, run_time=1000)
        sleep(0.5)
        self.buzzer_loop()
        self.ros_nav.pubArm([], 6, 146)
        sleep(1)
        self.model = "Transport"

    def buzzer_loop(self):
        """Buzzer feedback"""
        self.ros_nav.pubBuzzer(True)
        sleep(1)
        self.ros_nav.pubBuzzer(False)
        sleep(1)


def main(args=None):
    rclpy.init(args=args)
    
    node = ColorTransportNode()
    
    capture = cv.VideoCapture("/dev/camera_usb")
    if not capture.isOpened():
        capture = cv.VideoCapture(0)
    
    cv_edition = cv.__version__
    if cv_edition[0] == '3':
        capture.set(cv.CAP_PROP_FOURCC, cv.VideoWriter_fourcc(*'XVID'))
    else:
        capture.set(cv.CAP_PROP_FOURCC, cv.VideoWriter.fourcc('M', 'J', 'P', 'G'))
    capture.set(cv.CAP_PROP_FRAME_WIDTH, 640)
    capture.set(cv.CAP_PROP_FRAME_HEIGHT, 480)
    
    text = '0'
    try:
        while capture.isOpened() and rclpy.ok():
            start = time()
            ret, frame = capture.read()
            if not ret:
                continue
            
            action = cv.waitKey(10) & 0xFF
            if action == ord('q') or action == 113:
                break
            
            frame = node.process(frame, action, text)
            text = "FPS : " + str(int(1 / (time() - start))) if (time() - start) > 0 else "FPS: 0"
            
            if node.ros_nav.img_show:
                cv.imshow("frame", frame)
            
            rclpy.spin_once(node, timeout_sec=0.001)
    
    except KeyboardInterrupt:
        pass
    finally:
        node.ros_nav.cancel()
        capture.release()
        cv.destroyAllWindows()
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()

