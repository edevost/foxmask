#!/usr/bin/python3.5
# -*- coding: utf-8 -*-


"""
Setup script
"""


from setuptools import setup, find_packages

setup(
    name='foxmask',
    version='2.1',
    packages=find_packages(),
    include_package_data=True,
    install_requires=[
        'Click',
    ],
    entry_points='''
        [console_scripts]
        foxmask = foxmask.cli:cli
    ''',
)
