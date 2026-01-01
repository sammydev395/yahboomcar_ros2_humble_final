# generated from rosidl_generator_py/resource/_idl.py.em
# with input from cartographer_ros_msgs:msg/RobotPose.idl
# generated code does not contain a copyright notice


# Import statements for member types

import builtins  # noqa: E402, I100

import math  # noqa: E402, I100

import rosidl_parser.definition  # noqa: E402, I100


class Metaclass_RobotPose(type):
    """Metaclass of message 'RobotPose'."""

    _CREATE_ROS_MESSAGE = None
    _CONVERT_FROM_PY = None
    _CONVERT_TO_PY = None
    _DESTROY_ROS_MESSAGE = None
    _TYPE_SUPPORT = None

    __constants = {
    }

    @classmethod
    def __import_type_support__(cls):
        try:
            from rosidl_generator_py import import_type_support
            module = import_type_support('cartographer_ros_msgs')
        except ImportError:
            import logging
            import traceback
            logger = logging.getLogger(
                'cartographer_ros_msgs.msg.RobotPose')
            logger.debug(
                'Failed to import needed modules for type support:\n' +
                traceback.format_exc())
        else:
            cls._CREATE_ROS_MESSAGE = module.create_ros_message_msg__msg__robot_pose
            cls._CONVERT_FROM_PY = module.convert_from_py_msg__msg__robot_pose
            cls._CONVERT_TO_PY = module.convert_to_py_msg__msg__robot_pose
            cls._TYPE_SUPPORT = module.type_support_msg__msg__robot_pose
            cls._DESTROY_ROS_MESSAGE = module.destroy_ros_message_msg__msg__robot_pose

            from geometry_msgs.msg import Pose
            if Pose.__class__._TYPE_SUPPORT is None:
                Pose.__class__.__import_type_support__()

    @classmethod
    def __prepare__(cls, name, bases, **kwargs):
        # list constant names here so that they appear in the help text of
        # the message class under "Data and other attributes defined here:"
        # as well as populate each message instance
        return {
        }


class RobotPose(metaclass=Metaclass_RobotPose):
    """Message class 'RobotPose'."""

    __slots__ = [
        '_robot_pose',
        '_covariance_score',
        '_current_trajectory',
        '_last_update_pose',
        '_last_update_duration',
    ]

    _fields_and_field_types = {
        'robot_pose': 'geometry_msgs/Pose',
        'covariance_score': 'float',
        'current_trajectory': 'string',
        'last_update_pose': 'geometry_msgs/Pose',
        'last_update_duration': 'float',
    }

    SLOT_TYPES = (
        rosidl_parser.definition.NamespacedType(['geometry_msgs', 'msg'], 'Pose'),  # noqa: E501
        rosidl_parser.definition.BasicType('float'),  # noqa: E501
        rosidl_parser.definition.UnboundedString(),  # noqa: E501
        rosidl_parser.definition.NamespacedType(['geometry_msgs', 'msg'], 'Pose'),  # noqa: E501
        rosidl_parser.definition.BasicType('float'),  # noqa: E501
    )

    def __init__(self, **kwargs):
        assert all('_' + key in self.__slots__ for key in kwargs.keys()), \
            'Invalid arguments passed to constructor: %s' % \
            ', '.join(sorted(k for k in kwargs.keys() if '_' + k not in self.__slots__))
        from geometry_msgs.msg import Pose
        self.robot_pose = kwargs.get('robot_pose', Pose())
        self.covariance_score = kwargs.get('covariance_score', float())
        self.current_trajectory = kwargs.get('current_trajectory', str())
        from geometry_msgs.msg import Pose
        self.last_update_pose = kwargs.get('last_update_pose', Pose())
        self.last_update_duration = kwargs.get('last_update_duration', float())

    def __repr__(self):
        typename = self.__class__.__module__.split('.')
        typename.pop()
        typename.append(self.__class__.__name__)
        args = []
        for s, t in zip(self.__slots__, self.SLOT_TYPES):
            field = getattr(self, s)
            fieldstr = repr(field)
            # We use Python array type for fields that can be directly stored
            # in them, and "normal" sequences for everything else.  If it is
            # a type that we store in an array, strip off the 'array' portion.
            if (
                isinstance(t, rosidl_parser.definition.AbstractSequence) and
                isinstance(t.value_type, rosidl_parser.definition.BasicType) and
                t.value_type.typename in ['float', 'double', 'int8', 'uint8', 'int16', 'uint16', 'int32', 'uint32', 'int64', 'uint64']
            ):
                if len(field) == 0:
                    fieldstr = '[]'
                else:
                    assert fieldstr.startswith('array(')
                    prefix = "array('X', "
                    suffix = ')'
                    fieldstr = fieldstr[len(prefix):-len(suffix)]
            args.append(s[1:] + '=' + fieldstr)
        return '%s(%s)' % ('.'.join(typename), ', '.join(args))

    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            return False
        if self.robot_pose != other.robot_pose:
            return False
        if self.covariance_score != other.covariance_score:
            return False
        if self.current_trajectory != other.current_trajectory:
            return False
        if self.last_update_pose != other.last_update_pose:
            return False
        if self.last_update_duration != other.last_update_duration:
            return False
        return True

    @classmethod
    def get_fields_and_field_types(cls):
        from copy import copy
        return copy(cls._fields_and_field_types)

    @builtins.property
    def robot_pose(self):
        """Message field 'robot_pose'."""
        return self._robot_pose

    @robot_pose.setter
    def robot_pose(self, value):
        if __debug__:
            from geometry_msgs.msg import Pose
            assert \
                isinstance(value, Pose), \
                "The 'robot_pose' field must be a sub message of type 'Pose'"
        self._robot_pose = value

    @builtins.property
    def covariance_score(self):
        """Message field 'covariance_score'."""
        return self._covariance_score

    @covariance_score.setter
    def covariance_score(self, value):
        if __debug__:
            assert \
                isinstance(value, float), \
                "The 'covariance_score' field must be of type 'float'"
            assert not (value < -3.402823466e+38 or value > 3.402823466e+38) or math.isinf(value), \
                "The 'covariance_score' field must be a float in [-3.402823466e+38, 3.402823466e+38]"
        self._covariance_score = value

    @builtins.property
    def current_trajectory(self):
        """Message field 'current_trajectory'."""
        return self._current_trajectory

    @current_trajectory.setter
    def current_trajectory(self, value):
        if __debug__:
            assert \
                isinstance(value, str), \
                "The 'current_trajectory' field must be of type 'str'"
        self._current_trajectory = value

    @builtins.property
    def last_update_pose(self):
        """Message field 'last_update_pose'."""
        return self._last_update_pose

    @last_update_pose.setter
    def last_update_pose(self, value):
        if __debug__:
            from geometry_msgs.msg import Pose
            assert \
                isinstance(value, Pose), \
                "The 'last_update_pose' field must be a sub message of type 'Pose'"
        self._last_update_pose = value

    @builtins.property
    def last_update_duration(self):
        """Message field 'last_update_duration'."""
        return self._last_update_duration

    @last_update_duration.setter
    def last_update_duration(self, value):
        if __debug__:
            assert \
                isinstance(value, float), \
                "The 'last_update_duration' field must be of type 'float'"
            assert not (value < -3.402823466e+38 or value > 3.402823466e+38) or math.isinf(value), \
                "The 'last_update_duration' field must be a float in [-3.402823466e+38, 3.402823466e+38]"
        self._last_update_duration = value
