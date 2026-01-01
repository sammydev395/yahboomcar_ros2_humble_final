// generated from rosidl_generator_py/resource/_idl_support.c.em
// with input from yahboomcar_msgs:msg/Target.idl
// generated code does not contain a copyright notice
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <Python.h>
#include <stdbool.h>
#ifndef _WIN32
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wunused-function"
#endif
#include "numpy/ndarrayobject.h"
#ifndef _WIN32
# pragma GCC diagnostic pop
#endif
#include "rosidl_runtime_c/visibility_control.h"
#include "yahboomcar_msgs/msg/detail/target__struct.h"
#include "yahboomcar_msgs/msg/detail/target__functions.h"

#include "rosidl_runtime_c/string.h"
#include "rosidl_runtime_c/string_functions.h"

ROSIDL_GENERATOR_C_IMPORT
bool builtin_interfaces__msg__time__convert_from_py(PyObject * _pymsg, void * _ros_message);
ROSIDL_GENERATOR_C_IMPORT
PyObject * builtin_interfaces__msg__time__convert_to_py(void * raw_ros_message);

ROSIDL_GENERATOR_C_EXPORT
bool yahboomcar_msgs__msg__target__convert_from_py(PyObject * _pymsg, void * _ros_message)
{
  // check that the passed message is of the expected Python class
  {
    char full_classname_dest[35];
    {
      char * class_name = NULL;
      char * module_name = NULL;
      {
        PyObject * class_attr = PyObject_GetAttrString(_pymsg, "__class__");
        if (class_attr) {
          PyObject * name_attr = PyObject_GetAttrString(class_attr, "__name__");
          if (name_attr) {
            class_name = (char *)PyUnicode_1BYTE_DATA(name_attr);
            Py_DECREF(name_attr);
          }
          PyObject * module_attr = PyObject_GetAttrString(class_attr, "__module__");
          if (module_attr) {
            module_name = (char *)PyUnicode_1BYTE_DATA(module_attr);
            Py_DECREF(module_attr);
          }
          Py_DECREF(class_attr);
        }
      }
      if (!class_name || !module_name) {
        return false;
      }
      snprintf(full_classname_dest, sizeof(full_classname_dest), "%s.%s", module_name, class_name);
    }
    assert(strncmp("yahboomcar_msgs.msg._target.Target", full_classname_dest, 34) == 0);
  }
  yahboomcar_msgs__msg__Target * ros_message = _ros_message;
  {  // frame_id
    PyObject * field = PyObject_GetAttrString(_pymsg, "frame_id");
    if (!field) {
      return false;
    }
    assert(PyUnicode_Check(field));
    PyObject * encoded_field = PyUnicode_AsUTF8String(field);
    if (!encoded_field) {
      Py_DECREF(field);
      return false;
    }
    rosidl_runtime_c__String__assign(&ros_message->frame_id, PyBytes_AS_STRING(encoded_field));
    Py_DECREF(encoded_field);
    Py_DECREF(field);
  }
  {  // stamp
    PyObject * field = PyObject_GetAttrString(_pymsg, "stamp");
    if (!field) {
      return false;
    }
    if (!builtin_interfaces__msg__time__convert_from_py(field, &ros_message->stamp)) {
      Py_DECREF(field);
      return false;
    }
    Py_DECREF(field);
  }
  {  // scores
    PyObject * field = PyObject_GetAttrString(_pymsg, "scores");
    if (!field) {
      return false;
    }
    assert(PyFloat_Check(field));
    ros_message->scores = (float)PyFloat_AS_DOUBLE(field);
    Py_DECREF(field);
  }
  {  // ptx
    PyObject * field = PyObject_GetAttrString(_pymsg, "ptx");
    if (!field) {
      return false;
    }
    assert(PyFloat_Check(field));
    ros_message->ptx = (float)PyFloat_AS_DOUBLE(field);
    Py_DECREF(field);
  }
  {  // pty
    PyObject * field = PyObject_GetAttrString(_pymsg, "pty");
    if (!field) {
      return false;
    }
    assert(PyFloat_Check(field));
    ros_message->pty = (float)PyFloat_AS_DOUBLE(field);
    Py_DECREF(field);
  }
  {  // distw
    PyObject * field = PyObject_GetAttrString(_pymsg, "distw");
    if (!field) {
      return false;
    }
    assert(PyFloat_Check(field));
    ros_message->distw = (float)PyFloat_AS_DOUBLE(field);
    Py_DECREF(field);
  }
  {  // disth
    PyObject * field = PyObject_GetAttrString(_pymsg, "disth");
    if (!field) {
      return false;
    }
    assert(PyFloat_Check(field));
    ros_message->disth = (float)PyFloat_AS_DOUBLE(field);
    Py_DECREF(field);
  }
  {  // centerx
    PyObject * field = PyObject_GetAttrString(_pymsg, "centerx");
    if (!field) {
      return false;
    }
    assert(PyFloat_Check(field));
    ros_message->centerx = (float)PyFloat_AS_DOUBLE(field);
    Py_DECREF(field);
  }
  {  // centery
    PyObject * field = PyObject_GetAttrString(_pymsg, "centery");
    if (!field) {
      return false;
    }
    assert(PyFloat_Check(field));
    ros_message->centery = (float)PyFloat_AS_DOUBLE(field);
    Py_DECREF(field);
  }

  return true;
}

