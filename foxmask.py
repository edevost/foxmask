'''
This script will evaluate the background based on images
located, by default, in the /vagrant/foxmask/Images directory.
All images are resized (for performance issues) and the background
is evaluated. The resulting images are saved in a temporary
directory located within the /vagrant/foxmask/Images directory.
'''



'''
Importing Python libraries
'''



import csv
import sys
sys.path.append("/usr/local/lib/python2.7/site-packages")
import os
import cv2
import cv2.cv as cv
import numpy as np
from numpy import genfromtxt
import glob
from itertools import groupby
import time
from datetime import datetime,date
import shutil
import exiftool
import re
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from scipy.stats.stats import pearsonr
from scipy.stats.stats import linregress
import pylab
import random
import xlrd



from parameters import *


time1 = time.time()


'''
Creation of a temporary directory in imagesDir
'''



if not os.path.exists(imagesDir + 'temp1'):
    os.makedirs(imagesDir + 'temp1')

temp1 = os.path.join(imagesDir, 'temp1/')



'''
Specification of the location of the compiled cpp libraries
'''



cppex     = './cpplibs/background_estimation_code/code/EstimateBackground'
cppex2    = './cpplibs/foreground_detection_code/code/ForegroundSegmentation'

cppcom1   = [cppex, temp1, 'EstBG']
cppcom    = ' '.join(cppcom1)

cppcom2   = [cppex2, temp1, 'MasksResults']
paramsdir = './cpplibs/foreground_detection_code/code/'
cppfg     = ' '.join(cppcom2)

print cppfg

# In cpplibs/foreground_detection_code/code/main.cpp, you need to specify output and params.txt paths.



'''
Getting list of images
'''



imglist = []
imglist1 = glob.glob(imagesDir + '/*.JPG')
imglist2 = glob.glob(imagesDir + '/*.jpg')
imglist = imglist1 + imglist2

print 'Done Listing Images...'



'''
Compiling regular expression search in metadata
'''



date_reg_exp = re.compile('\d{4}[-/]\d{1,2}[-/]\d{1,2}\ \d{1,2}:\d{1,2}:\d{1,2}')
date_reg_expAM = re.compile('\d{4}[-/]\d{1,2}[-/]\d{1,2}\ \d{1,2}:\d{1,2}:\d{1,2}\ [AP][M]')
date_reg_exp2 = re.compile('\d{4}[:/]\d{1,2}[:/]\d{1,2}\ \d{1,2}:\d{1,2}:\d{1,2}')



'''
Getting images metadata
'''



listtags = []

with exiftool.ExifTool() as et:
    metadata = et.get_metadata_batch(imglist)
    key = 'EXIF:DateTimeOriginal'
    if key in metadata[0]:
        print 'Key found'
        tags1 = []
        for y in range(len(metadata)):
            tag = metadata[y]['EXIF:DateTimeOriginal']
            tags1.append(datetime.fromtimestamp(time.mktime((time.strptime(tag,"%Y:%m:%d %H:%M:%S")))))
        listtags.extend(tags1)
    else:
        print "Key not found, looking in comments"
        tags1 = []
        for y in range(len(metadata)):
            key2 = 'File:Comment'
            if key2 in metadata[y]:
                tag2 = date_reg_expAM.findall(metadata[y]['File:Comment'])
                if tag2 == []:
                    tag2 = date_reg_exp.findall(metadata[y]['File:Comment'])
                    tag = ''.join(map(str, tag2))
                    tags1.append(datetime.fromtimestamp(time.mktime((time.strptime(tag,"%Y-%m-%d %H:%M:%S")))))
                else:
                    tag = ''.join(map(str, tag2))
                    tags1.append(datetime.fromtimestamp(time.mktime((time.strptime(tag,"%Y-%m-%d %I:%M:%S %p")))))
            else:
                #print 'Problem with metadata, using FileModifyDate'
                #tag2 = date_reg_exp2.findall(metadata[y]['File:FileModifyDate'])
                #tag = ''.join(map(str, tag2))
                #tag = datetime.strptime(tag,"%Y:%m:%d %H:%M:%S").strftime('%Y-%m-%d %H:%M:%S')
                #tags1.append(datetime.fromtimestamp(time.mktime((time.strptime(tag,"%Y-%m-%d %H:%M:%S")))))
                sys.exit("Unable to read metadata...")
        listtags.extend(tags1)

print 'DONE Loading Metadata...'



'''
Sorting Images list based on their metadata
'''



imglist = [x for (y,x) in sorted(zip(listtags,imglist), key = lambda pair: pair[0])]
#imglist.sort()
#listtags.sort()
listtags = [x for (y,x) in sorted(zip(listtags,listtags), key = lambda pair: pair[0])]



'''
Computing time differences from the first image (get IMPG)
'''



impg = []
res  = []

for x in listtags:
    diff = int((x - listtags[0]).total_seconds())
    res.append(diff)

groups = [[res[0]]]

for y in res[1 : ]:
    if abs(y - groups[-1][-1]) <= maxgap:
        groups[-1].append(y)
    else:
        groups.append([y])

impgtemp = []

for group in groups:
    impgtemp.append(len(group))

impg.extend(impgtemp)

print 'Done Creating image sequences'



'''
Computing optimal maxgap
'''



tempimpg2 = impg

