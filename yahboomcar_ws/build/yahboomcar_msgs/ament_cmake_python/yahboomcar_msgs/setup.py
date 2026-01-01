from setuptools import find_packages
from setuptools import setup

setup(
    name='yahboomcar_msgs',
    version='0.0.0',
    packages=find_packages(
        include=('yahboomcar_msgs', 'yahboomcar_msgs.*')),
)
