<center> <h1>FoxMask</h1> </center>

What is it ?
==============
FoxMask has been created to give researchers a tool to
analyze images taken by RECONYX cameras. Theses cameras
are triggered by a motion detection system and


detection and subtraction. It is written in Python and C++.
The algorithm has been written to be able to analyse

Documentation
=============
The documentation available as of the date of this release is
included in the docs/ directory.

Installation
=============
This code is intended to be used with an Ubuntu 14 Vagrant box.
To install the FoxMask Vagrant box on your computer, clone this
repository and run *vagrant up*.

- git clone https://github.com/edevost/foxmask.git
- cd foxmask
- vagrant up

Licensing
=========
Please see the file called LICENSE

Contact
=======


Dependencies
============
This software is intended to be used on a virtual machine with vagrant.
Needed dependencies to get and run the software:
- [Virtualbox](https://www.virtualbox.org/)
- [Vagrant](https://www.vagrantup.com/)
- [git](https://git-scm.com/)


Software structure
===================
This software consist of two main python components

- generatemasks.py
- foxcountRW.py

and two cpp libraries

- background_estimation_code
- foreground_detection_code

Usage
=====
This software is shipped with a sample images directory, located
in the Images folder where you will find 100 images of arctic fox den.
You first need to generate the masks with generatemasks.py.
- python generatemasks.py

This will take as input the sample images located in the Images folder.

The masks will then be
masks, you can run foxcountRW.py on theses masks. See code for detailed documentation.
