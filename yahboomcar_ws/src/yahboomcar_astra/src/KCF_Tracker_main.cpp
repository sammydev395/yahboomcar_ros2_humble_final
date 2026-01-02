#include <memory>
#include <rclcpp/rclcpp.hpp>
#include "yahboomcar_astra/KCF_Tracker.h"

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<ImageConverter>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
