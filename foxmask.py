# -*- coding: utf-8 -*-

'''
This script will evaluate the background based on images
located, by default, in the ``Images`` directory of the
FoxMask repository, and perform a foreground segmentation
to identify moving objects.

Example:
    To run this module, simply invoke it from the command
    line.

    $ python2 foxmask.py

The resulting images are saved in a temporary
directory located within the Images directory.
'''

import csv
import sys
sys.path.append("/usr/local/lib/python2.7/site-packages")
import os
import cv2
from collections import defaultdict
from numpy import genfromtxt
import glob
from itertools import groupby
import time
from datetime import datetime, date
import shutil
import exiftool
import random
import xlrd
#import parameters
from fnmatch import filter, fnmatch
from parameters import *

time1 = time.time()



class Setup:
    '''
    This class will do all the preliminary
    setup required to run the code and clean
    temporary files.
    '''
    def getfolders(self):
        '''
        This function will take as input the
        ``imagesDir`` variable specified in
        ``parameters.py`` and return a list
        of all folders present in it. This list
        will then feed the software with all the folders
        containing images to be analyzed. Each folders are 
        considered as a single analyzed entity. This will be 
        reflected in the writing of the results, which will 
        written as one results per folders.

        Returns:
          list. A list of all folders in ``imageDir``.

        '''

        folderslist = os.walk(imagesDir).next()[1]
        print "Anlysing images in :", folderslist
        folderslist = [imagesDir + s for s in folderslist]
        return folderslist

    def maketempdir(self, folder):
        '''
        Creation of a temporary directory in which
        all analyzed images will be written. This directory
        will be created in all folders being analyzed.

        Returns:
           string. The location of the actual folder being
           analyzed concatenated with ``temp1``.
        '''

        if not os.path.exists(folder + '/temp1'):
            os.makedirs(folder + '/temp1')
        temp1 = os.path.join(folder, 'temp1/')
        return temp1

    def delmaskresults(self, folder):
        '''
        Remove MaskResults directory if
        the flag is set to 0 in parameters.
        '''
        if rmmasks == 0:
            if os.path.exists(folder + '/MasksResults'):
                shutil.rmtree(folder + '/MasksResults')
            else:
                pass
        else:
            pass

    def makeresultsfolder(self, folder):
        '''
        Make all needed folders to store the final results.
        '''
        resultsdirs = ['Results',
                       'Results/tables', 'Results/tables/',
                       'Results/images', 'Results/images',
                       'Results/masks', 'Results/masks']
        for newdir in resultsdirs:
            if not os.path.exists(outputDir + newdir):
                os.mkdir(os.path.join(outputDir, newdir))

class Getimagesinfos:
    '''
    This class will gather all images
    and informations about them, so they
    can afterwards be analysed.
    '''
    def getimageslist(self, folder):
        '''
        Get a list of all images under `folder`.

        .. note::
            This should be rewritten using ignore case
            or something else.

        '''
        imglist = []
        imglist1 = glob.glob(folder + '/*.JPG')
        imglist2 = glob.glob(folder + '/*.jpg')
        imglist = imglist1 + imglist2
        print 'Done Listing Images...'
        return imglist

    def getimagesmeta(self, imglist):
        '''
        Get metadata (creation time) of every
        image and store it into a list.

        Format returned:

        >>> datetime.datetime(2014, 8, 6, 16, 5, 55)
        '''
        listtags = []

        with exiftool.ExifTool() as et:
            metadata = et.get_metadata_batch(imglist)
            key = 'EXIF:DateTimeOriginal'
            if key in metadata[0]:
                tags1 = []
                for y in range(len(metadata)):
                    tag = metadata[y]['EXIF:DateTimeOriginal']
                    tags1.append(datetime.fromtimestamp(time.mktime(
                        (time.strptime(tag,"%Y:%m:%d %H:%M:%S")))))
                listtags.extend(tags1)
            else:
                tags1 = []
                for y in range(len(metadata)):
                    key2 = 'File:Comment'
                    if key2 in metadata[y]:
                        tag2 = date_reg_expAM.findall(metadata[y]['File:Comment'])
                        if tag2 == []:
                            tag2 = date_reg_exp.findall(metadata[y]['File:Comment'])
                            tag = ''.join(map(str, tag2))
                            tags1.append(datetime.fromtimestamp(time.mktime(
                                (time.strptime(tag,"%Y-%m-%d %H:%M:%S")))))
                        else:
                            tag = ''.join(map(str, tag2))
                            tags1.append(datetime.fromtimestamp(time.mktime(
                                (time.strptime(tag,"%Y-%m-%d %I:%M:%S %p")))))
                    else:
                        sys.exit("Unable to read metadata...")
                listtags.extend(tags1)

        print 'DONE Loading Metadata...'
        return listtags

    def sortimages(self, imglist, listtags):
        '''
        Sort images based on their metadata.
        This function can not be called
        before getimagesmeta.
        '''
        sortedimglist  = [x for (y,x) in sorted(zip(listtags, imglist))]
        sortedlisttags = [x for (y,x) in sorted(zip(listtags, listtags))]
        return sortedimglist, sortedlisttags

    def getimpg(self, sortedlisttags):
        '''
        This will group images based on the differences in time between
        each shot. It is crucial that the listtags are well sorted by
        time.
        '''
        impg = []
        res  = []
        for x in sortedlisttags:
            diff = int((x - sortedlisttags[0]).total_seconds())
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
        return impg

class Imagesanalysis:
    '''
    This class constitue the core code of
    the image analysis. It uses external
    cpp libraries coded by... citation...

    '''
    def bgfgestimation(self, impg, sortedimglist, folder, temp1):
        '''
        Estimate background model and perform
        foreground segmentation, which is define
        by subtracting the detected background to
        all images in `folder`.

        .. note::
            The function will write generated images masks
            in every `folders` analyzed. The masks will
            resided in `folders/MaksResults` directory,
            where folder is derived from the variable
            `imagesDir` defined in `parameters.py`. There
            is also an hard coded path residing in the
            foreground segmentation code, in
            `cpplibs/foreground_detection_code/code/main.cpp`. This
            is far for optimal, as it links the code to a location
            called `/vagrant/`, which will only work on a vagrant
            vm context.

        .. note::
            For performances reasons, the images have been reduced
            by a factor of 1/3 (0.3). (Link to code).

         '''
        for sequence in range(len(impg)):
            print "Analysing sequence ",sequence + 1
            print impg
            print "range",range(impg[sequence])
            for image in range(impg[sequence]):
                currentFrame = cv2.imread(sortedimglist[image + int(
                    sum(impg[0:sequence]))])
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
                cv2.imwrite(os.path.join(temp1, os.path.basename(
                    sortedimglist[image + int(
                        sum(impg[0:sequence]))])[0:-4] + '.jpg'), resizimg1)
                print 'Saving resized images', os.path.join(
                    temp1, os.path.basename(sortedimglist[image + int(
                        sum(impg[0:sequence]))])[0:-4] + '.jpg')
            # Build cpp commands
            cppcom1   = [cppex, temp1, 'EstBG']
            cppcom    = ' '.join(cppcom1)
            cppcom2   = [cppex2, temp1,
                         os.path.split(
                             os.path.split(folder)[0])[1] + "/" +
                         os.path.split(folder)[1] + '/MasksResults']
            cppfg     = ' '.join(cppcom2)
            # Calling cpp executable to estimate background
            os.system(cppcom)
            # Calling cpp executable to subtract background
            os.system(cppfg)
            # Cleaning temp dir
            for the_file in os.listdir(temp1):
                file_path = os.path.join(temp1, the_file)
                try:
                    if os.path.isfile(file_path):
                        os.unlink(file_path)
                except Exception, e:
                    print e

    def loadframes(self, sortedimglist, maskslist, i):
        '''
        This function will load frames, one by one, to return all
        needed images for analysis. It will perform small opening
        and closing operations on images masks to cleanup some
        noise.
        '''
        print "Analysing", maskslist[i]
        currentFrame = cv2.imread(sortedimglist[i])
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
        contoursE,hye = cv2.findContours(currentMask.copy(),
                                         cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)
        return currentMask,contoursE,currentMask2,workFrame,workFramecp


    def getmaskslist(self, folder):
        '''
        Load the resulted created masks as a list
        and sort it.
        '''
        resmasks  = folder + '/MasksResults'
        todel = glob.glob(resmasks + '/EstBG*')
        for c in todel:
            os.remove(c)

        maskslist = glob.glob(resmasks + '/*.png')
        maskslist.sort()
        print 'Loading and sorting masks'
        return maskslist

    def generateresults(self, i, maskslist, currentMask, contoursE,
                    currentMask2, workFrame, workFramecp):
        '''
        Load mask  and find contours and size of objects in masks.
        This will return a list of results for each analyzed image, 0
        for no detected object and 1 for a detected object.

        This function returns the results of one single
        image and stores it into a list of list.
        '''
        Frame = workFramecp
        Mask = currentMask
        # Computing total areas
        areas = [cv2.contourArea(c) for c in contoursE]
        for k in range(len(minsize)):
            # Object detected
            if sum(areas) < 1:
                imgresult = 0
            else:
                nobj = []
                for area in areas:
                    if area < minsize[k]:
                        nobj.append(0)
                    else:
                        nobj.append(1)
                if sum(nobj) != 0:
                    imgresult = 1
                else:
                    imgresult = 0
            resultrow = [maskslist[i], imgresult, minsize[k]]
        return resultrow


class Resultshandling:
    '''
    This class regroup functions that handles
    results of the image analysis.
    '''
    def writetable(self, folder, resultlist ):
        '''
        Write results of animal detection
        in a csv table, one table on each
        analyzed folder.
        '''
        outname = os.path.basename(folder)

        resultfile = outputDir + 'Results/tables/' + outname + '.csv'
        with open(resultfile, 'w') as f:
            w = csv.writer(f)
            w.writerows(resultlist)

def main():
    # Code execution
    # Instantiate classes
    runsetup = Setup()
    rungetimagesinfo = Getimagesinfos()
    runimagesanalysis = Imagesanalysis()
    runresultshandling = Resultshandling()
    # Get the folders list to be analysed
    folderslist = runsetup.getfolders()
    # Analyse folders one by one
    for folder in folderslist:
        '''
        Iterate through folders and execute the
        code on each of them
        '''
        imglist = rungetimagesinfo.getimageslist(folder)
        listtags = rungetimagesinfo.getimagesmeta(imglist)
        sortedimglist, sortedlisttags = rungetimagesinfo.sortimages(imglist, listtags)
        impg = rungetimagesinfo.getimpg(sortedlisttags)
        temp1 = runsetup.maketempdir(folder)
        runsetup.makeresultsfolder(folder)
        runimagesanalysis.bgfgestimation(impg, sortedimglist, folder, temp1)
        maskslist = runimagesanalysis.getmaskslist(folder)
        # Analysing images
        resultlist = []
        for i in range(len(maskslist)):
            currentMask, contoursE, currentMask2, workFrame, workFramecp = runimagesanalysis.loadframes(
                sortedimglist, maskslist, i )
            resultrow = runimagesanalysis.generateresults(i, maskslist, currentMask,
                                          contoursE,
                                          currentMask2,
                                          workFrame,
                                          workFramecp)
            resultlist.append(resultrow)
        # Write results for analysed folder
        runresultshandling.writetable(folder, resultlist)
        # Cleanup Maksresults
        runsetup.delmaskresults(folder)

if __name__ == "__main__":
    main()
