satsd = []
for i in range(len(imglist)):
    currentFrame = cv2.imread(imglist[i]) #
    print "analysing",i
    resizimg1 = cv2.resize(
            currentFrame,(0,0),fx=0.3,fy=0.3)# (0.3 works well)
    workFrame  = resizimg1[200:-20,1:-10]
    workFramecp = workFrame.copy()
    workFrameHSV = cv2.cvtColor(workFrame, cv2.COLOR_BGR2HSV)
    workFrameGray = cv2.cvtColor(workFrame,cv2.COLOR_BGR2GRAY)
    grayforev = cv2.cvtColor(workFrame,cv2.COLOR_BGR2GRAY)
    meanHUE = cv2.mean(workFrameHSV)
    if meanHUE[1] > 100 and sdGreyI[0] > 50:
        satsd.append(1)
    else:
        satsd.append(0)
