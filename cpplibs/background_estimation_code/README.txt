----------------------
Citation Details
----------------------
  
Please cite the following journal article when using this source code:

 V. Reddy, C. Sanderson,  B.C. Lovell.
 A Low-Complexity Algorithm for Static Background Estimation from Cluttered Image Sequences in Surveillance Contexts.
 Eurasip Journal on Image and Video Processing, 2011.
  
 http://dx.doi.org/10.1155/2011/164956


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
1. OpenCV 2.1 (later versions may also work)
2. Armadillo - http://arma.sourceforge.net


Under Linux, to compile the code use the following command:
g++ -L/usr/lib64 -L/usr/local/lib -I/usr/include -I/usr/local/include/opencv  main.cpp SequentialBge.cpp  SequentialBgeParams.cpp -O3   -larmadillo -lcv -lhighgui -fopenmp -o "EstimateBackground"


You may need to adapt the library and include paths to suit your environment.

After successful compilation, to execute the code, run the following command: 
./EstimateBackground  <set input path sequence>   <background_frame_name>
eg. ./EstimateBackground  /home/Project/datasets/UCSD/seq1/      estimated_bg 


Points to note:

1.
Supported input formats: png, jpeg, and bmp

2.
Internally, the code sorts the input image files of a given folder in ascending order.
Hence, the file names must contain a constant number of digits in their suffixes
(eg. test_0001, test_0002, test_0100, test_1000,...).

3.
To estimate the background, the algorithm uses all frames in the sequence (or upto 400 frames if the sequence is longer than that) 

4.
At the end of the execution, the estimated background frame is stored in the same folder.

5.
The code is currently not optimised for speed.


