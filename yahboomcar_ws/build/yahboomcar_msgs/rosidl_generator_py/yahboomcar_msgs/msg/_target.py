# generated from rosidl_generator_py/resource/_idl.py.em
# with input from yahboomcar_msgs:msg/Target.idl
# generated code does not contain a copyright notice


# Import statements for member types

import builtins  # noqa: E402, I100

import math  # noqa: E402, I100

import rosidl_parser.definition  # noqa: E402, I100


class Metaclass_Target(type):
    """Metaclass of message 'Target'."""

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
            module = import_type_support('yahboomcar_msgs')
        except ImportError:
            import logging
            import traceback
            logger = logging.getLogger(
                'yahboomcar_msgs.msg.Target')
            logger.debug(
                'Failed to import needed modules for type support:\n' +
                traceback.format_exc())
        else:
            cls._CREATE_ROS_MESSAGE = module.create_ros_message_msg__msg__target
            cls._CONVERT_FROM_PY = module.convert_from_py_msg__msg__target
            cls._CONVERT_TO_PY = module.convert_to_py_msg__msg__target
            cls._TYPE_SUPPORT = module.type_support_msg__msg__target
            cls._DESTROY_ROS_MESSAGE = module.destroy_ros_message_msg__msg__target

            from builtin_interfaces.msg import Time
            if Time.__class__._TYPE_SUPPORT is None:
                Time.__class__.__import_type_support__()

    @classmethod
    def __prepare__(cls, name, bases, **kwargs):
        # list constant names here so that they appear in the help text of
        # the message class under "Data and other attributes defined here:"
        # as well as populate each message instance
        return {
        }


