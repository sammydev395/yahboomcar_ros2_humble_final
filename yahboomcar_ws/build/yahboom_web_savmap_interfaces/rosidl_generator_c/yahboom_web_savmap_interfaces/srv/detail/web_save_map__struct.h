// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from yahboom_web_savmap_interfaces:srv/WebSaveMap.idl
// generated code does not contain a copyright notice

#ifndef YAHBOOM_WEB_SAVMAP_INTERFACES__SRV__DETAIL__WEB_SAVE_MAP__STRUCT_H_
#define YAHBOOM_WEB_SAVMAP_INTERFACES__SRV__DETAIL__WEB_SAVE_MAP__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'mapname'
#include "rosidl_runtime_c/string.h"

/// Struct defined in srv/WebSaveMap in the package yahboom_web_savmap_interfaces.
typedef struct yahboom_web_savmap_interfaces__srv__WebSaveMap_Request
{
  rosidl_runtime_c__String mapname;
} yahboom_web_savmap_interfaces__srv__WebSaveMap_Request;

// Struct for a sequence of yahboom_web_savmap_interfaces__srv__WebSaveMap_Request.
typedef struct yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence
{
  yahboom_web_savmap_interfaces__srv__WebSaveMap_Request * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'response'
// already included above
// #include "rosidl_runtime_c/string.h"

/// Struct defined in srv/WebSaveMap in the package yahboom_web_savmap_interfaces.
typedef struct yahboom_web_savmap_interfaces__srv__WebSaveMap_Response
{
  rosidl_runtime_c__String response;
} yahboom_web_savmap_interfaces__srv__WebSaveMap_Response;

// Struct for a sequence of yahboom_web_savmap_interfaces__srv__WebSaveMap_Response.
typedef struct yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence
{
  yahboom_web_savmap_interfaces__srv__WebSaveMap_Response * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // YAHBOOM_WEB_SAVMAP_INTERFACES__SRV__DETAIL__WEB_SAVE_MAP__STRUCT_H_
