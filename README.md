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

Python installation
====================
Documenting installation on Manjaro linux.

Needed libraries
-----------------
- Open CV

sudo pacman -Sy opencv
- python-pyexiftool

yaourt python-pyexiftool
- python-pandas

sudo pacman -Sy python2-pandas
- python-matplotlib

sudo pacman -Sy python2-matplotlib
- python2-xlrd

sudo pacman -Sy python2-xlrd
- python2-scipy

sudo pacman -Sy python2-scipy

CPP libraries installation
===========================

Needed libraries
----------------
- armadillo

yaourt armadillo

To compile background_estimation_code on manjaro/archlinux 
------------------------------------------------------------

- cd cpplibs/background_estimation_code

g++ -L/usr/lib64 -L/usr/lib -I/usr/include -I/usr/include/opencv  main.cpp SequentialBge.cpp SequentialBgeParams.cpp -O3   -larmadillo -lopencv_core -lopencv_highgui -fopenmp -o "EstimateBackground"

To compile foreground_detection_code on manjaro/archlinux
------------------------------------------------------------

- cd cpplibs/foreground_estimation_code

g++ -o ForegroundSegmentation main.cpp input_preprocessor.cpp -O2 -fopenmp -I/usr/include/opencv -I/usr/include/opencv -L/usr/lib64 -L/usr/lib -larmadillo -lopencv_core -lopencv_highgui -lopencv_imgproc

Usage
=====

You first need to generate the masks with generatemasks.py. Once you have your masks, you can run foxcountRW.py on theses masks. See code for detailed documentation.
