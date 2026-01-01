# generated from ament/cmake/core/templates/nameConfig.cmake.in

# prevent multiple inclusion
if(_arm_color_transport_CONFIG_INCLUDED)
  # ensure to keep the found flag the same
  if(NOT DEFINED arm_color_transport_FOUND)
    # explicitly set it to FALSE, otherwise CMake will set it to TRUE
    set(arm_color_transport_FOUND FALSE)
  elseif(NOT arm_color_transport_FOUND)
    # use separate condition to avoid uninitialized variable warning
    set(arm_color_transport_FOUND FALSE)
  endif()
  return()
endif()
set(_arm_color_transport_CONFIG_INCLUDED TRUE)

# output package information
if(NOT arm_color_transport_FIND_QUIETLY)
  message(STATUS "Found arm_color_transport: 0.1.0 (${arm_color_transport_DIR})")
endif()

# warn when using a deprecated package
if(NOT "" STREQUAL "")
  set(_msg "Package 'arm_color_transport' is deprecated")
  # append custom deprecation text if available
  if(NOT "" STREQUAL "TRUE")
    set(_msg "${_msg} ()")
  endif()
  # optionally quiet the deprecation message
  if(NOT ${arm_color_transport_DEPRECATED_QUIET})
    message(DEPRECATION "${_msg}")
  endif()
endif()

# flag package as ament-based to distinguish it after being find_package()-ed
set(arm_color_transport_FOUND_AMENT_PACKAGE TRUE)

# include all config extra files
set(_extras "")
foreach(_extra ${_extras})
  include("${arm_color_transport_DIR}/${_extra}")
endforeach()
