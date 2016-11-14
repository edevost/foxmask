
import csv
import sys
ver = "python{}.{}/site-packages" .format(sys.version_info.major,sys.version_info.minor)
sys.path.append("/usr/local/lib/" + ver)
import os
import cv2
import numpy as np
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
from scipy.stats.stats import pearsonr, linregress
import pylab
import random
from numpy import genfromtxt

''' This script will evaluate the background based on images
located, by default, in the /vagrant/foxmask/Images directory.
All images are resized (for performance issues) and the background
is evaluated. The resulting images are saved in a temporary
directory located within the /vagrant/foxmask/Images directory.
'''

### Configuration #####################################
''' Needed configuration for the script to work.
By default the script will use images in the Images
directory. You can specify here the location of your
images.
'''
imagesDir = "./Images/"


### End of configuration
########################################################

# create temporary directory in imagesDir
if not os.path.exists(imagesDir + "temp1"):
    os.makedirs(imagesDir + "temp1")
temp1 = os.path.join(imagesDir,"temp1/")

# Specify the location of the compiled c++ libraries
######################################################################
# cpp executable.
cppex = './cpplibs/background_estimation_code/code/EstimateBackground'
cppex2 = './cpplibs/foreground_detection_code/code/ForegroundSegmentation'
# c++ executable
cppcom1 = [cppex,temp1,"EstBG"]
cppcom = " ".join(cppcom1)
# 
cppcom2 = [cppex2,temp1,"MasksResults"]
paramsdir = "./cpplibs/foreground_detection_code/code/"
cppfg = " ".join(cppcom2)
print cppfg
######################################################################
# in cpplibs/foreground_detection_code/code/main.cpp
# you need to specify output path and params.txt path
######################################################################
# End cpp libraries section

# Get images list -----------------
imglist = []
imglist = glob.glob(imagesDir + "/*.jpg") # fix this...
print "done"

# get metadata -----------------------------------------
# Compiling regular expression search in metadata
date_reg_exp = re.compile(
                    '\d{4}[-/]\d{1,2}[-/]\d{1,2}\ \d{1,2}:\d{1,2}:\d{1,2}')
date_reg_expAM = re.compile(
                    '\d{4}[-/]\d{1,2}[-/]\d{1,2}\ \d{1,2}:\d{1,2}:\d{1,2}\ [AP][M]')
date_reg_exp2 = re.compile(
                    '\d{4}[:/]\d{1,2}[:/]\d{1,2}\ \d{1,2}:\d{1,2}:\d{1,2}')
listtags = []

with exiftool.ExifTool() as et:
    metadata = et.get_metadata_batch(imglist)
    #print "meta",metadata
    #check if EXIF key exist
    key = 'EXIF:DateTimeOriginal'
    #print "meta",metadata
    if key in metadata[0]:
        print "key found"
        tags1 = []
        for y in range(len(metadata)):
            tag = metadata[y]['EXIF:DateTimeOriginal']
            #print "tag", tag
            tags1.append(datetime.fromtimestamp(time.mktime(
                (time.strptime(tag,"%Y:%m:%d %H:%M:%S")))))
            #listtags.append(tag)
            #print listtags
        listtags.extend(tags1)
       # listtags.extend(tags1)
    else:
        print "key not found, looking in comments"
        tags1 = []
        for y in range(len(metadata)):
            key2 = 'File:Comment'
            if key2 in metadata[y]:
                tag2 = date_reg_expAM.findall(metadata[y]['File:Comment'])
                # account for AM PM, no AM PM if tag2 is empty
                if tag2 == []:
                    #print "24h date format detected"
                    tag2 = date_reg_exp.findall(
                        metadata[y]['File:Comment'])
                    tag = ''.join(map(str, tag2))
                    tags1.append(datetime.fromtimestamp(time.mktime(
                    (time.strptime(tag,"%Y-%m-%d %H:%M:%S")))))
                else:
                    #print "AM/PM date format detected"
                    tag = ''.join(map(str, tag2))
                    #print "tag",tag
                #tag = metadata[y]['File:Comment'][0][0:35]
                # print "keys",metadata[y].keys()
                #print "metadata img 1",metadata[0]
                    tags1.append(datetime.fromtimestamp(time.mktime(
                        (time.strptime(tag,"%Y-%m-%d %I:%M:%S %p")))))
                #listtags.append(tag)
                #print listtags
            else:
                #print "Problem with metadata, using FileModifyDate"
                tag2 = date_reg_exp2.findall(
                    metadata[y]['File:FileModifyDate'])
                tag = ''.join(map(str, tag2))
                tag = datetime.strptime(
                    tag,"%Y:%m:%d %H:%M:%S").strftime('%Y-%m-%d %H:%M:%S')
                tags1.append(datetime.fromtimestamp(time.mktime(
                    (time.strptime(tag,"%Y-%m-%d %H:%M:%S")))))
        listtags.extend(tags1)