for y in range(3):
    tempimpg = []
    for x in range(len(tempimpg2)):
        if tempimpg2[x] <= 30:
            tempimpg.append(tempimpg2[x])
        elif 30 < tempimpg2[x] < 100:
            tempimpg.append(tempimpg2[x]-30)
            tempimpg.append(tempimpg2[x]-(tempimpg2[x]-30))
        elif 100 <= tempimpg2[x] <= 200:
            tempimpg.append(tempimpg2[x]-75)
            tempimpg.append(tempimpg2[x]-(tempimpg2[x]-75))
        else:
            tempimpg.append(tempimpg2[x]-100)
            tempimpg.append(tempimpg2[x]-(tempimpg2[x]-100))
    tempimpg2 = tempimpg

impg = tempimpg



'''
Background estimation and foreground segmentation
'''



for sequence in range(len(impg)):
    print "Analysing sequence ",sequence + 1
    for image in range(impg[sequence]):
        currentFrame = cv2.imread(imglist[image + int(sum(impg[0:sequence]))])
        imggray1 = cv2.cvtColor(currentFrame.copy(),cv2.COLOR_BGR2GRAY)
        imggray2 = imggray1[120:-10 , 1:-10]
        avgB = cv2.mean(imggray2)
        outf = open(os.path.join(paramsdir, 'params.txt'),'w')
        if avgB[0] < 100.0:
            print 'Low light', avgB
            outf.write(str(0.001))
        else:
            print 'High light', avgB
            outf.write(str(0.001))
        resizimg1 = cv2.resize(currentFrame,(0, 0), fx = 0.3, fy = 0.3)
        cv2.imwrite(os.path.join(temp1, os.path.basename(imglist[image + int(sum(impg[0:sequence]))])[0:-4] + '.jpg'), resizimg1)
        print 'Saving resized images', os.path.join(temp1, os.path.basename(imglist[image + int(sum(impg[0:sequence]))])[0:-4] + '.jpg')
    # Calling cpp executable to estimate background
    os.system(cppcom)
    # Calling cpp executable to subtract background
    os.system(cppfg)
    # Cleaning tempdir
    for the_file in os.listdir(temp1):
       file_path = os.path.join(temp1, the_file)
       try:
           if os.path.isfile(file_path):
               os.unlink(file_path)
       except Exception, e:
           print e



'''
Removing temporary folders
'''



shutil.rmtree(temp1)
shutil.rmtree('./output')



'''
################################################################################
################################################################################
################################################################################
'''



'''
FUNCTION DEFINITION - Loading original frame and masks and find external contour
'''



def loadF():
    currentFrame = cv2.imread(imglist[i])
    resizimg1 = cv2.resize(currentFrame, (0, 0), fx = 0.3, fy = 0.3)
    workFrame  = resizimg1[100:-20, 1:-10]
    workFramecp = workFrame.copy()
    workFrameGray = cv2.cvtColor(workFrame, cv2.COLOR_BGR2GRAY)
    currentMask1 = cv2.imread(maskslist[i])
    currentMask2 = currentMask1[100:-20, 1:-10]
    # Slight opening to clean noise
    opened = cv2.morphologyEx(currentMask2, cv2.MORPH_OPEN, kernelO1)
    # Find and draw external contours
    currentMaskOp1 = cv2.cvtColor(opened, cv2.COLOR_BGR2GRAY)
    # Perform small closing, to see
    currentMaskOp = cv2.morphologyEx(currentMaskOp1, cv2.MORPH_CLOSE, kernel)
    currentMask = cv2.convertScaleAbs(currentMaskOp)
    contoursE,hye = cv2.findContours(currentMask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)
    return currentMask,contoursE,currentMask2,workFrame,workFramecp



'''
FUNCTION DEFINITION - Find significant objects (in size)
'''



def detection():
    nobj = []
    for area in areas:
        if area < minsize:
            nobj.append(0)
        else:
            nobj.append(1)
    if sum(nobj) != 0:
        detect = 1
    else:
        detect = 0
    return detect



'''
################################################################################
################################################################################
################################################################################
'''



'''
Creation of a the final folder
'''



if not os.path.exists(outputDir + "Results/"):
    os.makedirs(outputDir + "Results/")



print 'First image', imglist[0]
print 'Loading masks and sorting masks'




'''
Getting list of masks
'''



resmasks  = './MasksResults'
todel = glob.glob(resmasks + '/EstBG*')
for c in todel:
	os.remove(c)

maskslist = glob.glob(resmasks + '/*.png')
maskslist.sort()
#maskslist = maskslist[4:]



'''
'''



print "First mask", maskslist[0]
print "Last mask", maskslist[-1]


WHITE = (255, 255, 255)



# Configuration of opening and erosion values

kernelO1 = np.ones((1, 1), np.uint8)
kernel   = np.ones((10, 10), np.uint8)



#for i in range(maskslist.index(maskslist[starti]), maskslist.index(maskslist[stopi])):
for i in range(len(maskslist)):
    print i
    print "Analysing", maskslist[i]
    # Loading current Frame
    currentMask, contoursE, currentMask2, workFrame, workFramecp = loadF()
    di = 1
    Frame = workFramecp
    Mask = currentMask
    # Computing total areas
    areas = [cv2.contourArea(c) for c in contoursE]
    # Object detected
    if sum(areas) < 1:
        res = 0
    else:
        res = detection()
    # Export in CSV
    resultrow = [maskslist[i], res]
    myfile = open(outputDir + 'Results/' + ouname + '.csv', 'a')
    wr = csv.writer(myfile)
    wr.writerow(resultrow)
    myfile.close()



'''
Remove Masks folder
'''

if rmmasks == 1:
    shutil.rmtree(resmasks)



time2 = time.time()
print('\nElapsed time: ' + str(round(time2 - time1)) + ' seconds')




'''
################################################################################
################################################################################
################################################################################
'''
