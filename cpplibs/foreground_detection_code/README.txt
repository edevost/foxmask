----------------------
Citation Details
----------------------
  
Please cite the following journal article when using this source code:
  
  V. Reddy, C. Sanderson, B.C. Lovell.
  Improved Foreground Detection via Block-based Classifier Cascade with Probabilistic Decision Integration.
  IEEE Transactions on Circuits and Systems for Video Technology, Vol. 23, No. 1, pp. 83-93, 2013.
  
  DOI: 10.1109/TCSVT.2012.2203199
  
You can obtain a copy of this article via:
http://dx.doi.org/10.1109/TCSVT.2012.2203199

    

----------------------
License
----------------------
  
The source code is provided without any warranty of fitness for any purpose.
You can redistribute it and/or modify it under the terms of the
GNU General Public License (GPL) as published by the Free Software Foundation,
either version 3 of the License or (at your option) any later version.
A copy of the GPL license is provided in the "GPL.txt" file.



----------------------
Instructions and Notes
----------------------

To run the code the following libraries must be installed:
1. OpenCV 2.4 (later versions should also work)
2. Armadillo 3.920 - http://arma.sourceforge.net


Under Linux, to compile the code use the following command:
g++ -o ForegroundSegmentation main.cpp input_preprocessor.cpp -O2 -fopenmp -I/usr/include/opencv -I/usr/local/include/opencv -L/usr/lib64 -L/usr/local/lib -larmadillo -lopencv_core -lopencv_highgui -lopencv_imgproc

You may need to adapt the paths for libraries and includes to suit your environment.
The above command line has been tested on Fedora 19, using Armadillo 3.920.2 and OpenCV 2.4.6

After successful compilation, to execute the code, run the following command: 
./ForegroundSegmentation  <set input path sequence>   <sequence name>
For example:
./ForegroundSegmentation  /home/Project/datasets/UCSD/seq1/  seq1 


Points to note:

1.
Supported input formats: png, jpeg, bmp and tif.

2.
Internally, the code sorts the input image files of a given folder in ascending order.
Hence, the file names must contain a constant number of digits in their suffixes
(eg. test_0001, test_0002, test_0100, test_1000,...).

3.
Initially, the algorithm uses first 200 frames to build a model of the background
before producing foreground mask for each frame. 

4.
To save the masks, WRITEMASK must be defined in main.hpp (by default, this is defined).
An output folder is automatically created to store all the generated foreground masks.
The output masks are stored as png images.

5.
The code is currently not optimised for speed.


