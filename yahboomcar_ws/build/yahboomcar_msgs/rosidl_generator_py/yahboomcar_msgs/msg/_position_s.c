// generated from rosidl_generator_py/resource/_idl_support.c.em
// with input from yahboomcar_msgs:msg/Position.idl
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
#include "yahboomcar_msgs/msg/detail/position__struct.h"
#include "yahboomcar_msgs/msg/detail/position__functions.h"


ROSIDL_GENERATOR_C_EXPORT
bool yahboomcar_msgs__msg__position__convert_from_py(PyObject * _pymsg, void * _ros_message)
{
  // check that the passed message is of the expected Python class
  {
    char full_classname_dest[39];
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
    assert(strncmp("yahboomcar_msgs.msg._position.Position", full_classname_dest, 38) == 0);
  }
  yahboomcar_msgs__msg__Position * ros_message = _ros_message;
  {  // anglex
    PyObject * field = PyObject_GetAttrString(_pymsg, "anglex");
    if (!field) {
      return false;
    }
    assert(PyFloat_Check(field));
    ros_message->anglex = (float)PyFloat_AS_DOUBLE(field);
    Py_DECREF(field);
  }
  {  // angley
    PyObject * field = PyObject_GetAttrString(_pymsg, "angley");
    if (!field) {
      return false;
    }
    assert(PyFloat_Check(field));
    ros_message->angley = (float)PyFloat_AS_DOUBLE(field);
    Py_DECREF(field);
  }
  {  // distance
    PyObject * field = PyObject_GetAttrString(_pymsg, "distance");
    if (!field) {
      return false;
    }
    assert(PyFloat_Check(field));
    ros_message->distance = (float)PyFloat_AS_DOUBLE(field);
    Py_DECREF(field);
  }

  return true;
}

ROSIDL_GENERATOR_C_EXPORT
PyObject * yahboomcar_msgs__msg__position__convert_to_py(void * raw_ros_message)
{
  /* NOTE(esteve): Call constructor of Position */
  PyObject * _pymessage = NULL;
  {
    PyObject * pymessage_module = PyImport_ImportModule("yahboomcar_msgs.msg._position");
    assert(pymessage_module);
    PyObject * pymessage_class = PyObject_GetAttrString(pymessage_module, "Position");
    assert(pymessage_class);
    Py_DECREF(pymessage_module);
    _pymessage = PyObject_CallObject(pymessage_class, NULL);
    Py_DECREF(pymessage_class);
    if (!_pymessage) {
      return NULL;
    }
  }
  yahboomcar_msgs__msg__Position * ros_message = (yahboomcar_msgs__msg__Position *)raw_ros_message;
  {  // anglex
    PyObject * field = NULL;
    field = PyFloat_FromDouble(ros_message->anglex);
    {
      int rc = PyObject_SetAttrString(_pymessage, "anglex", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // angley
    PyObject * field = NULL;
    field = PyFloat_FromDouble(ros_message->angley);
    {
      int rc = PyObject_SetAttrString(_pymessage, "angley", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // distance
    PyObject * field = NULL;
    field = PyFloat_FromDouble(ros_message->distance);
    {
      int rc = PyObject_SetAttrString(_pymessage, "distance", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }

  // ownership of _pymessage is transferred to the caller
  return _pymessage;
}
