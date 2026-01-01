cmake_minimum_required(VERSION 3.8)
project(arm_moveit_demo)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)
find_package(rclpy REQUIRED)

# Install Python package
ament_python_install_package(${PROJECT_NAME})

# Install Python scripts
install(PROGRAMS
  arm_moveit_demo/random_move.py
  arm_moveit_demo/set_pose_plan.py
  arm_moveit_demo/set_joint_plan.py
  arm_moveit_demo/simulation_to_machine.py
  DESTINATION lib/${PROJECT_NAME}
)

# Install launch files
install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}
)

ament_package()

