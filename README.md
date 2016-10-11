pyclassifier2
==============
Python classifier to analyse images and count objects based on background detection and subtraction.

Software structure
===================
This sofware consist of two main python components

- generatemasks.py
- foxcountRW.py

and two cpp libraries

- background_estimation_code
- foreground_detection_code

Installation
=============
As is, this code is intended to use with a Ubuntu 14 Vagrant box.


Usage
=====

You first need to generate the masks with generatemasks.py. Once you have your
masks, you can run foxcountRW.py on theses masks. See code for detailed documentation.
