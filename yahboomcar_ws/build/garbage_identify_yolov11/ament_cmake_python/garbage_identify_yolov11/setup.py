from setuptools import find_packages
from setuptools import setup

setup(
    name='garbage_identify_yolov11',
    version='0.3.0',
    packages=find_packages(
        include=('garbage_identify_yolov11', 'garbage_identify_yolov11.*')),
)
