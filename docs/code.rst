==============
FoxMask module
==============

.. automodule:: foxmask
   :members:

Gathering folders to analyze
============================

.. autofunction:: getfolders

Creating results folders
========================

.. autofunction:: makeresultsfolder


Getting images information
==========================

FoxMask will create a dictionary of classes
containing all methods and attributes needed
to analyze images in each found directory. Thus, the
:class:`foxmask.Getimagesinfo` class will be stored
in a dictionary, one class for each directory being analyzed.
This class is the parent of :class:`foxmask.Imagesanalysis`

.. autoclass:: foxmask.Getimagesinfos
   :members: __init__, getimageslist, getimagesmeta, sortimages,
             getimpg

Analyzing images
================

The following class constitue the core code
of the image analysis. It uses two external cpp
libraries, coded by Vikkas Reddy:

* `EstimateBackground`_
* `ForegroundSegmentation`_.

The two libraries have been modified to be intragrated
with FoxMask and to be able to run successful analysis on
a small set of images (e.g. 5). The EstimateBackground
code was performing well on a small set, but we could not get
the ForegroundSegmentation code, which does it's own background
estimation, to perform well on small sets. So we used the
EstimateBackround code to feed the ForegroundSegmentation in
it's background estimation. We accomplish that simply by
duplicating the detected backround and configured the
ForegroundSegmentation code to use theses images as
training images.

.. _estimatebackground: http://ieeexplore.ieee.org/document/5289348/
.. _foregroundsegmentation: https://arxiv.org/abs/1303.4160
.. autoclass:: foxmask.Imagesanalysis
   :members: bgfgestimation, masks_analysis, getmaskslist,
             writeresults
