// D1 stub. Lifecycle returns SUCCESS, read/write are no-ops, no state or
// command interfaces are declared. D2 brings up chassis (4 wheels +
// FUNC_REPORT_ENCODER push); D3 adds IMU; D4 adds the 6 arm joints.

#include "yahboom_ros2_control/yahboom_system.hpp"

#include <pluginlib/class_list_macros.hpp>

namespace yahboom_ros2_control {

hardware_interface::CallbackReturn YahboomSystem::on_init(
    const hardware_interface::HardwareInfo& info) {
  if (hardware_interface::SystemInterface::on_init(info) !=
      hardware_interface::CallbackReturn::SUCCESS) {
    return hardware_interface::CallbackReturn::ERROR;
  }
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn YahboomSystem::on_configure(
    const rclcpp_lifecycle::State& /*previous_state*/) {
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn YahboomSystem::on_activate(
    const rclcpp_lifecycle::State& /*previous_state*/) {
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn YahboomSystem::on_deactivate(
    const rclcpp_lifecycle::State& /*previous_state*/) {
  return hardware_interface::CallbackReturn::SUCCESS;
}

std::vector<hardware_interface::StateInterface> YahboomSystem::export_state_interfaces() {
  return {};
}

std::vector<hardware_interface::CommandInterface> YahboomSystem::export_command_interfaces() {
  return {};
}

hardware_interface::return_type YahboomSystem::read(const rclcpp::Time& /*time*/,
                                                     const rclcpp::Duration& /*period*/) {
  return hardware_interface::return_type::OK;
}

hardware_interface::return_type YahboomSystem::write(const rclcpp::Time& /*time*/,
                                                      const rclcpp::Duration& /*period*/) {
  return hardware_interface::return_type::OK;
}

}  // namespace yahboom_ros2_control

PLUGINLIB_EXPORT_CLASS(yahboom_ros2_control::YahboomSystem,
                       hardware_interface::SystemInterface)
