<center> <h1>FoxMask</h1> </center>

What is it ?
==============
FoxMask has been created to give researchers a tool to
analyze images taken by RECONYX cameras. Theses cameras
are triggered by a motion detection system and FoxMask has
been written to be able to isolate moving object with very
short image sequences (minimum of 5). It is written in
Python and C++.

Documentation
=============
The documentation available as of the date of this release is
included in the docs/ directory.

Installation
=============
This code is intended to be used with an Ubuntu 14 Vagrant box.
Thus, to run the software, you need to install Virtualbox and Vagrant:
- [Virtualbox](https://www.virtualbox.org/)
- [Vagrant](https://www.vagrantup.com/)

To clone this repository, you need git:
- [git](https://git-scm.com/)

Once theses dependencies are installed you can proceed to install
the FoxMask box on your computer. Simply clone this repository
repository and run *vagrant up*.

- git clone https://github.com/edevost/foxmask.git
- cd foxmask
- vagrant up

Licensing
=========
Please see the file called LICENSE

Contact
=======
Feel free to contact me if you have any comments or questions.
ericdevost@gmail.com

Usage
=====
This software is shipped with a sample images directory, located
in the Images folder where you will find 100 images of arctic fox den.
You first need to generate the masks with generatemasks.py.
- python generatemasks.py

This will take as input the sample images located in the Images folder
and creates Masks in the MasksResults folder. Once the masks are
generated, you can run foxcount.py to count the foxes on the images.
