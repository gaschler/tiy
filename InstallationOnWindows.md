Tested on Windows 7 64-bit.

# Dependencies #
  * **Visual Studio 2010 / Visual C++ 10.0** from http://www.microsoft.com/visualstudio/eng/downloads#d-2010-express
> > For 64-bit use, download and install the Windows SDK (e.g. 7.1. for Windows 7 http://msdn.microsoft.com/en-us/windowsserver/bb980924.aspx)


> (for other compilers some of the following steps could be different)
  * **OpenCV** (>= 2.3):
    1. Download the newest Version from http://sourceforge.net/projects/opencvlibrary/files/opencv-win/
    1. Run the _.exe_ file and extract
    1. Create a new folder e.g. _C:\OpenCV2.4\_ and move the content of the extracted folder _opencv_ into there
    1. Run _Start Menu > All Programs > Accessories > Command prompt_ as admin (right click on it and _run as administrator_) and type _command:SETX /M OPENCV\_DIR C:\OpenCV2.4\build_ (or replace the directory as under point 3.)
    1. Add _;%OPENCV\_DIR%\x64\vc10\bin;_ for 64-bit or _;%OPENCV\_DIR%\x86\vc10\bin;_ for 32-bit to the system path (in Windows 7 by right click on _My Computer_ on your desktop, select _Properties_, click on _Extended system properties_, go to the _Advanced_ tab, click on _Environment Variables_ and double click on _Path_ in the _System variables_ area)
    1. Copy the files _tbb.dll_ and _tbb\_debug.dll_ from _C:\OpenCV2.4\build\common\tbb\intel64\vc10\_ to _C:\OpenCV2.4\build\x64\vc10\bin_ for 64-bit and/or from _C:\OpenCV2.4\build\common\tbb\ia32\vc10\_ to _C:\OpenCV2.4\build\x86\vc10\bin_ for 32-bit
  * (Build) **Boost** (>= 1.46):
    1. Download the newest version from http://sourceforge.net/projects/boost/files/boost/
    1. Unzip the folder (to e.g. _C:\boost`_`1`_`53`_`0_, but NOT into _C:\boost\boost`_`1`_`53`_`0_)
    1. For 32-bit, start a MSVC command prompt (_Start_->_ALL Programms_->_Microsoft Visual Studio 2010 Express_), for 64-Bit (or also 32-Bit), start a SDK command prompt instead (_Start_->_ALL Programms_->_Microsoft Windows SDK v7.1_)
    1. Navigate to the directory where Boost was unzipped to:
```
cd c:\boost_1_53_0
```
    1. Build the boost builder (in x86):
```
setenv /x86 /Release  // only necessary in the SDK command prompt
```
```
bootstrap
```
    1. Build boost for 32-Bit (header files: _C:\boost\include\boost-(version)_, 32(64)-bit libraries: _C:\boost\lib\boost-(version)_)
```
b2 --toolset=msvc-10.0 --build-type=complete install --libdir=C:\boost\lib\ --includedir=C:\boost\include\ --with-thread --with-chrono --with-regex --with-date_time --with-filesystem --with-system
```
> > or for 64-Bit
```
setenv /x64 /Release
```
```
b2 --toolset=msvc-10.0 --build-type=complete install --libdir=C:\boost\lib\ --includedir=C:\boost\include\ --with-thread --with-chrono --with-regex --with-date_time --with-filesystem --with-system architecture=x86 address-model=64
```
  * **Interception**:
    1. Download the installer from https://github.com/downloads/oblitum/Interception/install-interception.exe
    1. Run the command prompt as admin (see point 4. under OpenCV), navigate to the directory, where the downloaded _install-interception.exe_ is located and type _install-interception.exe /install_
    1. Reboot your system before the first use of the TIY library
  * **CMake** (optional - only when building TIY yourself):
> > Downloadable from http://www.cmake.org/cmake/resources/software.html

# TIY library (and examples) #

Two options are available for installation:
  * Pre-build installers: one single installation file, no building necessary
  * Build from source (based on _CMake_): further customization possible, also if no suitable installer available

## A. Pre-build installers ##

The _tiy`_`client_ example can be downloaded and installed without the TIY library and on all PCs in the same network.

For TIY, download and install the newest _tiy-X.X.-win_ _.exe_ version from the [Downloads](http://code.google.com/p/tiy/downloads/list) area for 64-bit (_amd64_) or 32-bit (_i386_) systems. The TIY library with the _tiy`_`server_ and _tiy`_`client_ example will be installed.

## B. Build from source ##

  1. Download and unzip the newest _tiy-X.X..zip_ (or the _tiy`_`client_ example) file from the [Downloads](http://code.google.com/p/tiy/downloads/list) section
  1. Start _CMake_ (_cmake-gui_) and select the _src_ directory as source and the _build_ directory as build
  1. Click _Configure_ and choose _Visual Studio 10_ (or Win64 version for 64-bit) as generator
  1. Change the options as you want to
    * _BUILD`_`client_: the client example will also be build
    * _BUILD`_`server_: the server example will also be build
  1. Click _Configure_ and then _Generate_
  1. Go into the _build_ directory and open the Visual Studio project by opening _tiy.sln_
  1. Change the build configuration from _Debug_ to _Release_ and build the project _PACKAGE_ (right click on it and select _Build_)
  1. Now you have built an installer by yourself and can install it as under point A.