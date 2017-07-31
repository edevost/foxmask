.. _usage:

=====
Usage
=====

FoxMask program is shipped with a sample images directory (images/example/)
where you will find 50 jpeg images of Arctic fox. We encourage you to test
FoxMask on this sample of images.

To run FoxMask on the default provided set of images, you
simply have to run the following command, from the ``foxmask``
directory:

.. code-block:: console

   python2 foxmask.py

This will launch the analysis process. You will see images being
analyzed and mask created. At the end of the process, a CSV file is
exported with the image names and the presence of detected objects (0 or 1).
If required, masks and images with detected objects are also copied in subfolders.

Configuration
-------------

Configuration of where the images sets are as well as where
to write results are all confined in the file ``parameters.py``.
In this file you will find configurable variables used when
the program is executed:

imagesDir
  The directory where the images to analyze are located. This directory
  can be anywhere (shared folder, usb drive, local disk), it just need
  to be accessible by your machine running FoxMask. It can consist of
  one single folder or a set of folders. FoxMask will iterate through
  the folders and analyze them all.

outputDir
  The location where the results of the analysis will be written

maxgap
  The maximum time gap (in seconds) between two consecutive images
  to build the sequence.

minsize
  Minimum size of moving objects to be considered as an animal
