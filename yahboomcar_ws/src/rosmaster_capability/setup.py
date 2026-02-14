from setuptools import find_packages, setup
import os
from glob import glob

package_name = 'rosmaster_capability'

setup(
    name=package_name,
    version='0.1.0',
    packages=find_packages(),
    data_files=[
        ('share/ament_index/resource_index/packages', ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        (os.path.join('share', package_name, 'launch'), glob('launch/*.py')),
        (os.path.join('share', package_name, 'config'), glob('config/*.yaml')),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='sammydev295',
    maintainer_email='sammydev295@gmail.com',
    description='Capability facade node for Rosmaster X3+',
    license='MIT',
    entry_points={
        'console_scripts': [
            'capability_node = rosmaster_capability.capability_node:main',
        ],
    },
)
