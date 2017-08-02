import re
import numpy as np

'''
This file contains all parameters the user needs to set to run the Python program foxmask.py.
'''



# Location of the folder containing photos to analyze
# Can be one folder or a list of folders (but no embedded folders)
# Do not forget the final '/'
# Do not forget to enclose strings with '[' and ']'
imagesDir = "./images/"
# imagesDir = ["/media/sf_vagrant/reconyx/photos-fox/"]
# imagesDir = ["/media/sf_vagrant/reconyx-2/Fox024-2008-Cam1-test/"]
# imagesDir = ["/media/sf_vagrant/reconyx-2/Fox024-2008-Cam1-test/",
#              "/media/sf_vagrant/reconyx-2/Fox024-2008-Cam2-test/"]
# imagesDir = [
# '/media/sf_vagrant/reconyx/F001-2015-RE07-1/',
# '/media/sf_vagrant/reconyx/F003-2014-DB25-1/',
# '/media/sf_vagrant/reconyx/F006-2014-DB35-1/',
# '/media/sf_vagrant/reconyx/F006-2015-BD45-1/',
# '/media/sf_vagrant/reconyx/F010-2014-DB46-1/',
# '/media/sf_vagrant/reconyx/F014-2014-DB21-1/',
# '/media/sf_vagrant/reconyx/F014-2015-DB38-1/',
# '/media/sf_vagrant/reconyx/F024-2014-DB24-1/',
# '/media/sf_vagrant/reconyx/F112-2015-RE48-1/',
# '/media/sf_vagrant/reconyx/F113-2014-RE03-1/',
# '/media/sf_vagrant/reconyx/F122-2014-RE52-1/',
# '/media/sf_vagrant/reconyx/F133-2014-RE03-2/',
# '/media/sf_vagrant/reconyx/F134-2014-RE15-1/',
# '/media/sf_vagrant/reconyx/F137-2013-RE06-1/',
# '/media/sf_vagrant/reconyx/F142-2014-RE22-1/',
# '/media/sf_vagrant/reconyx/F142-2014-RE22-2/',
# '/media/sf_vagrant/reconyx/F143-2015-RE18-1/',
# '/media/sf_vagrant/reconyx/F149-2014-RE37-1/',
# '/media/sf_vagrant/reconyx/F152-2016-RE06-1/',
# '/media/sf_vagrant/reconyx/F153-2014-RE48-1/',
# '/media/sf_vagrant/reconyx/F155-2014-REXX-1/',
# '/media/sf_vagrant/reconyx/F161-RE08-2016-1/',
# '/media/sf_vagrant/reconyx/F201-2014-BD20-1/',
# '/media/sf_vagrant/reconyx/F203-2013-RE41-1/',
# '/media/sf_vagrant/reconyx/F204-2014-BD15-1/',
# '/media/sf_vagrant/reconyx/F209-2016-RE05-1/',
# '/media/sf_vagrant/reconyx/F322-2015-DB26-1/',
# '/media/sf_vagrant/reconyx/F325-2014-DB16-1/',
# '/media/sf_vagrant/reconyx/F325-2015-DB27-1/',
# '/media/sf_vagrant/reconyx/F336-2013-DB28-1/'
# ]


# Location of a single folder to store results
# Do not forget the final '/'
# A subfolder Results/ will be created

outputDir = './'



# Name of the CSV file(s) generated
# One file will be created for each input folder (and for each value of minsize, see after)
# Do not forget to enclose strings with '[' and ']'

# ouname = ["Fox024-2008-Cam1-test"]
# ouname = ["Fox024-2008-Cam1-test",
#           "Fox024-2008-Cam2-test"]
# ouname = [
# 'F001-2015-RE07-1',
# 'F003-2014-DB25-1',
# 'F006-2014-DB35-1',
# 'F006-2015-BD45-1',
# 'F010-2014-DB46-1',
# 'F014-2014-DB21-1',
# 'F014-2015-DB38-1',
# 'F024-2014-DB24-1',
# 'F112-2015-RE48-1',
# 'F113-2014-RE03-1',
# 'F122-2014-RE52-1',
# 'F133-2014-RE03-2',
# 'F134-2014-RE15-1',
# 'F137-2013-RE06-1',
# 'F142-2014-RE22-1',
# 'F142-2014-RE22-2',
# 'F143-2015-RE18-1',
# 'F149-2014-RE37-1',
# 'F152-2016-RE06-1',
# 'F153-2014-RE48-1',
# 'F155-2014-REXX-1',
# 'F161-RE08-2016-1',
# 'F201-2014-BD20-1',
# 'F203-2013-RE41-1',
# 'F204-2014-BD15-1',
# 'F209-2016-RE05-1',
# 'F322-2015-DB26-1',
# 'F325-2014-DB16-1',
# 'F325-2015-DB27-1',
# 'F336-2013-DB28-1'
# ]
outname = ["test"]

# Maximum time gap (in seconds) between two consecutive images (to build sequence)

maxgap = 5


# Configuration of opening and erosion values

kernelO1 = np.ones(( 1,  1), np.uint8)
kernel   = np.ones((10, 10), np.uint8)

# Deleting masks (0: no OR 1: yes [recommended])

rmmasks = 0

# Copying photo with animal (0: no OR 1: yes)
# Images will be copied only if minsize takes one single value

cpphotos = 1

# Minimum size of detected objects
# Can be one single value or a list of values (to perform sensitivity analyses)
# Do not forget to enclose strings with '[' and ']'

minsize = [500]
#minsize = list(np.arange(5,5000,5))


'''
The following variables should not be edited
'''

date_reg_exp = re.compile('\d{4}[-/]\d{1,2}[-/]\d{1,2}\ \d{1,2}:\d{1,2}:\d{1,2}')
date_reg_expAM = re.compile('\d{4}[-/]\d{1,2}[-/]\d{1,2}\ \d{1,2}:\d{1,2}:\d{1,2}\ [AP][M]')
date_reg_exp2 = re.compile('\d{4}[:/]\d{1,2}[:/]\d{1,2}\ \d{1,2}:\d{1,2}:\d{1,2}')

# Location of the compiled cpp libraries.
cppex     = './cpplibs/background_estimation_code/code/EstimateBackground'
cppex2    = './cpplibs/foreground_detection_code/code/ForegroundSegmentation'
paramsdir = './cpplibs/foreground_detection_code/code/'
