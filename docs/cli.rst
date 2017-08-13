==========
Cli module
==========

This module is the entry point of the
FoxMask package. When ``foxmask`` is invoke
from the command line, this module is executed.
The entry point is specified in the ``setup.py``
file, as follows:

.. literalinclude:: ../setup.py
   :language: python
   :lines: 313-315

.. autofunction:: cli.main
