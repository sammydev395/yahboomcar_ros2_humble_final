// generated from rosidl_typesupport_introspection_c/resource/idl__type_support.c.em
// with input from yahboomcar_msgs:msg/Position.idl
// generated code does not contain a copyright notice

#include <stddef.h>
#include "yahboomcar_msgs/msg/detail/position__rosidl_typesupport_introspection_c.h"
#include "yahboomcar_msgs/msg/rosidl_typesupport_introspection_c__visibility_control.h"
#include "rosidl_typesupport_introspection_c/field_types.h"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/message_introspection.h"
#include "yahboomcar_msgs/msg/detail/position__functions.h"
#include "yahboomcar_msgs/msg/detail/position__struct.h"


#ifdef __cplusplus
extern "C"
{
#endif

void yahboomcar_msgs__msg__Position__rosidl_typesupport_introspection_c__Position_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  yahboomcar_msgs__msg__Position__init(message_memory);
}

void yahboomcar_msgs__msg__Position__rosidl_typesupport_introspection_c__Position_fini_function(void * message_memory)
{
  yahboomcar_msgs__msg__Position__fini(message_memory);
}

static rosidl_typesupport_introspection_c__MessageMember yahboomcar_msgs__msg__Position__rosidl_typesupport_introspection_c__Position_message_member_array[3] = {
  {
    "anglex",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(yahboomcar_msgs__msg__Position, anglex),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "angley",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(yahboomcar_msgs__msg__Position, angley),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "distance",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(yahboomcar_msgs__msg__Position, distance),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers yahboomcar_msgs__msg__Position__rosidl_typesupport_introspection_c__Position_message_members = {
  "yahboomcar_msgs__msg",  // message namespace
  "Position",  // message name
  3,  // number of fields
  sizeof(yahboomcar_msgs__msg__Position),
  yahboomcar_msgs__msg__Position__rosidl_typesupport_introspection_c__Position_message_member_array,  // message members
  yahboomcar_msgs__msg__Position__rosidl_typesupport_introspection_c__Position_init_function,  // function to initialize message memory (memory has to be allocated)
  yahboomcar_msgs__msg__Position__rosidl_typesupport_introspection_c__Position_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t yahboomcar_msgs__msg__Position__rosidl_typesupport_introspection_c__Position_message_type_support_handle = {
  0,
  &yahboomcar_msgs__msg__Position__rosidl_typesupport_introspection_c__Position_message_members,
  get_message_typesupport_handle_function,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_yahboomcar_msgs
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, yahboomcar_msgs, msg, Position)() {
  if (!yahboomcar_msgs__msg__Position__rosidl_typesupport_introspection_c__Position_message_type_support_handle.typesupport_identifier) {
    yahboomcar_msgs__msg__Position__rosidl_typesupport_introspection_c__Position_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &yahboomcar_msgs__msg__Position__rosidl_typesupport_introspection_c__Position_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif
