.. _usage:

=====
Usage
=====

FoxMask program is shipped with a sample of images located in the
``images`` directory, where you will find two other directories (example-1
and example-2) containing JPEG images of Arctic fox. We encourage you to test
FoxMask on theses samples first. The full analysis takes around 2 minutes on
an Intel core i5.

To run FoxMask, simply invoke it from the command line.

.. code-block:: console

   $ foxmask
   Usage: foxmask [OPTIONS] SRCDIR RESULTSDIR

   Error: Missing argument "srcdir".

Without argument, the software will exit and print it's usage and
what argument is missing. The first argument [SRCDIR] is
the directory where all your directories containing images are.
The second argument [RESULTSDIR] is the directory where you want the
results to be created. For example:

.. code-block:: console

   $ foxmask images .

Will launch the software and run the analysis for all directories
present in the ``images`` directory  and create ``FoxMaskResults`` in your
current directory (.). If you are launching the application in the foxmask
directory, the command will launch the analysis process on the default
set of images distributed with FoxMask and write it's results in
``FoxMaskResults``.

As the code run, you will see images being analyzed and mask created. At the end of
the process, a CSV file is exported with the image names and the presence
of detected objects (0 or 1). If required, masks and images with detected
objects are also copied in sub directories.


Running the code on your own set
================================

To run the code on your own set of images, simply specify
where the directory containing the images are located. For
example:

.. code-block:: console

   foxmask /media/reconyximages/2015  analysis/results

This will launch the analysis on all directories present in
``/media/reconyximages/2015``. There is no limits in the
number of directory's that can be analyzed. Just keep in mind
that the more directories, the more the analysis will take time.
We suggest testing the software on just one directory to see
how it behaves on your images, and learn how FoxMask is functioning.
The results will be written in the ``analysis/results`` directory.

.. note::

   You do not need to be in the ``foxmask`` directory to run FoxMask.
   Just make sure to specify to path to your images directory relative
   to where you are launching the foxmask command, or specify an
   absolute path as in the example above. The same applies for where
   you want the results to be written.


Constraints
-----------

The are a few minor constraints concerning the directory structure containing the
images to be analyzed and the format of the images.

Directories naming and structure
  All directories to be analyzed should have a descriptive name, as FoxMask
  will store the results using the names of these directories. Also,
  every directory should contains images and **no** sub directory.

Images naming and format
  Your images should be in the JPEG format, with the proper extension
  (e.g. image_0001.jpg, or image_0002.JPG). The numbering is crucial:
  it is mandatory to use a 4 digit number prefixed by an underscore.
  The case does not matter, as well as the name of the image.
  A future version of foxmask will be able to analyze other image format.
  See `issue number 45`_

  .. note::

     We believe that it is the user's responsibility to format their images
     in an uniform way before feeding them to an analysis software.


.. _issue number 45: https://github.com/edevost/foxmask/issues/45

Configuration
=============

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


Image analysis pipeline
=======================

Setting up an image analysis pipeline should be done by using code
under version control (e.g. git). It should be 100% automated and
documented, even for small jobs. It's the only way to use efficiently
any analysis software.

The FoxMask team will implement such a pipeline in the near future,
and will make it freely available as a real word example of how
to implement FoxMask in a fully automated image analysis pipeline.
