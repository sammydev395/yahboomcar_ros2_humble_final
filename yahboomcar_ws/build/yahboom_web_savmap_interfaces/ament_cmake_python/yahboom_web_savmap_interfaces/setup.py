from setuptools import find_packages
from setuptools import setup

setup(
    name='yahboom_web_savmap_interfaces',
    version='0.0.0',
    packages=find_packages(
        include=('yahboom_web_savmap_interfaces', 'yahboom_web_savmap_interfaces.*')),
)
