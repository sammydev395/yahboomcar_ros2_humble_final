# generated from ament/cmake/core/templates/nameConfig.cmake.in

# prevent multiple inclusion
if(_garbage_identify_yolov11_CONFIG_INCLUDED)
  # ensure to keep the found flag the same
  if(NOT DEFINED garbage_identify_yolov11_FOUND)
    # explicitly set it to FALSE, otherwise CMake will set it to TRUE
    set(garbage_identify_yolov11_FOUND FALSE)
  elseif(NOT garbage_identify_yolov11_FOUND)
    # use separate condition to avoid uninitialized variable warning
    set(garbage_identify_yolov11_FOUND FALSE)
  endif()
  return()
endif()
set(_garbage_identify_yolov11_CONFIG_INCLUDED TRUE)

# output package information
if(NOT garbage_identify_yolov11_FIND_QUIETLY)
  message(STATUS "Found garbage_identify_yolov11: 0.3.0 (${garbage_identify_yolov11_DIR})")
endif()

# warn when using a deprecated package
if(NOT "" STREQUAL "")
  set(_msg "Package 'garbage_identify_yolov11' is deprecated")
  # append custom deprecation text if available
  if(NOT "" STREQUAL "TRUE")
    set(_msg "${_msg} ()")
  endif()
  # optionally quiet the deprecation message
  if(NOT ${garbage_identify_yolov11_DEPRECATED_QUIET})
    message(DEPRECATION "${_msg}")
  endif()
endif()

# flag package as ament-based to distinguish it after being find_package()-ed
set(garbage_identify_yolov11_FOUND_AMENT_PACKAGE TRUE)

# include all config extra files
set(_extras "")
foreach(_extra ${_extras})
  include("${garbage_identify_yolov11_DIR}/${_extra}")
endforeach()