print "DONE Loading Metadata"

# sort imglist based on metadatas (listtags)
imglist = [x for (y,x) in sorted(
    zip(listtags,imglist), key=lambda pair: pair[0])]


imglist.sort()
print imglist[0:10]

# sort listtags
listtags.sort()

# get IMPG
# compute time diffs from first image

maxgap = 5 # param
impg = []
res = []
for x in listtags:
    diff = int((x - listtags[0]).total_seconds())
    res.append(diff)
groups = [[res[0]]]
for y in res[1:]:
    if abs(y - groups[-1][-1]) <= maxgap:
        groups[-1].append(y)
    else:
        groups.append([y])
    # get values impg and nseq
impgtemp = []
for group in groups:
    impgtemp.append(len(group))
impg.extend(impgtemp)
print "done"
# Select images we want
# Get high img count
# get index of wanted sequence
#print impg.index(285)
#idx = impg.index(285)

# function to compute best maxgap
"""
To doucument
"""

tempimpg2 = impg
for y in range(3):
    tempimpg = []
    for x in range(len(tempimpg2)): # range 3, adjust if needed
        #print impg[x]
        if tempimpg2[x] <= 30:
            tempimpg.append(tempimpg2[x])
        elif 30 < tempimpg2[x] < 100:
            #print impg[x]
            tempimpg.append(tempimpg2[x]-30)
            tempimpg.append(tempimpg2[x]-(tempimpg2[x]-30))
        elif 100 <= tempimpg2[x] <= 200:
            #print impg[x]
            tempimpg.append(tempimpg2[x]-75)
            tempimpg.append(tempimpg2[x]-(tempimpg2[x]-75))
        else:
            tempimpg.append(tempimpg2[x]-100)
            tempimpg.append(tempimpg2[x]-(tempimpg2[x]-100))
    tempimpg2 = tempimpg


impg = tempimpg    
#impg = [5]

# Background estimation and foreground segmentation
#for 3 in range(1):
#sequence = 6 # 289
#for sequence in range(impg[2]):
#for 1 in 1:
for sequence in range(len(impg)):
    print "Analysing sequence ",sequence+1 ### xx
    # resize images
    #for image in range(impg):
    for image in range(impg[sequence]):
        #print "image",image
        currentFrame = cv2.imread(imglist[image + int(sum(impg[0:sequence]))])
        #print "img",imglist[image + int(sum(impg[0:sequence]))]
        # convert to grayscale
        imggray1 = cv2.cvtColor(currentFrame.copy(),cv2.COLOR_BGR2GRAY)
        # crop gray img
        imggray2 = imggray1[120:-10,1:-10]
        #print "img avg brightness",cv2.mean(imggray2)
        avgB = cv2.mean(imggray2)
        outf = open(os.path.join(paramsdir, 'params.txt'),'w')
        if avgB[0] < 100.0:
            print "low light",avgB
            outf.write(str(0.001))
        else:
            print "high light",avgB
            outf.write(str(0.001)) # 0.0011
        #print "Resizing",image+1
        #r = 651.0 / currentFrame.shape[1]
        #dim = (651, int(currentFrame.shape[0] * r ))
        resizimg1 = cv2.resize(
            currentFrame,(0,0),fx=0.3,fy=0.3)# (0.3 works well)
        # crop top and bottom image
        #resizimg = resizimg1
        # save resized frame to temp dir
        cv2.imwrite(os.path.join(temp1,
            os.path.basename(imglist[image + int(sum(
                impg[0:sequence]))])[0:-4]+".jpg"), resizimg1)
        print "saving resized images",os.path.join(temp1,
            os.path.basename(imglist[image + int(sum(
                impg[0:sequence]))])[0:-4]+".jpg")

    # call c++ executable to estimate background
    os.system(cppcom)
    # call c++ executable to subtract background
    os.system(cppfg)    
    # clean tmpdir

    for the_file in os.listdir(temp1):
       file_path = os.path.join(temp1, the_file)
       try:
           if os.path.isfile(file_path):
               os.unlink(file_path)
       except Exception, e:
           print e

