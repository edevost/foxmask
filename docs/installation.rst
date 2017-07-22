.. _installation:

============
Installation
============

We provide two methods for installing and using FoxMask:

* :ref:`Installation through a virtual machine` (Any platform)
* :ref:`Standalone installation on Linux`

.. _installation through a virtual machine:

Installation through a virtual machine
======================================

We provide a complete virtual environment to run and
use FoxMask. The main advantages of this installation
is that there is no installation or configuration
needed, apart of course the needed virtual environment.
It can also be easily deployed on local virtual
machines infrastructure or on cloud computing services
as Amazon or Google cloud.

To run the virtual machine on your computer and to use
FoxMask, you will need the following three open source
softwares:

* `VirtualBox`_
* `Vagrant`_
* `Git`_

.. _virtualbox: https://www.virtualbox.org/
.. _vagrant: https://www.vagrantup.com/
.. _git: https://git-scm.com/

Follow the installation procedure for your platform.
Once theses softwares are installed, you are ready
to use FoxMask by issuing the following command:

.. code-block:: console

   git clone https://github.com/edevost/foxmask.git

This will fetch all needed components to run the virtual
machine. Once the clone command is completed, you can
start the virtual machine by issuing the following commands:

.. code-block:: console

   cd foxmask
   vagrant up

At the end of the installation, the virtual box is automatically launched.
If not, open VirtualBox and run the FoxMask Box by 1) selecting the box, and 2)
clicking on the Start button *(Figure1)* . You will see the Ubuntu 14.04
interface. In this virtual environment, your user name is vagrant and the
associated password is vagrant.

.. _figure1:

Figure1
  .. figure:: images/screenshot-1.png

     Screenshot example of the VirtualBox manager with the
     newly created foxmask virtual machine.

Issues on MacOS
---------------

The shared folder on Vagrant does not behave as expected on MacOS.
We need to configure the vm to share the folder correctly with the
host machine.

Shared folder
~~~~~~~~~~~~~

The shared folder is used to make the cloned FoxMask repository
accessible to the virtual machine. This is very useful, for example,
to easily make images needed to be analyzed available to the
virtual machine.

To activate the shared folder, open VirtualBox manager
and access the FoxMask Box settings:

.. _figure2:

Figure2
  .. figure:: images/screenshot-2.png

    Screenshot showing how to access the settings in VirtualBox manager.

Then, make sure the settings look like in *Figure3*

Figure3
  .. figure:: images/screenshot-3.png

     Screenshot showing the settings of the shared folder on
     VirtualBox manager.

To use FoxMask, it has to be located in the ``/vagrant`` folder
on the virtual machine. However, this folder is not shared
as expected, but instead reside in ``/media/sf_vagrant``, which
is the actual shared folder. A symbolic link can be created
on the Ubuntu box between the two:

.. code-block:: console

   sudo rm -rf /vagrant
   sudo ln -s /media/sf_vagrant /vagrant
   sudo chown vagrant /vagrant

Restart your VM and should be ready to use FoxMask! Consult
our :ref:`usage` page to get started.


.. _standalone installation on Linux:

Standalone installation on Linux
================================

This method will install FoxMask on your Linux Box.
We provide here commands for Ubuntu, but this installation
should work on any distro. It has been tested on Ubunutu 16.04.
We recommend this installation for users wishing to contribute
to the FoxMask's code or to the ones interested in implementing
FoxMask in their lab on bare metal hardware, or to build a
FosMasks's cluster !


Requirements
------------

* `Armadillo`_
* `OpenCV2`_
* `Pyexiftool`_

Armadillo
~~~~~~~~~

Before installing armadillo on Ubunutu, make sure you install
the recommended dependencies

.. code-block:: console

   sudo apt-get install cmake, libblas-dev, liblapack-dev, libarpack-dev


Get and unzip armadillo from their web site to compile the library.
Make sure you are in the armadillo folder created when you unziped
the archive.

.. code-block:: console

   make
   sudo make install


OpenCV2
~~~~~~~

The FoxMask code has not yet been ported to use the latest OpenCV libraries, so
we need to `install OpenCV2`_.

.. code-block:: console

   sudo apt-get install libjpeg8-dev libtiff5-dev libjasper-dev libpng12-dev libavcodec-dev libavformat-dev libswscale-dev libv4l-dev
   wget http://sourceforge.net/projects/opencvlibrary/files/opencv-unix/2.4.9/opencv-2.4.9.zip
   unzip opencv-2.4.9.zip
   cd opencv-2.4.9
   mkdir build
   cd build
   cmake -D WITH_TBB=ON -D BUILD_NEW_PYTHON_SUPPORT=ON -D WITH_V4L=ON -D INSTALL_C_EXAMPLES=ON -D INSTALL_PYTHON_EXAMPLES=ON -D BUILD_EXAMPLES=ON -D WITH_QT=ON -D WITH_OPENGL=ON -D WITH_VTK=ON ..


.. _install opencv2: http://www.samontab.com/web/2014/06/installing-opencv-2-4-9-in-ubuntu-14-04-lts/

Pyexiftool
~~~~~~~~~~

Python module needed to read the images metadata.

.. code-block:: console

   git clone git://github.com/smarnach/pyexiftool.git
   cd pyexiftool
   sudo python2 setup.py install



.. _armadillo: http://arma.sourceforge.net/download.html
.. _opencv2: http://docs.opencv.org/2.4.13.2/
.. _pyexiftool: https://github.com/smarnach/pyexiftool


Installing FoxMask
------------------

Once the dependencies have been installed, you are
ready to install FoxMask on your computer. First
clone the FoxMaks repository:

.. code-block:: console

   git clone https://github.com/edevost/foxmask.git


Once the repo has been cloned, you need to compile
the two cpp libraries used to detect the background
and perform a foreground segmentation on images.Theses
libraries need to be compiled and liked to your version
of Armadillo.

.. code-block:: console

   cd ~/foxmask/cpplibs/background_estimation_code/code/
   g++ -L/usr/lib64 -L/usr/lib -I/usr/include -I/usr/local/include/opencv main.cpp SequentialBge.cpp SequentialBgeParams.cpp -O3 -larmadillo -lopencv_core -lopencv_highgui -fopenmp -o "EstimateBackground"
   cd ~/foxmask/cpplibs/foreground_detection_code/code/
   g++ -o ForegroundSegmentation main.cpp input_preprocessor.cpp -O2 -fopenmp -I/usr/include/opencv -I/usr/local/include/opencv -L/usr/lib64 -L/usr/local/lib -larmadillo -lopencv_core -lopencv_highgui -lopencv_imgproc

Finaly, install the needed python libraries needed by FoxMask:

.. code-block:: console

   cd ~/foxmask
   python2 -m pip install -r requirements.txt --user

Congratulation, you are now ready to start using FoxMask
on your Linux Box ! Consult our :ref:`usage` page to get
started.
