// generated from rosidl_generator_c/resource/idl__functions.h.em
// with input from yahboom_web_savmap_interfaces:srv/WebSaveMap.idl
// generated code does not contain a copyright notice

#ifndef YAHBOOM_WEB_SAVMAP_INTERFACES__SRV__DETAIL__WEB_SAVE_MAP__FUNCTIONS_H_
#define YAHBOOM_WEB_SAVMAP_INTERFACES__SRV__DETAIL__WEB_SAVE_MAP__FUNCTIONS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdlib.h>

#include "rosidl_runtime_c/visibility_control.h"
#include "yahboom_web_savmap_interfaces/msg/rosidl_generator_c__visibility_control.h"

#include "yahboom_web_savmap_interfaces/srv/detail/web_save_map__struct.h"

/// Initialize srv/WebSaveMap message.
/**
 * If the init function is called twice for the same message without
 * calling fini inbetween previously allocated memory will be leaked.
 * \param[in,out] msg The previously allocated message pointer.
 * Fields without a default value will not be initialized by this function.
 * You might want to call memset(msg, 0, sizeof(
 * yahboom_web_savmap_interfaces__srv__WebSaveMap_Request
 * )) before or use
 * yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__create()
 * to allocate and initialize the message.
 * \return true if initialization was successful, otherwise false
 */
ROSIDL_GENERATOR_C_PUBLIC_yahboom_web_savmap_interfaces
bool
yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__init(yahboom_web_savmap_interfaces__srv__WebSaveMap_Request * msg);

/// Finalize srv/WebSaveMap message.
/**
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_yahboom_web_savmap_interfaces
void
yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__fini(yahboom_web_savmap_interfaces__srv__WebSaveMap_Request * msg);

/// Create srv/WebSaveMap message.
/**
 * It allocates the memory for the message, sets the memory to zero, and
 * calls
 * yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__init().
 * \return The pointer to the initialized message if successful,
 * otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_yahboom_web_savmap_interfaces
yahboom_web_savmap_interfaces__srv__WebSaveMap_Request *
yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__create();

/// Destroy srv/WebSaveMap message.
/**
 * It calls
 * yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__fini()
 * and frees the memory of the message.
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_yahboom_web_savmap_interfaces
void
yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__destroy(yahboom_web_savmap_interfaces__srv__WebSaveMap_Request * msg);

/// Check for srv/WebSaveMap message equality.
/**
 * \param[in] lhs The message on the left hand size of the equality operator.
 * \param[in] rhs The message on the right hand size of the equality operator.
 * \return true if messages are equal, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_yahboom_web_savmap_interfaces
bool
yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__are_equal(const yahboom_web_savmap_interfaces__srv__WebSaveMap_Request * lhs, const yahboom_web_savmap_interfaces__srv__WebSaveMap_Request * rhs);

/// Copy a srv/WebSaveMap message.
/**
 * This functions performs a deep copy, as opposed to the shallow copy that
 * plain assignment yields.
 *
 * \param[in] input The source message pointer.
 * \param[out] output The target message pointer, which must
 *   have been initialized before calling this function.
 * \return true if successful, or false if either pointer is null
 *   or memory allocation fails.
 */
ROSIDL_GENERATOR_C_PUBLIC_yahboom_web_savmap_interfaces
bool
yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__copy(
  const yahboom_web_savmap_interfaces__srv__WebSaveMap_Request * input,
  yahboom_web_savmap_interfaces__srv__WebSaveMap_Request * output);

/// Initialize array of srv/WebSaveMap messages.
/**
 * It allocates the memory for the number of elements and calls
 * yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__init()
 * for each element of the array.
 * \param[in,out] array The allocated array pointer.
 * \param[in] size The size / capacity of the array.
 * \return true if initialization was successful, otherwise false
 * If the array pointer is valid and the size is zero it is guaranteed
 # to return true.
 */
ROSIDL_GENERATOR_C_PUBLIC_yahboom_web_savmap_interfaces
bool
yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence__init(yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence * array, size_t size);

/// Finalize array of srv/WebSaveMap messages.
/**
 * It calls
 * yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__fini()
 * for each element of the array and frees the memory for the number of
 * elements.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_yahboom_web_savmap_interfaces
void
yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence__fini(yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence * array);

/// Create array of srv/WebSaveMap messages.
/**
 * It allocates the memory for the array and calls
 * yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence__init().
 * \param[in] size The size / capacity of the array.
 * \return The pointer to the initialized array if successful, otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_yahboom_web_savmap_interfaces
yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence *
yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence__create(size_t size);

/// Destroy array of srv/WebSaveMap messages.
/**
 * It calls
 * yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence__fini()
 * on the array,
 * and frees the memory of the array.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_yahboom_web_savmap_interfaces
void
yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence__destroy(yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence * array);

/// Check for srv/WebSaveMap message array equality.
/**
 * \param[in] lhs The message array on the left hand size of the equality operator.
 * \param[in] rhs The message array on the right hand size of the equality operator.
 * \return true if message arrays are equal in size and content, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_yahboom_web_savmap_interfaces
bool
yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence__are_equal(const yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence * lhs, const yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence * rhs);

/// Copy an array of srv/WebSaveMap messages.
/**
 * This functions performs a deep copy, as opposed to the shallow copy that
 * plain assignment yields.
 *
 * \param[in] input The source array pointer.
 * \param[out] output The target array pointer, which must
 *   have been initialized before calling this function.
 * \return true if successful, or false if either pointer
 *   is null or memory allocation fails.
 */
