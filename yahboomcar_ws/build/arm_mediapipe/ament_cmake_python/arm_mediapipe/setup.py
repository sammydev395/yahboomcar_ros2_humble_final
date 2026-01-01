from setuptools import find_packages
from setuptools import setup

setup(
    name='arm_mediapipe',
    version='0.1.0',
    packages=find_packages(
        include=('arm_mediapipe', 'arm_mediapipe.*')),
)
