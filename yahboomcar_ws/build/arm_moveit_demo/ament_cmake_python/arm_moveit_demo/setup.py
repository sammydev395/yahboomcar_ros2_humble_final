from setuptools import find_packages
from setuptools import setup

setup(
    name='arm_moveit_demo',
    version='0.3.0',
    packages=find_packages(
        include=('arm_moveit_demo', 'arm_moveit_demo.*')),
)
