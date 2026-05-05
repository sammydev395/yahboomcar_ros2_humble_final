cmake_minimum_required(VERSION 3.16)
project(yahboom_ros2_control LANGUAGES CXX)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
  add_compile_options(-g -O1 -fno-omit-frame-pointer)
endif()

find_package(ament_cmake REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(pluginlib REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)

add_library(yahboom_ros2_control SHARED
  src/yahboom_system.cpp
  src/yahboom_serial.cpp
)

target_include_directories(yahboom_ros2_control PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(yahboom_ros2_control
  hardware_interface
  pluginlib
  rclcpp
  rclcpp_lifecycle
)

pluginlib_export_plugin_description_file(hardware_interface yahboom_ros2_control.xml)

# Hardware smoke test — opens /dev/yahboom_stm32, enables push reports, listens
# for ~3s, prints frames. Not a unit test (needs real STM32). Run as
# `ros2 run yahboom_ros2_control smoke_serial`.
add_executable(smoke_serial tools/smoke_serial.cpp)
target_link_libraries(smoke_serial yahboom_ros2_control)
target_include_directories(smoke_serial PRIVATE include)

install(
  TARGETS yahboom_ros2_control
  EXPORT export_yahboom_ros2_control
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(
  TARGETS smoke_serial
  DESTINATION lib/${PROJECT_NAME}
)

install(
  DIRECTORY include/
  DESTINATION include
)

install(
  DIRECTORY config description launch
  DESTINATION share/${PROJECT_NAME}
  OPTIONAL
)

ament_export_targets(export_yahboom_ros2_control HAS_LIBRARY_TARGET)
ament_export_dependencies(hardware_interface pluginlib rclcpp rclcpp_lifecycle)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_yahboom_protocol test/test_yahboom_protocol.cpp)
  target_include_directories(test_yahboom_protocol PRIVATE include)
endif()

ament_package()
