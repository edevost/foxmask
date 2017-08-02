==================
Code documentation
==================

.. automodule:: foxmask

Setup
=====

The following class will do all the preliminary
setup required to run the code and clean
temporary files and directories. One variable
residing in ``parameters.py`` can be parametrised:
**rmmasks**, defining if resulting masks should be
kept. This could be useful, for example, for visual
evaluation of the analysis.


.. autoclass:: Setup
   :members: getfolders, maketempdir, delmaskresults

Images to analyze and metadata
==============================

The following class will gather all images in the folder
being analysed, and search in each image metadata
for the ``DateTimeOriginal``, which is the time the picture
was taken. Based on the latter, groups of images are made.

.. autoclass:: Getimagesinfos
   :members: getimageslist, getimagesmeta, sortimages,
             getimpg
