# -*- coding: utf-8 -*-

'''
The ``foxmask.py`` module will evaluate the background based on images
located, by default, in the ``Images`` directory of the
FoxMask repository, and perform a foreground segmentation
to identify moving objects. The main routine of the code will
iterate through every folders present in the ``Images`` directory, and
analyze found images. The ``Images`` directory is set by a variable define
in ``parameters.py``: **imagesDir**.

Example:
    To run this module, simply invoke it from the command
    line.

    $ python2 foxmask.py

The ultimate output of this module is a ``Results``
directory, under which resulting masks are copied, as
well as images containing moving objects (as the result
of the analysis). Tables are also written, one table for
each directory analyzed. Each table contains the name
of the image, the result (0 or 1) of detection and parameters
used during the analysis.
'''

import csv
import sys
#sys.path.append("/usr/local/lib/python2.7/site-packages")
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
import parameters
from fnmatch import filter, fnmatch
from parameters import *

time1 = time.time()


def getfolders(foldersdir):
    """Get the list of all folders in **foldersdir**
    """
    if not os.path.exists(foldersdir):
        print(foldersdir,
              "directory does not exist !")
        sys.exit()

    folderslist = os.walk(foldersdir).next()[1]
    folderslist = [foldersdir + '/' + s for s in folderslist]
    if "images/MasksResults" in folderslist: folderslist.remove("images/MasksResults")
    print folderslist
    return folderslist


def makeresultsfolder(resultsdir):
    """Make all needed folders to store the final results.
    """
    directories = ['FoxMaskResults',
                   'FoxMaskResults/tables',
                   'FoxMaskResults/images',
                   'FoxMaskResults/masks']
    for newdir in directories:
        if not os.path.exists(resultsdir + '/' + newdir):
            os.mkdir(os.path.join(resultsdir, newdir))


class Getimagesinfos:

    """Build data structure of images to be analyzed.

    Images to be analyzed have the following attributes:

    Attributes:
        folder: A string representing the folder containing images
        to be analyzed.
        imglist: A list of images to be analyzed.
        timeofcreation: A list containing images time of creation.


    """

    def __init__(self, folder):
        """Initialize attributes used across functions.
        """
        self.folder = folder
        self.imglist = None
        self.timeofcreation = None

    def getimageslist(self):
        """
        Generate a list of all images under `folder`

        .. note::
            This function should be rewritten using ignore
            case, and should be able to gather other images
            format.

        Returns:
           list. The list of images under the folder
           being analyzed.

        """
        imglist = []
        imglist1 = glob.glob(self.folder + '/*.JPG')
        imglist2 = glob.glob(self.folder + '/*.jpg')
        imglist = imglist1 + imglist2
        print 'Done Listing Images...'
        self.imglist = imglist
        return imglist

    def getimagesmeta(self):
        """
        Get metadata ``DateTimeOriginal`` from every
        image in **imglist**

        Format returned:

        >>> datetime.datetime(2014, 8, 6, 16, 5, 55)

        Returns:
           list. List of datetime objects representing the exact
           time of creation of each image.

        """
        timeofcreation = []

        with exiftool.ExifTool() as et:
            metadata = et.get_metadata_batch(self.imglist)
            key = 'EXIF:DateTimeOriginal'
            if key in metadata[0]:
                tags1 = []
                for y in range(len(metadata)):
                    tag = metadata[y]['EXIF:DateTimeOriginal']
                    tags1.append(datetime.fromtimestamp(time.mktime(
                        (time.strptime(tag,"%Y:%m:%d %H:%M:%S")))))
                timeofcreation.extend(tags1)
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
                timeofcreation.extend(tags1)

        print 'DONE Loading Metadata...'
        self.timeofcreation = timeofcreation
        return timeofcreation

    def sortimages(self):
        """Sort images based on their time of creation.

        This method makes sure images are appropriately sorted,
        not relying on images file name.
        """
        sortedimglist = [x for (y,x) in sorted(zip(self.timeofcreation,
                                                   self.imglist))]
        sortedtimeofcreation = [x for (y,x) in sorted(zip(self.timeofcreation,
                                                    self.timeofcreation))]
        self.sortedimglist = sortedimglist
        self.sortedtimeofcreation = sortedtimeofcreation
        return sortedimglist

    def getimpg(self):
        '''
        This will group images based on the differences in time between
        each shot. It is crucial that the listtags are well sorted by
        time.

        Attributes:
            parameters.maxgap: An int representing the maximum gap
            in seconds for two consecutive images to be considered as
            being part of the same group.
        '''
        impg = []
        res  = []
        for x in self.sortedtimeofcreation:
            diff = int((x - self.sortedtimeofcreation[0]).total_seconds())
            res.append(diff)
        groups = [[res[0]]]
        for y in res[1:]:
            if abs(y - groups[-1][-1]) <= parameters.maxgap:
                groups[-1].append(y)
            else:
                groups.append([y])
        impgtemp = []
        for group in groups:
            impgtemp.append(len(group))
        impg.extend(impgtemp)
        print 'Done Creating image sequences'
        return impg


