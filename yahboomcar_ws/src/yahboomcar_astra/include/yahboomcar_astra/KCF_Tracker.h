//
// Created by yahboom on 2021/7/30.
//

#ifndef TRANSBOT_ASTRA_KCF_TRACKER_H
#define TRANSBOT_ASTRA_KCF_TRACKER_H

#include <iostream>
#include <algorithm>
#include <memory>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/msg/image.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <std_msgs/msg/bool.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "kcftracker.h"
#include "PID.h"
#include <rclcpp/rclcpp.hpp>
#include <rcl_interfaces/msg/set_parameters_result.hpp>
#include "std_srvs/srv/set_bool.hpp"

using namespace std;
using namespace cv;

class ImageConverter : public rclcpp::Node {
public:
    ImageConverter();
    ~ImageConverter();

    void reset();
    void cancel();
    void imageCb(const sensor_msgs::msg::Image::ConstSharedPtr msg);
    void depthCb(const sensor_msgs::msg::Image::ConstSharedPtr msg);
    void joyCb(const std_msgs::msg::Bool::ConstSharedPtr msg);
    void parameterCallback(const std::vector<rclcpp::Parameter> &parameters);
    void enable_depth_stream();

    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr pub;
    PID *linear_PID;
    PID *angular_PID;
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr image_sub_;
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr depth_sub_;
    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr joy_sub_;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr image_pub_;
    const char *RGB_WINDOW = "rgb_img";
    const char *DEPTH_WINDOW = "depth_img";
    float minDist = 1.0;
    float linear_speed = 0;
    float rotation_speed = 0;
    bool enable_get_depth = false;
    float dist_val[5];
    bool HOG = true;
    bool FIXEDWINDOW = false;
    bool MULTISCALE = true;
    bool LAB = false;
    int center_x;
    KCFTracker tracker;

private:
    void declare_and_get_parameters();
    rclcpp::Client<std_srvs::srv::SetBool>::SharedPtr depth_toggle_client_;
};

#endif //TRANSBOT_ASTRA_KCF_TRACKER_H
