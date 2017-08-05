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
to be analyzed:

Folders naming and structure
  All folders to be analyzed should have a descriptive name, as FoxMask
  will store the results using the names of these folders. Also,
  every folder should contains images and **no** sub folders.

Images naming and format
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

maxgap
  The maximum time gap (in seconds) between two consecutive images
  to be considered as part of the same sequence. This parameter
  is set by default to 5 seconds. Under conditions where the
  background is dynamic (e.g. movements in static objects occurs),
  the ``maxgap`` parameter should not be raised very high. However,
  under conditions where the background is very static, higher values
  might be best.

minsize
  Minimum size of moving objects to be considered as an animal. This
  value is set by default to 500 pixels. You will need to test this
  parameters on your images to determine the best value. Keep in mind
  that with lower values, the risk of getting false positive is higher and
  with higher value, the risk of getting false negative is higher.
