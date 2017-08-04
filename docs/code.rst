==================
Code documentation
==================

.. automodule:: foxmask

Setup
=====

Images to analyze and metadata
==============================

The following class will gather all images in the folder
being analyzed, and search in each image metadata
for the ``DateTimeOriginal`` flag, which is the time the picture
was taken. Based on the latter, groups of images are made.

.. autoclass:: foxmask.Getimagesinfos
   :members: getimageslist, getimagesmeta, sortimages,
             getimpg
