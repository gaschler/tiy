Tested on Ubuntu 11.10, 12.04 and 12.10 all in 64-bit.

# Dependencies #
  * **OpenCV** (>= 2.2):
```
 $ sudo apt-get install libopencv-dev
```
  * **Boost** (>= 1.46):
```
 $ sudo apt-get install libboost-dev libboost-filesystem-dev libboost-system-dev libboost-date-time-dev libboost-thread-dev
```
  * **CMake** and Compilers (optional - only when building TIY yourself)
```
 $ sudo apt-get install cmake build-essential cmake-curses-gui
```
  * **Aravis** (>= 1.15) (optional - only for Aravis supported cameras (e.g. Basler)):
    1. install required software
```
 $ sudo apt-get install libglib2.0-dev libgtk-3-dev intltool libgtk2.0-dev libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev
```
    1. download and extract the newest Aravis library from http://ftp.gnome.org/pub/gnome/sources/aravis/
    1. go into the extracted directory and _configure_ (see _./configure --help_ for more information):
```
 $ ./configure --enable-gst-plugin --enable-viewer --enable-gtk3
```
    1. _make_ and _make install_ the library


# TIY library (and examples) #

Two options are available for installation:
  * Pre-build package: one single installation file, no building necessary
  * Build from source (based on _CMake_): further customization possible, also if no suitable package available

## A. Pre-build package ##

The _tiy`_`client_ example can be downloaded and installed without the TIY library and on all PCs in the same network.

TIY _.deb_ packages for 64-bit (_amd64_) and 32-bit (_i386_) are available in the [Downloads](http://code.google.com/p/tiy/downloads/list) section. The TIY library with the _tiy`_`server_ and _tiy`_`client_ example will be installed.

For Aravis camera support, please build from source.

  1. Download the newest _tiy-X.X.-linux_ _.deb_ package from the [Downloads](http://code.google.com/p/tiy/downloads/list) area.
  1. Install the package by double clicking on it or by (e.g.):
```
 sudo dpkg -i tiy-0.1.-linux-amd64.deb 
```
> removal by:
```
 sudo dpkg -r tiy
```

## B. Build from source ##

  1. Download and _unzip_ the newest _tiy-X.X..zip_ file from the [Downloads](http://code.google.com/p/tiy/downloads/list) section
  1. Go into the _Release_ folder an use _cmake_ to build the make file
```
$ ccmake ../src
```
> Press the _C_ key, change the options as you want to
    * _BUILD`_`client_ _ON_: the client example will also be build
    * _BUILD`_`server_ _ON_: the server example will also be build
    * _USE`_`ARAVIS_ _ON_: build with the Aravis camera interface
    * (advanced option:) _BUILD`_`x64_: build in 64-Bit or not
> and set the option _CMAKE`_`BUILD`_`TYPE_ to _Release_.
> Press the _C_ key a few times and then the _G_ key.
  1. _make_ the project:
```
$ make
```
  1. Install the project by building a package (recommended):
```
 $ make package
```
> and install it as under point A.2.
> Alternatively, the project can directly be installed (more difficult to remove) by
```
 $ make install
```