ROSIDL_GENERATOR_C_PUBLIC_yahboom_web_savmap_interfaces
bool
yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence__copy(
  const yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence * input,
  yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence * output);

/// Initialize srv/WebSaveMap message.
/**
 * If the init function is called twice for the same message without
 * calling fini inbetween previously allocated memory will be leaked.
 * \param[in,out] msg The previously allocated message pointer.
 * Fields without a default value will not be initialized by this function.
 * You might want to call memset(msg, 0, sizeof(
 * yahboom_web_savmap_interfaces__srv__WebSaveMap_Response
 * )) before or use
 * yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__create()
 * to allocate and initialize the message.
 * \return true if initialization was successful, otherwise false
 */
ROSIDL_GENERATOR_C_PUBLIC_yahboom_web_savmap_interfaces
bool
yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__init(yahboom_web_savmap_interfaces__srv__WebSaveMap_Response * msg);

/// Finalize srv/WebSaveMap message.
/**
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_yahboom_web_savmap_interfaces
void
yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__fini(yahboom_web_savmap_interfaces__srv__WebSaveMap_Response * msg);

/// Create srv/WebSaveMap message.
/**
 * It allocates the memory for the message, sets the memory to zero, and
 * calls
 * yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__init().
 * \return The pointer to the initialized message if successful,
 * otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_yahboom_web_savmap_interfaces
yahboom_web_savmap_interfaces__srv__WebSaveMap_Response *
yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__create();

/// Destroy srv/WebSaveMap message.
/**
 * It calls
 * yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__fini()
 * and frees the memory of the message.
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_yahboom_web_savmap_interfaces
void
yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__destroy(yahboom_web_savmap_interfaces__srv__WebSaveMap_Response * msg);

/// Check for srv/WebSaveMap message equality.
/**
 * \param[in] lhs The message on the left hand size of the equality operator.
 * \param[in] rhs The message on the right hand size of the equality operator.
 * \return true if messages are equal, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_yahboom_web_savmap_interfaces
bool
yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__are_equal(const yahboom_web_savmap_interfaces__srv__WebSaveMap_Response * lhs, const yahboom_web_savmap_interfaces__srv__WebSaveMap_Response * rhs);

/// Copy a srv/WebSaveMap message.
/**
 * This functions performs a deep copy, as opposed to the shallow copy that
 * plain assignment yields.
 *
 * \param[in] input The source message pointer.
 * \param[out] output The target message pointer, which must
 *   have been initialized before calling this function.
 * \return true if successful, or false if either pointer is null
 *   or memory allocation fails.
 */
ROSIDL_GENERATOR_C_PUBLIC_yahboom_web_savmap_interfaces
bool
yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__copy(
  const yahboom_web_savmap_interfaces__srv__WebSaveMap_Response * input,
  yahboom_web_savmap_interfaces__srv__WebSaveMap_Response * output);

/// Initialize array of srv/WebSaveMap messages.
/**
 * It allocates the memory for the number of elements and calls
 * yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__init()
 * for each element of the array.
 * \param[in,out] array The allocated array pointer.
 * \param[in] size The size / capacity of the array.
 * \return true if initialization was successful, otherwise false
 * If the array pointer is valid and the size is zero it is guaranteed
 # to return true.
 */
ROSIDL_GENERATOR_C_PUBLIC_yahboom_web_savmap_interfaces
bool
yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence__init(yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence * array, size_t size);

/// Finalize array of srv/WebSaveMap messages.
/**
 * It calls
 * yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__fini()
 * for each element of the array and frees the memory for the number of
 * elements.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_yahboom_web_savmap_interfaces
void
yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence__fini(yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence * array);

/// Create array of srv/WebSaveMap messages.
/**
 * It allocates the memory for the array and calls
 * yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence__init().
 * \param[in] size The size / capacity of the array.
 * \return The pointer to the initialized array if successful, otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_yahboom_web_savmap_interfaces
yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence *
yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence__create(size_t size);

/// Destroy array of srv/WebSaveMap messages.
/**
 * It calls
 * yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence__fini()
 * on the array,
 * and frees the memory of the array.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_yahboom_web_savmap_interfaces
void
yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence__destroy(yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence * array);

/// Check for srv/WebSaveMap message array equality.
/**
 * \param[in] lhs The message array on the left hand size of the equality operator.
 * \param[in] rhs The message array on the right hand size of the equality operator.
 * \return true if message arrays are equal in size and content, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_yahboom_web_savmap_interfaces
bool
yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence__are_equal(const yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence * lhs, const yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence * rhs);

/// Copy an array of srv/WebSaveMap messages.
/**
 * This functions performs a deep copy, as opposed to the shallow copy that
 * plain assignment yields.
 *
 * \param[in] input The source array pointer.
 * \param[out] output The target array pointer, which must
 *   have been initialized before calling this function.
 * \return true if successful, or false if either pointer
 *   is null or memory allocation fails.
 */
ROSIDL_GENERATOR_C_PUBLIC_yahboom_web_savmap_interfaces
bool
yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence__copy(
  const yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence * input,
  yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence * output);

#ifdef __cplusplus
}
#endif

#endif  // YAHBOOM_WEB_SAVMAP_INTERFACES__SRV__DETAIL__WEB_SAVE_MAP__FUNCTIONS_H_
