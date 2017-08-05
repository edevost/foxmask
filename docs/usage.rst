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

    foxmask --foldersdir images --resultsdir .

This will launch the analysis process on the default set of images
distributed with FoxMask. Theses images resides in the ``images``
folder. The results will be written in the FoxMaskResults folder.

As the code run, you will see images being analyzed and mask created. At the end of
the process, a CSV file is exported with the image names and the presence
of detected objects (0 or 1). If required, masks and images with detected
objects are also copied in subfolders.


Running the code on your own set
--------------------------------

To run the code on your own set of images, simply specify
where the folders containing the images are located. For
example:

.. code-block:: console

   foxmask --foldersdir /media/reconyximages/2015 --resultsdir analysis/results

This will launch the analysis on all folders present in
``/media/reconyximages/2015``. There is no limits in the
number of folders that can be analyzed. Just keep in mind
that the more folders, the more the analysis will take time.
We suggest testing the software on just one folder to see
how it behaves on your images, and learn it's functioning.

The are a few constraints concerning the folders and the images
to be analyzed.

Folders naming and structure
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

All folders to be analyzed should have a descriptive name, as FoxMask
will store the results using the names of these folders. Also,
every folder should contains images and **no** sub folders.

Images naming and format
~~~~~~~~~~~~~~~~~~~~~~~~

Your images should be in the jpeg format, with the proper extension
(e.g. image-01.jpg, or image-01.jpeg). The case does not matter, as well
as the name of the image. A future version of foxmask will be able to analyze
other image format. See `issue number 45`_

.. _issue number 45: https://github.com/edevost/foxmask/issues/45

Configuration
-------------

There are a few values that can be parametrized in the
``parameters.py`` file. However, the defaults should
be sane enough for allowing testing the software on
your own set of images.

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
