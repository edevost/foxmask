#!/bin/sh

####
# Install script for Ubuntu 16.04
# Intended to be run as user, in /home/user
cd ~/
sudo apt-get update
sudo apt-get -y install cmake
sudo apt-get -y install libblas-dev
sudo apt-get -y install liblapack-dev
sudo apt-get -y install libsuperlu-dev
sudo apt-get -y install libarpack2
sudo apt-get -y install libarpack2-dev
sudo apt-get -y install pkg-config
sudo apt-get -y install python-pip
sudo apt-get -y install unzip
sudo apt-get -y install mingetty

sudo apt-get -y install build-essential libgtk2.0-dev libjpeg-dev libtiff5-dev libjasper-dev libopenexr-dev python-dev python-numpy python-tk libtbb-dev libeigen3-dev yasm libfaac-dev libopencore-amrnb-dev libopencore-amrwb-dev libtheora-dev libvorbis-dev libxvidcore-dev libx264-dev libqt4-dev libqt4-opengl-dev sphinx-common libv4l-dev libdc1394-22-dev libavcodec-dev libavformat-dev libswscale-dev default-jdk ant libvtk5-qt4-dev

sudo apt-get -y install libopencv-dev checkinstall yasm libavcodec-dev libavformat-dev libswspcale-dev libdc1394-22-dev libxine2 libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev libqt4-dev libmp3lame-dev libopencore-amrnb-dev libopencore-amrwb-dev libtheora-dev libvorbis-dev libxvidcore-dev x264 v4l-utils

sudo apt-get -y install libimage-exiftool-perl

# If we are on vagrant, install xfce desktop
if [ -d "/home/vagrant" ];then
sudo apt-get install xfce4
fi

# Armadillo
sudo apt-get instal libarmadillo6-dev

# FoxMask
cd ~/foxmask/cpplibs/background_estimation_code/code/

g++ -L/usr/lib -L/usr/local/lib -I/usr/include -I/usr/include/opencv main.cpp SequentialBge.cpp SequentialBgeParams.cpp -O3 -larmadillo -lopencv_core -lopencv_highgui -fopenmp -o "EstimateBackground"

cd ~/foxmask/cpplibs/foreground_detection_code/code/

g++ -o ForegroundSegmentation main.cpp input_preprocessor.cpp -O2 -fopenmp -I/usr/include/opencv -L/usr/lib64  -L/usr/lib -L/usr/local/lib -larmadillo -lopencv_core -lopencv_highgui -lopencv_imgproc

cd ~/foxmask
sudo python2 -m pip install -r requirements.txt

# If on google cloud, make the machine available
# with a connection via turbovnc.
if curl metadata.google.internal -i ; then
cd ~/
wget https://sourceforge.net/projects/virtualgl/files/2.5.2/virtualgl_2.5.2_amd64.deb
wget https://sourceforge.net/projects/turbovnc/files/2.1/turbovnc_2.1_amd64.deb
sudo apt-get -y install xfce4 xfce4-goodies
sudo dpkg -i turbovnc_2.1_amd64.deb
sudo dpkg -i virtualgl_2.5.2_amd64.deb

mkdir /home/$USER/.vnc
cat << EOF > /home/$USER/.vnc/xstartup.turbovnc
#!/bin/sh
xrdb $HOME/.Xresources
startxfce4 &
EOF
else
    echo "Not on GCE"

fi