class Imagesanalysis(Getimagesinfos):
    """Performs the image analysis.

    This class constitue the core code of
    the image analysis. It uses external
    cpp libraries coded by... citation...

    """

    def bgfgestimation(self, sortedimglist, impg, foldersdir):
        """Estimate background model and perform
        foreground segmentation.

        .. note::
            For performances reasons, the images have been reduced
            by a factor of 1/3 (0.3). (Link to code).

         """
        tempdir = foldersdir + '/temp1'

        for sequence in range(len(impg)):
            print "Analyzing sequence ", sequence + 1
            print impg
            print "range", range(impg[sequence])
            for image in range(impg[sequence]):

                if not os.path.exists(tempdir):
                    os.makedirs(tempdir)
                currentFrame = cv2.imread(sortedimglist[image + int(
                    sum(impg[0:sequence]))])
                imggray1 = cv2.cvtColor(currentFrame.copy(), cv2.COLOR_BGR2GRAY)
                imggray2 = imggray1[120:-10, 1:-10]
                avgB = cv2.mean(imggray2)
                outf = open(os.path.join(parameters.paramsdir, 'params.txt'), 'w')
                if avgB[0] < 100.0:
                    print 'Low light', avgB
                    outf.write(str(0.001))
                else:
                    print 'High light', avgB
                    outf.write(str(0.001))
                resizimg1 = cv2.resize(currentFrame, (0, 0), fx=0.3, fy=0.3)
                print "images", image
                formatedname = os.path.join(tempdir, os.path.basename(
                    sortedimglist[image + int(
                        sum(impg[0:sequence]))]))
                formatedname = os.path.splitext(formatedname)[0]+'.jpg'
                print formatedname
                cv2.imwrite(formatedname, resizimg1)
                print "Saving resized image as", formatedname
            cppcom1 = [parameters.cppex, tempdir + '/', 'EstBG']
            cppcom = ' '.join(cppcom1)
            cppcom2 = [parameters.cppex2, tempdir + '/', foldersdir + '/MasksResults']
            cppfg     = ' '.join(cppcom2)
            os.system(cppcom)
            os.system(cppfg)
            shutil.rmtree(tempdir)

    def masks_analysis(self, sortedimglist):
        """Masks analysis to detect moving objects large
        enough to be considered as animals.
        """
        resultslist = []
        for i in range(len(self.maskslist)):
            print "Analysing", self.maskslist[i]
            currentMask1 = cv2.imread(self.maskslist[i])
            currentMask2 = currentMask1[100:-20, 1:-10]
            opened = cv2.morphologyEx(currentMask2,
                                      cv2.MORPH_OPEN, parameters.kernelO1)
            currentMaskOp1 = cv2.cvtColor(opened, cv2.COLOR_BGR2GRAY)
            currentMaskOp = cv2.morphologyEx(currentMaskOp1,
                                             cv2.MORPH_CLOSE, parameters.kernel)
            currentMask = cv2.convertScaleAbs(currentMaskOp)
            im2, contoursE, hye = cv2.findContours(currentMask.copy(),
                                              cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)
            areas = [cv2.contourArea(c) for c in contoursE]
            for k in range(len(minsize)):
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
                resultrow = [self.maskslist[i], imgresult, minsize[k]]
                print resultrow
            resultslist.append(resultrow)
        self.resultslist = resultslist
        return resultslist

    def getmaskslist(self, foldersdir):
        """
        Create a sorted list of generated masks.
        """
        resmasks = foldersdir + '/MasksResults'
        todelete = glob.glob(resmasks + '/EstBG*')
        for c in todelete:
            os.remove(c)
        maskslist = glob.glob(resmasks + '/*.png')
        maskslist.sort()
        self.maskslist = maskslist
        return maskslist

    def writeresults(self, item, resultsdir):
        """Write results of the mask analysis to file
        """
        tablename = os.path.basename(item) + '.csv'
        tablepath = resultsdir + '/FoxMaskResults/tables/' + tablename
        with open(tablepath, 'w') as f:
            w = csv.writer(f)
            w.writerows(self.resultslist)


def main():
    pass


if __name__ == "__main__":
    main()
