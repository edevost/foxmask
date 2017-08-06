#!/bin/sh

####
# Install script for Ubuntu 16.04
# Intended to be run as user, in /home/user
cd ~/
sudo apt-get update
sudo apt-get -y install cmake libblas-dev liblapack-dev libsuperlu-dev libarpack2 libarpack2-dev pkg-config python-pip unzip mingetty

sudo apt-get -y install build-essential libgtk2.0-dev libjpeg-dev libtiff5-dev libjasper-dev libopenexr-dev python-dev python-numpy python-tk libtbb-dev libeigen3-dev yasm libfaac-dev libopencore-amrnb-dev libopencore-amrwb-dev libtheora-dev libvorbis-dev libxvidcore-dev libx264-dev libqt4-dev libqt4-opengl-dev sphinx-common libv4l-dev libdc1394-22-dev libavcodec-dev libavformat-dev libswscale-dev default-jdk ant libvtk5-qt4-dev

sudo apt-get -y install libopencv-dev checkinstall yasm libavcodec-dev libavformat-dev libswspcale-dev libdc1394-22-dev libxine2 libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev libqt4-dev libmp3lame-dev libopencore-amrnb-dev libopencore-amrwb-dev libtheora-dev libvorbis-dev libxvidcore-dev x264 v4l-utils

sudo apt-get -y install libimage-exiftool-perl

# If we are on vagrant, install xfce desktop
if [ -d "/home/vagrant" ];then
sudo apt-get -y install xfce4
sudo sed -i 's/\b\/agetty\b/& -a vagrant/' /etc/systemd/system/getty.target.wants/getty@tty1.service
cat << 'EOF' > /home/vagrant/.bash_profile
#starx on login
if [[ -z "$DISPLAY" ]] && [[ $(tty) = /dev/tty1 ]]; then
 . startx
 logout
fi
EOF
    
fi

# Armadillo
sudo apt-get -y install libarmadillo6 libarmadillo-dev

# Open CV
sudo apt-get -y install python-opencv libopencv-dev

# FoxMask
cd ~/foxmask/cpplibs/background_estimation_code/code/

g++ -L/usr/lib -L/usr/local/lib -I/usr/include -I/usr/include/opencv main.cpp SequentialBge.cpp SequentialBgeParams.cpp -O3 -larmadillo -lopencv_core -lopencv_highgui -fopenmp -o "EstimateBackground"

sudo cp EstimateBackground /usr/local/bin

cd ~/foxmask/cpplibs/foreground_detection_code/code/

g++ -o ForegroundSegmentation main.cpp input_preprocessor.cpp -O2 -fopenmp -I/usr/include/opencv -L/usr/lib64  -L/usr/lib -L/usr/local/lib -larmadillo -lopencv_core -lopencv_highgui -lopencv_imgproc

sudo cp ForegroundSegmentation /usr/local/bin


cd ~/foxmask
sudo pip install -r requirements.txt
sudo pip install .
sudo cp vagrant/Fox1.jpg /usr/share/backgrounds/xfce/
cd ~/
# If on google cloud, use the cpplibs
# without gui.
if curl metadata.google.internal -i ; then

    git clone https://github.com/edevost/foxmask-cpplibs-headless.git
    cd foxmask-cpplibs-headless/image-segmentation-code/background_estimation_code/code/
    g++ -L/usr/lib -L/usr/local/lib -I/usr/include -I/usr/include/opencv main.cpp SequentialBge.cpp SequentialBgeParams.cpp -O3 -larmadillo -lopencv_core -lopencv_highgui -fopenmp -o "EstimateBackground"
    sudo cp EstimateBackground /usr/local/bin
    cd ~/foxmask-cpplibs-headless/image-segmentation-code/foreground_detection_code/code/
    g++ -o ForegroundSegmentation main.cpp input_preprocessor.cpp -O2 -fopenmp -I/usr/include/opencv -L/usr/lib64  -L/usr/lib -L/usr/local/lib -larmadillo -lopencv_core -lopencv_highgui -lopencv_imgproc

cp ForegroundSegmentation /usr/local/bin    
fi
