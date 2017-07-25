#!/bin/sh

####
# Install script for Ubuntu 16.04
# Intended to be run as user, in /home/user
cd ~/
sudo apt-get -y install cmake
sudo apt-get -y install libblas-dev
sudo apt-get -y install liblapack-dev
sudo apt-get -y install libsuperlu-dev
sudo apt-get -y install libarpack2
sudo apt-get -y install libarpack2-dev
sudo apt-get -y install pkg-config
sudo apt-get -y install python-pip

sudo apt-get -y install build-essential libgtk2.0-dev libjpeg-dev libtiff5-dev libjasper-dev libopenexr-dev python-dev python-numpy python-tk libtbb-dev libeigen3-dev yasm libfaac-dev libopencore-amrnb-dev libopencore-amrwb-dev libtheora-dev libvorbis-dev libxvidcore-dev libx264-dev libqt4-dev libqt4-opengl-dev sphinx-common libv4l-dev libdc1394-22-dev libavcodec-dev libavformat-dev libswscale-dev default-jdk ant libvtk5-qt4-dev


sudo apt-get -y install libimage-exiftool-perl

# OpenCV2

wget http://sourceforge.net/projects/opencvlibrary/files/opencv-unix/2.4.9/opencv-2.4.9.zip
unzip opencv-2.4.9.zip
cd opencv-2.4.9
mkdir build
cd build
cmake -D WITH_TBB=ON -D BUILD_NEW_PYTHON_SUPPORT=ON -D WITH_V4L=ON -D INSTALL_C_EXAMPLES=ON -D INSTALL_PYTHON_EXAMPLES=ON -D BUILD_EXAMPLES=ON -D WITH_QT=ON -D WITH_OPENGL=ON -D WITH_VTK=ON -D CMAKE_INSTALL_PREFIX=/usr/ ..
make -j8
sudo make install -j8

# Armadillo
wget http://sourceforge.net/projects/arma/files/armadillo-7.950.1.tar.xz
tar -xf armadillo-7.950.1.tar.xz

cd armadillo-7.950.1/
cmake .
make
sudo make install
cd ..

# Pyexiftool
cd ../../
git clone git://github.com/smarnach/pyexiftool.git
cd pyexiftool/
sudo python2 setup.py install
cd ..


# FoxMask

sudo mkdir /vagrant
sudo chown $USER /vagrant
git clone https://github.com/edevost/foxmask.git /vagrant

cd /vagrant/cpplibs/background_estimation_code/code/

g++ -L/usr/lib -L/usr/local/lib -I/usr/include -I/usr/include/opencv main.cpp SequentialBge.cpp SequentialBgeParams.cpp -O3 -larmadillo -lopencv_core -lopencv_highgui -fopenmp -o "EstimateBackground"


cd /vagrant/cpplibs/foreground_detection_code/code/

g++ -o ForegroundSegmentation main.cpp input_preprocessor.cpp -O2 -fopenmp -I/usr/include/opencv -L/usr/lib64  -L/usr/lib -L/usr/local/lib -larmadillo -lopencv_core -lopencv_highgui -lopencv_imgproc

cd /vagrant/
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
cat << EOF > /home/$USER/.vnc/xstartup
#!/bin/sh
xrdb $HOME/.Xresources
startxfce4 &
EOF
else
    echo "Not on GCE"

fi

    
     

