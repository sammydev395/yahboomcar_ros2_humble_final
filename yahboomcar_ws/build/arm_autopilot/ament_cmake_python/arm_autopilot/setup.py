from setuptools import find_packages
from setuptools import setup

setup(
    name='arm_autopilot',
    version='0.1.0',
    packages=find_packages(
        include=('arm_autopilot', 'arm_autopilot.*')),
)
