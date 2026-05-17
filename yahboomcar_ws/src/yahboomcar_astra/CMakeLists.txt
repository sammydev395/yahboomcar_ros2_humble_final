cmake_minimum_required(VERSION 3.8)
project(yahboomcar_astra)

# PATCHED 2026-05-16 (container rebuild prod strategy): the vendor built
# astra_camera_msgs via add_subdirectory() with no add_dependencies(),
# which races on a clean parallel colcon build (msg headers not ready
# when main.cpp compiles). The interface-identical astra_camera_msgs is
# built into the baked overlay (/opt/ros/humble_overlays) by the
# container Dockerfile, and is also provided by library_ws on host
# rebuilds — so we find_package() it like a normal dependency instead.
# The in-tree astra_camera_msgs/ subdir is now dead (left in place,
# unused).
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_msgs REQUIRED)
find_package(tf2_sensor_msgs REQUIRED)
find_package(camera_info_manager REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(image_transport REQUIRED)
find_package(image_geometry REQUIRED)
# find_package(image_publisher REQUIRED)  # Optional, not available in Humble
find_package(Eigen3 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(class_loader REQUIRED)
find_package(message_filters REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(astra_camera_msgs REQUIRED)

include_directories(
  include
  ${OpenCV_INCLUDE_DIRS}
  ${EIGEN3_INCLUDE_DIRS}
)

set(OB_CAMERA_SOURCES
  src/main.cpp
  src/ob_camera_info.cpp
  src/ob_camera_node.cpp
  src/ob_camera_node_factory.cpp
  src/ob_context.cpp
  src/ob_timer_filter.cpp
  src/dynamic_params.cpp
  src/ros_param_backend.cpp
  src/ros_service.cpp
  src/utils.cpp
  src/uvc_camera_driver.cpp
  src/point_cloud_proc/point_cloud_xyz.cpp
  src/point_cloud_proc/point_cloud_xyzrgb.cpp
)

set(OB_CAMERA_HEADERS
  include/astra_camera/ob_camera_node.h
  include/astra_camera/ob_camera_node_factory.h
  include/astra_camera/ob_context.h
  include/astra_camera/ob_timer_filter.h
  include/astra_camera/dynamic_params.h
  include/astra_camera/ros_param_backend.h
  include/astra_camera/ros_service.h
  include/astra_camera/utils.h
  include/astra_camera/uvc_camera_driver.h
  include/astra_camera/constants.h
  include/astra_camera/types.h
  include/astra_camera/json.hpp
  include/magic_enum/magic_enum.hpp
  include/astra_camera/point_cloud_proc/point_cloud_proc.h
  include/astra_camera/point_cloud_proc/depth_traits.h
  include/astra_camera/point_cloud_proc/point_cloud_xyz.h
  include/astra_camera/point_cloud_proc/point_cloud_xyzrgb.h
)

add_library(yahboomcar_astra_driver SHARED
  ${OB_CAMERA_SOURCES}
)

ament_target_dependencies(yahboomcar_astra_driver
  rclcpp
  rclcpp_components
  sensor_msgs
  std_msgs
  std_srvs
  tf2
  tf2_ros
  tf2_msgs
  tf2_sensor_msgs
  camera_info_manager
  cv_bridge
  image_transport
  image_geometry
  # image_publisher  # Not available in Humble
  Eigen3
  OpenCV
  class_loader
  message_filters
  ament_index_cpp
  astra_camera_msgs
)

target_include_directories(yahboomcar_astra_driver PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  # astra_camera_msgs headers now come from find_package via
  # ament_target_dependencies (no more in-tree binary-dir hack).
  ${OpenCV_INCLUDE_DIRS}
  ${EIGEN3_INCLUDE_DIRS}
)

target_link_libraries(yahboomcar_astra_driver
  ${OpenCV_LIBS}
  Eigen3::Eigen
  # astra_camera_msgs  # Built as subdirectory, headers available via include
  -lOpenNI2
  -L${CMAKE_CURRENT_SOURCE_DIR}/openni2_redist/arm64
  -Wl,-rpath,${CMAKE_CURRENT_SOURCE_DIR}/openni2_redist/arm64
  -lglog
  -luvc
)

add_executable(astra_camera_node src/main.cpp)
target_link_libraries(astra_camera_node
  yahboomcar_astra_driver
  ${OpenCV_LIBS}
  Eigen3::Eigen
)
# astra_camera_msgs typesupport now linked via ament_target_dependencies
# (resolves the rosidl libs from the found package) instead of a
# hardcoded in-tree binary-dir .so path.
ament_target_dependencies(astra_camera_node rclcpp astra_camera_msgs)
# Set rpath so executable can find the driver library
set_target_properties(astra_camera_node PROPERTIES
  INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib"
  BUILD_WITH_INSTALL_RPATH TRUE
)

# Keep KCFTracker_node build for now
add_executable(KCFTracker_node
  src/KCF_Tracker_main.cpp
  src/KCF_Tracker.cpp
  src/yahboomcar_astra/kcftracker.cpp
  src/yahboomcar_astra/fhog.cpp
  src/yahboomcar_astra/PID.cpp
)
target_include_directories(KCFTracker_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(KCFTracker_node
  ${OpenCV_LIBRARIES}
)
ament_target_dependencies(KCFTracker_node
  rclcpp
  sensor_msgs
  geometry_msgs
  std_msgs
  std_srvs
  cv_bridge
)

install(TARGETS
  yahboomcar_astra_driver
  astra_camera_node
  KCFTracker_node
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
  DESTINATION include
)

install(DIRECTORY openni2_redist/arm64
  DESTINATION lib/${PROJECT_NAME}/openni2_redist
)

install(DIRECTORY launch/
  DESTINATION share/${PROJECT_NAME}/launch
)

# Fix for recursion issue - use standard install instead of symlink
ament_package()