class Target(metaclass=Metaclass_Target):
    """Message class 'Target'."""

    __slots__ = [
        '_frame_id',
        '_stamp',
        '_scores',
        '_ptx',
        '_pty',
        '_distw',
        '_disth',
        '_centerx',
        '_centery',
    ]

    _fields_and_field_types = {
        'frame_id': 'string',
        'stamp': 'builtin_interfaces/Time',
        'scores': 'float',
        'ptx': 'float',
        'pty': 'float',
        'distw': 'float',
        'disth': 'float',
        'centerx': 'float',
        'centery': 'float',
    }

    SLOT_TYPES = (
        rosidl_parser.definition.UnboundedString(),  # noqa: E501
        rosidl_parser.definition.NamespacedType(['builtin_interfaces', 'msg'], 'Time'),  # noqa: E501
        rosidl_parser.definition.BasicType('float'),  # noqa: E501
        rosidl_parser.definition.BasicType('float'),  # noqa: E501
        rosidl_parser.definition.BasicType('float'),  # noqa: E501
        rosidl_parser.definition.BasicType('float'),  # noqa: E501
        rosidl_parser.definition.BasicType('float'),  # noqa: E501
        rosidl_parser.definition.BasicType('float'),  # noqa: E501
        rosidl_parser.definition.BasicType('float'),  # noqa: E501
    )

    def __init__(self, **kwargs):
        assert all('_' + key in self.__slots__ for key in kwargs.keys()), \
            'Invalid arguments passed to constructor: %s' % \
            ', '.join(sorted(k for k in kwargs.keys() if '_' + k not in self.__slots__))
        self.frame_id = kwargs.get('frame_id', str())
        from builtin_interfaces.msg import Time
        self.stamp = kwargs.get('stamp', Time())
        self.scores = kwargs.get('scores', float())
        self.ptx = kwargs.get('ptx', float())
        self.pty = kwargs.get('pty', float())
        self.distw = kwargs.get('distw', float())
        self.disth = kwargs.get('disth', float())
        self.centerx = kwargs.get('centerx', float())
        self.centery = kwargs.get('centery', float())

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
        if self.frame_id != other.frame_id:
            return False
        if self.stamp != other.stamp:
            return False
        if self.scores != other.scores:
            return False
        if self.ptx != other.ptx:
            return False
        if self.pty != other.pty:
            return False
        if self.distw != other.distw:
            return False
        if self.disth != other.disth:
            return False
        if self.centerx != other.centerx:
            return False
        if self.centery != other.centery:
            return False
        return True

    @classmethod
    def get_fields_and_field_types(cls):
        from copy import copy
        return copy(cls._fields_and_field_types)

    @builtins.property
    def frame_id(self):
        """Message field 'frame_id'."""
        return self._frame_id

    @frame_id.setter
    def frame_id(self, value):
        if __debug__:
            assert \
                isinstance(value, str), \
                "The 'frame_id' field must be of type 'str'"
        self._frame_id = value

    @builtins.property
    def stamp(self):
        """Message field 'stamp'."""
        return self._stamp

    @stamp.setter
    def stamp(self, value):
        if __debug__:
            from builtin_interfaces.msg import Time
            assert \
                isinstance(value, Time), \
                "The 'stamp' field must be a sub message of type 'Time'"
        self._stamp = value

    @builtins.property
    def scores(self):
        """Message field 'scores'."""
        return self._scores

    @scores.setter
    def scores(self, value):
        if __debug__:
            assert \
                isinstance(value, float), \
                "The 'scores' field must be of type 'float'"
            assert not (value < -3.402823466e+38 or value > 3.402823466e+38) or math.isinf(value), \
                "The 'scores' field must be a float in [-3.402823466e+38, 3.402823466e+38]"
        self._scores = value

    @builtins.property
    def ptx(self):
        """Message field 'ptx'."""
        return self._ptx

    @ptx.setter
    def ptx(self, value):
        if __debug__:
            assert \
                isinstance(value, float), \
                "The 'ptx' field must be of type 'float'"
            assert not (value < -3.402823466e+38 or value > 3.402823466e+38) or math.isinf(value), \
                "The 'ptx' field must be a float in [-3.402823466e+38, 3.402823466e+38]"
        self._ptx = value

    @builtins.property
    def pty(self):
        """Message field 'pty'."""
        return self._pty

    @pty.setter
    def pty(self, value):
        if __debug__:
            assert \
                isinstance(value, float), \
                "The 'pty' field must be of type 'float'"
            assert not (value < -3.402823466e+38 or value > 3.402823466e+38) or math.isinf(value), \
                "The 'pty' field must be a float in [-3.402823466e+38, 3.402823466e+38]"
        self._pty = value

    @builtins.property
    def distw(self):
        """Message field 'distw'."""
        return self._distw

    @distw.setter
    def distw(self, value):
        if __debug__:
            assert \
                isinstance(value, float), \
                "The 'distw' field must be of type 'float'"
            assert not (value < -3.402823466e+38 or value > 3.402823466e+38) or math.isinf(value), \
                "The 'distw' field must be a float in [-3.402823466e+38, 3.402823466e+38]"
        self._distw = value

    @builtins.property
    def disth(self):
        """Message field 'disth'."""
        return self._disth

    @disth.setter
    def disth(self, value):
        if __debug__:
            assert \
                isinstance(value, float), \
                "The 'disth' field must be of type 'float'"
            assert not (value < -3.402823466e+38 or value > 3.402823466e+38) or math.isinf(value), \
                "The 'disth' field must be a float in [-3.402823466e+38, 3.402823466e+38]"
        self._disth = value

    @builtins.property
    def centerx(self):
        """Message field 'centerx'."""
        return self._centerx

    @centerx.setter
    def centerx(self, value):
        if __debug__:
            assert \
                isinstance(value, float), \
                "The 'centerx' field must be of type 'float'"
            assert not (value < -3.402823466e+38 or value > 3.402823466e+38) or math.isinf(value), \
                "The 'centerx' field must be a float in [-3.402823466e+38, 3.402823466e+38]"
        self._centerx = value

    @builtins.property
    def centery(self):
        """Message field 'centery'."""
        return self._centery

    @centery.setter
    def centery(self, value):
        if __debug__:
            assert \
                isinstance(value, float), \
                "The 'centery' field must be of type 'float'"
            assert not (value < -3.402823466e+38 or value > 3.402823466e+38) or math.isinf(value), \
                "The 'centery' field must be a float in [-3.402823466e+38, 3.402823466e+38]"
        self._centery = value