ROSIDL_GENERATOR_C_EXPORT
PyObject * yahboomcar_msgs__msg__target__convert_to_py(void * raw_ros_message)
{
  /* NOTE(esteve): Call constructor of Target */
  PyObject * _pymessage = NULL;
  {
    PyObject * pymessage_module = PyImport_ImportModule("yahboomcar_msgs.msg._target");
    assert(pymessage_module);
    PyObject * pymessage_class = PyObject_GetAttrString(pymessage_module, "Target");
    assert(pymessage_class);
    Py_DECREF(pymessage_module);
    _pymessage = PyObject_CallObject(pymessage_class, NULL);
    Py_DECREF(pymessage_class);
    if (!_pymessage) {
      return NULL;
    }
  }
  yahboomcar_msgs__msg__Target * ros_message = (yahboomcar_msgs__msg__Target *)raw_ros_message;
  {  // frame_id
    PyObject * field = NULL;
    field = PyUnicode_DecodeUTF8(
      ros_message->frame_id.data,
      strlen(ros_message->frame_id.data),
      "replace");
    if (!field) {
      return NULL;
    }
    {
      int rc = PyObject_SetAttrString(_pymessage, "frame_id", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // stamp
    PyObject * field = NULL;
    field = builtin_interfaces__msg__time__convert_to_py(&ros_message->stamp);
    if (!field) {
      return NULL;
    }
    {
      int rc = PyObject_SetAttrString(_pymessage, "stamp", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // scores
    PyObject * field = NULL;
    field = PyFloat_FromDouble(ros_message->scores);
    {
      int rc = PyObject_SetAttrString(_pymessage, "scores", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // ptx
    PyObject * field = NULL;
    field = PyFloat_FromDouble(ros_message->ptx);
    {
      int rc = PyObject_SetAttrString(_pymessage, "ptx", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // pty
    PyObject * field = NULL;
    field = PyFloat_FromDouble(ros_message->pty);
    {
      int rc = PyObject_SetAttrString(_pymessage, "pty", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // distw
    PyObject * field = NULL;
    field = PyFloat_FromDouble(ros_message->distw);
    {
      int rc = PyObject_SetAttrString(_pymessage, "distw", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // disth
    PyObject * field = NULL;
    field = PyFloat_FromDouble(ros_message->disth);
    {
      int rc = PyObject_SetAttrString(_pymessage, "disth", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // centerx
    PyObject * field = NULL;
    field = PyFloat_FromDouble(ros_message->centerx);
    {
      int rc = PyObject_SetAttrString(_pymessage, "centerx", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // centery
    PyObject * field = NULL;
    field = PyFloat_FromDouble(ros_message->centery);
    {
      int rc = PyObject_SetAttrString(_pymessage, "centery", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }

  // ownership of _pymessage is transferred to the caller
  return _pymessage;
}
