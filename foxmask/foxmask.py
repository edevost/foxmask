# -*- coding: utf-8 -*-

"""Evaluate if an image contains moving objects (animals).

The main routine of the code will iterate through every folders present 
in the **images** directory and analyze found images, looking
for moving objects.

The following module level attributes are passed
via the command line interface.

Attributes:
    srcdir (str): The directory containing folders (one or many) with images 
        to analyze.

    resultsdir (str): The location where to create the **FoxMaskResults**
        directory. 
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


def getfolders(srcdir):
    """Get the list of all folders in **srcdir**

    FoxMask needs a list of folders containing images to analyze.
    Each folder must strictly contain a set of images. No sub folders
    are allowed. Code will gracefully exit if the **srcdir** argument
    does not exist.

    Args:
        srcdir (str): Top level folder containing all folders to analyze.
    
    Returns:
        list: folderslist. Folders to analyze.
    """
    if not os.path.exists(srcdir):
        print(srcdir,
              "directory does not exist !")
        sys.exit()

    folderslist = os.walk(srcdir).next()[1]
    folderslist = [srcdir + '/' + s for s in folderslist]
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
           list: imglist. All images under the folder being analyzed.

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
           list: timeofcreation. Datetime objects representing the exact
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
            list: sortedimglist. Sorted time of creation of all images to analyze.

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
            maxgap (int): This represent the maximum gap,
                , in seconds for two consecutive images to be considered as
                being part of the same group.
        
        Returns:
            list: impg. Number of images in each group.
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
    """Analyze images to detect moving objects.

    Parent: 
        Getimagesinfos

    """

    def bgfgestimation(self, sortedimglist, impg, srcdir):
        """Estimate background model and perform foreground segmentation.
        
        The method will iterate over each item of the ``impg`` list, and
        performs the analysis on each groups. The `trcd` value, which
        is influencing significantly the outcome of the analysis, is pass
        to the ``ForegroundSegmentation`` code. To pass this value to
        the cpp code, it is written to a file `/tmp/params.txt` which is
        then read by the cpp code at runtime.

        Images to analyze are resized (for performances issues on non server
        grade hardware) and then saved to a temporary directory that is
        deleted after each run. The results of this method are black and white
        masks that are written to disks in ``srcdir/MasksResults``.

        .. Note::

            This code will only run successfully if the input images
            are named as the following: ``whatever-name_[4 digits].jpg``.
            It is a requirement of the ``ForegroundSegmentation`` code.
            We do not think that FoxMask should handle the naming of
            the images to be analyzed. This should be done beforehand, by
            a prepossessing task.
        
        * `trcd` : Threshold for cosine distance. This value is fed to
          the ForegroundSegmentation code. It is a static value, but it could
          be made more dynamic using, for example, the average light in the
          image, as shown in the code below:

        .. code-block:: python

            if avgB[0] < 100.0:
                print 'Low light', avgB
                tfcd.write(str(0.001))
            else:
                print 'High light', avgB
                tfcd.write(str(0.005))
        

        Args:
            sortedimglist (list): The images to be analyzed
            impg (list): Groups of images on which to run the analysis.
            scrdir (string): The top level directory of the analysis.
          """
        tempdir = srcdir + '/temp1'

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
                tfcd = open('/tmp/params.txt', 'w')
                if avgB[0] < 100.0:
                    print 'Low light', avgB
                    tfcd.write(str(0.001))
                else:
                    print 'High light', avgB
                    tfcd.write(str(0.001))
                resizimg1 = cv2.resize(currentFrame, (0, 0), fx=0.3, fy=0.3)
                print "images", image
                formatedname = os.path.join(tempdir, os.path.basename(
                    sortedimglist[image + int(
                        sum(impg[0:sequence]))]))
                formatedname = os.path.splitext(formatedname)[0]+'.jpg'
                print formatedname
                cv2.imwrite(formatedname, resizimg1)
                print "Saving resized image as", formatedname
            cppcom1 = ["EstimateBackground", tempdir + '/', 'EstBG']
            cppcom = ' '.join(cppcom1)
            cppcom2 = ["ForegroundSegmentation", tempdir + '/', srcdir + '/MasksResults']
            cppfg     = ' '.join(cppcom2)
            os.system(cppcom)
            os.system(cppfg)
            shutil.rmtree(tempdir)

    def masks_analysis(self):
        """Analyze masks to detect moving objects

        This method will analyze created masks, taking
        the masks list to analyze from 
        :func:`foxmask.Imagesanalysis.getmaskslist`. The
        area of all white objects in each masks are
        calculated. If the area is smaller than ``parameters.minsize``,
        the object is not considered as an animal.

        Returns:
            list: resultslist. Results of the masks analysis.

        """
        resultslist = []
        for i in range(len(self.maskslist)):
            print "Analyzing", self.maskslist[i]
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

    def getmaskslist(self, srcdir):
        """Create a sorted list of generated masks.

        This method will create a list of all the images
        present in ``srcdir/MasksResults``. 
        Theses images are the black and white masks created by the
        :func:`foxmask.Imagesanalysis.bgfgestimation` method.
        Before creating the list, masks with the prefix ``EstBG``
        are removed.

        Args:
            srcdir (str): Top level directory containing
                directories of images.

        Returns:
            list: maskslist. All masks to analyze.
        
        """
        resmasks = srcdir + '/MasksResults'
        todelete = glob.glob(resmasks + '/EstBG*')
        for c in todelete:
            os.remove(c)
        maskslist = glob.glob(resmasks + '/*.png')
        maskslist.sort()
        self.maskslist = maskslist
        return maskslist

    def writeresults(self, item, resultsdir):
        """Write results of the mask analysis to file

        Args:
            item (str): The name of the class analyzed, representing
                the name of the folder analyzed. This is used to
                name the table in which the results will be written.

            resultsdir (str): The top directory where to write
                the final results.
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
