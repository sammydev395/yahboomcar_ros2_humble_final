from setuptools import find_packages
from setuptools import setup

setup(
    name='arm_color_transport',
    version='0.1.0',
    packages=find_packages(
        include=('arm_color_transport', 'arm_color_transport.*')),
)
