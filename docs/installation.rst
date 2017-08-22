.. _installation:

============
Installation
============

We provide two methods for installing and using FoxMask:

* :ref:`Standalone installation on Linux`
* :ref:`Installation through a virtual machine` (Any platform)


.. _standalone installation on Linux:

Standalone installation on Linux
================================

**This is the recommended method for production.**

This method will install FoxMask on your Linux computer. We provide an
automated install script, which will take care of installing all the
dependencies needed to run FoxMask as well as the software itself. It is
made for **Ubuntu 16.04**. The installation script is located in
``install-scripts/install-ubuntu16.sh`` of the FoxMask repository.
To execute the script, simply clone the repository and execute the install
script as a regular user. Note that you will need to have ``sudo`` rights.


.. code-block:: console

   $ git clone https://github.com/edevost/foxmask.git
   $ cd foxmask
   $ sh install-Ubuntu16.sh

Congratulation, you are now ready to start using FoxMask
on your Linux Box ! Consult our :ref:`usage` page to get
started.


.. note::

   This installation script have been successfully tested on Ubuntu 16.04, on bare
   metal computers as well as on GCE (Google Cloud Engine) and Virtual-Box 5.0.
   Successful installation have also been performed on Debian Stretch and
   Ubuntu 14.04 container (TravisCI).

.. _installation through a virtual machine:


Installation through a virtual machine
======================================

**We do not recommend this installation for production.**

We provide a complete virtual environment to run and
use FoxMask. The main advantages of this installation
is that you can rapidly get FoxMask running on any platform. However,
there are important drawbacks to consider:

* Poor performances compared to standalone installation
* A bit heavy to manage (Vagrant + Virtual-Box + share USB)

Despite theses drawbacks, we consider that using
our FoxMask box can be very useful for first time users
wanting to test the software, or for development purposes.


.. note::
   Again, let me emphasize that this method is suitable to rapidly
   get FoxMask up and running on another platform than Ubuntu. However, anyone
   serious about implementing an automated image analysis pipeline with
   FoxMask should consider installing it on bare metal hardware running
   a Linux OS, or on a virtual infrastructure running *libvirt*. We provide
   a fully automated installation script, successfully
   tested on *Ubuntu 16.04*. Please see :ref:`standalone installation on Linux`
   to install FoxMask on you Linux machine.

To run the FoxMask virtual machine on your Linux, Mac or Windows computer,
you will need the following three open source software:

* `Virtual-Box`_
* `Vagrant`_
* `Git`_

.. _virtual-box: https://www.virtualbox.org/
.. _vagrant: https://www.vagrantup.com/
.. _git: https://git-scm.com/

Follow the installation procedure for your platform.
Once theses software are installed, you are ready
to install FoxMask by issuing the following command:


.. code-block:: console

   git clone https://github.com/edevost/foxmask.git


This will fetch all needed components to run the virtual
machine. Once the clone command is completed, you can
start the virtual machine by issuing the following commands:

.. code-block:: console

   cd foxmask
   vagrant up

This will boot the virtual machine. You can also manage you
virtual machine through Virtual-Box software, and start or
stop your machine from there. The start process will bring you to
the Ubuntu desktop. The default user and password on this
virtual environment is vagrant vagrant. Once the machine is ready, you can
proceed to our :ref:`usage` section to launch FoxMask on the provided
set of images.

Testing on your own set of images
---------------------------------

To test FoxMask on your own set of images, you will have to make your
images available to the virtual machine. To do so, we recommend saving
your image set on an USB drive, and make the drive available to Virtual-Box.
There are many good tutorials on the web explaining the process:

* http://www.dedoimedo.com/computers/virtualbox-usb.html
* https://www.groovypost.com/howto/mount-usb-drive-virtualbox/
* https://techtooltip.wordpress.com/2008/09/22/how-to-use-host-usb-device-from-guest-in-virtual-box/

Making this work will demand a bit of work and reading, but nothing out of
reach of any computer users with minimal computer knowledge. The main steps
to get your FoxMask virtual machine to have access to an USB drive plugged
on your host computer are the following:

* Shutdown your FoxMask virtual machine if it is running
* Plug your USB drive on your computer (host)
* Install `Virtual Box 5.0`_ extension pack
* Activate USB controller on Virtual-Box
* Add your drive

Once your drive is added, boot your FoxMask virtual machine
and check if there is a link to your drive on the desktop.
If everything went well, you will have direct access to your
USB drive on your virtual machine ! You can then proceed to our
:ref:`usage` section to start using FoxMask on your set of images.

.. _virtual box 5.0: https://www.virtualbox.org/wiki/Download_Old_Builds_5_0
