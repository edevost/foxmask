# -*- coding: utf-8 -*-

"""Evaluate if an image contains moving objects (animals).

The main routine of the code will iterate through every folders present 
in the **images** directory and analyze found images, looking
for moving objects. To run ``FoxMask``, simply invoke it from the command
line, from the root of the FoxMask repository.

.. code-block:: console
    
  $ foxmask --foldersdir images --resultsdir .


The following module level attributes are passed
via the command line interface.

Attributes:
    foldersdir (str): The directory containing folders (one or many) with images 
        to analyze. The default value is set to **images**.

    resultsdir (str): The location where to create the **FoxMaskResults**
        directory. The default value is set to the actual 
        directory where the ``foxmask`` command is launched.


The ultimate output of this module is a ``Results``
directory, under which resulting masks are copied, as
well as images containing moving objects (as the result
of the analysis). Tables are also written, one table for
each directory analyzed. Each table contains the name
of the image, the result (0 or 1) of detection and parameters
used during the analysis.

"""

import csv
import sys
import os
import cv2
import glob
import time
from datetime import datetime
import shutil
import exiftool
import parameters

time1 = time.time()


def getfolders(foldersdir):
    """Get the list of all folders in **foldersdir**

    FoxMask needs a list of folders containing images to analyze.
    Each folder must strictly contain a set of images. No subfolders
    are allowed. Code will gracefully exit if the **foldersdir** argument
    does not exist.

    Args:
        foldersdir (str): Top level folder containing all folders to analyze.
    
    Returns:
        list: Folders to analyze.
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
    
    Args:
        resultsdir (str): Top level folder for storing results.
    
    Returns:
        None
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

    """

    def __init__(self, folder):
        """Initialize attributes used across methods.

        Args:
           folder (str): Actual folder containing images to analyze.
           imglist (list): Images to analyze.
           timeofcreation (list): Images time of creation.

        """
        self.folder = folder
        self.imglist = None
        self.timeofcreation = None

    def getimageslist(self):
        """
        Generate a list of all images under `folder`

        Returns:
           list: All images under the folder being analyzed.

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
        Generate a list of all images time of creation.

        The time of creation is taken from the ``DateTimeOriginal``
        key present in every image metadata. If the method can not read
        the ``DateTimeOriginal`` key, it will look in the ``Comment`` key
        trying to find the time of creation. Method will gracefully exit
        if not time of creation can be found.

        Example of format returned:

        >>> datetime.datetime(2014, 8, 6, 16, 5, 55)

        Returns:
           list: Datetime objects representing the exact
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

        This method makes sure images are appropriately sorted.
        We do not want to rely on images file name to sort them.

        Args:
            timeofcreation (date): A date object representing the
                time of creation of each image.

        Returns:
            list: Sorted time of creation of all images to analyze.

        """
        sortedimglist = [x for (y,x) in sorted(zip(self.timeofcreation,
                                                   self.imglist))]
        sortedtimeofcreation = [x for (y,x) in sorted(zip(self.timeofcreation,
                                                    self.timeofcreation))]
        self.sortedimglist = sortedimglist
        self.sortedtimeofcreation = sortedtimeofcreation
        return sortedimglist

    def getimpg(self):
        """Group images based on their time of creation

        This method will group images based on the differences in time between
        each shot.

        Args:
            maxgap (int): An int representing the maximum gap
                in seconds for two consecutive images to be considered as
                being part of the same group.
        
        Returns:
            list: Number of images in each group.
        """
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
            for k in range(len(parameters.minsize)):
                if sum(areas) < 1:
                    imgresult = 0
                else:
                    nobj = []
                    for area in areas:
                        if area < parameters.minsize[k]:
                            nobj.append(0)
                        else:
                            nobj.append(1)
                    if sum(nobj) != 0:
                        imgresult = 1
                    else:
                        imgresult = 0
                resultrow = [self.maskslist[i], imgresult, parameters.minsize[k]]
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
