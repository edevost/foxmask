==============
FoxMask module
==============

.. automodule:: foxmask

Gathering folders to analyze
============================

.. autofunction:: getfolders

Creating results folders
========================

.. autofunction:: makeresultsfolder


Building methods and attributes
===============================

FoxMask will create a dictionary of classes
containing all methods and attributes needed
to analyze images in each found folders. Thus, the
:class:`foxmask.Getimagesinfo` class will be stored
in a dictionary, one class for each folder being analyzed.

.. autoclass:: foxmask.Getimagesinfos
   :members: __init__, getimageslist, getimagesmeta, sortimages,
             getimpg
