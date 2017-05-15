# FoxMask

A Python program to automatically detect moving objects on images


### What is it?

FoxMask has been created to give user a tool to automatically analyze images taken by RECONYX cameras. Theses cameras are triggered by a motion detection system and FoxMask has been written to be able to isolate moving objects with very short image sequences (minimum of 5). It is written in Python and depends on C++ libraries.


### Hardware requirements

This Python program is intended to be used within a virtual environment including an Ubuntu 14.04 Vagrant box. To run this virtual environment, you need at least 13 GB of free disk space and 4 GB of RAM.

**Note:** virtualization needs to be enabled for your CPU.


### Installation

Three open-source softwares need to be installed on your computer:

- [VirtualBox v.5.1](https://www.virtualbox.org/), to manage virtual environments;
- [Vagrant](https://www.vagrantup.com/), to unpack the Ubuntu 14.04 Box (including Linux Ubuntu 14.04, Python 2.7 and its required modules);
- [git](https://git-scm.com/), to clone (download) the actual repository (including FoxMask program and C++ dependencies).

Once theses softwares are installed, you can proceed to install the FoxMask Box on your computer. Two steps are required: 1) clone this repository and 2) install the FoxMask Box.

Open the command-line interpreter of your OS (the shell on Unix-like systems or the cmd on Windows), and select the directory where you want to download the repository. For instance, if you want to select the **Documents/** folder:

- Windows

```bash
cd C:/Users/<username>/Documents
```

- Unix-like systems

```bash
cd /Users/<username>/Documents
```

Make sure you change `<username>` by your real user name.

Then you can clone the repository:

```bash
git clone https://github.com/edevost/foxmask.git
```

And download, unpack and install the FoxMask Box:

```bash
cd foxmask
vagrant up
```

At the end of the installation, the virtual box is automatically launched. If not, open VirtualBox and run the FoxMask Box (see `Screenshot #1`). You will see the Ubuntu 14.04 interface. In this virtual environment, your user name is **vagrant** and the associated password is **vagrant**.

![Screenshot \#1: Launch FoxMask Box](screenshot.png)

### Configuration

Before using FoxMask, a final configuration step is necessary. Make sure you have open and close FoxMask Box at least once.

- Shared folder

- Folder permissions


### Usage

This software is shipped with a sample images directory (**images/example/**) where you will find 50 images of Arctic fox.
You first need to generate the masks with generatemasks.py.
- python generatemasks.py

This will take as input the sample images located in the Images folder
and creates Masks in the MasksResults folder. Once the masks are
generated, you can run foxcount.py to count the foxes on the images.

- python foxcount.py


### Licensing

Please see the file called LICENSE.


### Contact

Feel free to contact me if you have any comments or questions at ericdevost@gmail.com


### Reference

Devost É, Casajus N, Lai S & Berteaux D (9999) FOXMASK: a new automated tool for animal screening on camera trap images. **Methods in Ecology and Evolution**, submitted.
