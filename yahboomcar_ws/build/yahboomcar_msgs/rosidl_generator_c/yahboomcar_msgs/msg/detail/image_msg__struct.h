// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from yahboomcar_msgs:msg/ImageMsg.idl
// generated code does not contain a copyright notice

#ifndef YAHBOOMCAR_MSGS__MSG__DETAIL__IMAGE_MSG__STRUCT_H_
#define YAHBOOMCAR_MSGS__MSG__DETAIL__IMAGE_MSG__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'data'
#include "rosidl_runtime_c/primitives_sequence.h"

/// Struct defined in msg/ImageMsg in the package yahboomcar_msgs.
typedef struct yahboomcar_msgs__msg__ImageMsg
{
  int32_t height;
  int32_t width;
  int32_t channels;
  rosidl_runtime_c__uint8__Sequence data;
} yahboomcar_msgs__msg__ImageMsg;

// Struct for a sequence of yahboomcar_msgs__msg__ImageMsg.
typedef struct yahboomcar_msgs__msg__ImageMsg__Sequence
{
  yahboomcar_msgs__msg__ImageMsg * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} yahboomcar_msgs__msg__ImageMsg__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // YAHBOOMCAR_MSGS__MSG__DETAIL__IMAGE_MSG__STRUCT_H_
