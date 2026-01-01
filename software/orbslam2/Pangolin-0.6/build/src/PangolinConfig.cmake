# Compute paths
get_filename_component( PROJECT_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH )
SET( Pangolin_INCLUDE_DIRS "/home/jetson/yahboomcar_ros2_ws/software/orbslam2/Pangolin-0.6/include;/home/jetson/yahboomcar_ros2_ws/software/orbslam2/Pangolin-0.6/build/src/include;/usr/include;/usr/include;/usr/include;/usr/local/include/eigen3" )
SET( Pangolin_INCLUDE_DIR  "/home/jetson/yahboomcar_ros2_ws/software/orbslam2/Pangolin-0.6/include;/home/jetson/yahboomcar_ros2_ws/software/orbslam2/Pangolin-0.6/build/src/include;/usr/include;/usr/include;/usr/include;/usr/local/include/eigen3" )

# Library dependencies (contains definitions for IMPORTED targets)
if( NOT TARGET pangolin AND NOT Pangolin_BINARY_DIR )
  include( "${PROJECT_CMAKE_DIR}/PangolinTargets.cmake" )
  
endif()

SET( Pangolin_LIBRARIES    pangolin )
SET( Pangolin_LIBRARY      pangolin )
SET( Pangolin_CMAKEMODULES /home/jetson/yahboomcar_ros2_ws/software/orbslam2/Pangolin-0.6/src/../CMakeModules )
