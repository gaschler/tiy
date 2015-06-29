# 1. Install the TIY library #

Go to [InstallationOnWindows](http://code.google.com/p/tiy/wiki/InstallationOnWindows) to see how to install the TIY library (and examples) on Windows, or [InstallationOnLinux](http://code.google.com/p/tiy/wiki/InstallationOnLinux) for Linux systems respectively.

# 2. Setup Cmake #

Copy the files _Findtiy.cmake_, _LibFindMacros.cmake_ and _Findaravis.cmake_ from the _src_ directory of the _tiy`_`server_ example ([Downloads](http://code.google.com/p/tiy/downloads/list)) into your project directory.

Granted that your project has only one file called _main.cpp_ including the TIY library like
```
#include <tiy.h>
```
, create a file called _CMakeLists.txt_ with the following content:
```

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(CMAKE_PROJECT_NAME "my_project")

PROJECT(${CMAKE_PROJECT_NAME})

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})


##############
## Packages ##
##############

## Tiy ##

FIND_PACKAGE(tiy REQUIRED)

## INTERCEPTION ##

IF (WIN32 AND NOT UNIX) 		
	ADD_DEFINITIONS("-DINTERCEPTION_STATIC")
ENDIF(WIN32 AND NOT UNIX)

## BOOST ##

IF(WIN32)
	set(Boost_USE_STATIC_LIBS   ON)
ENDIF(WIN32)

FIND_PACKAGE(Boost COMPONENTS system thread filesystem REQUIRED )
MARK_AS_ADVANCED(Boost_LIB_DIAGNOSTIC_DEFINITIONS)
MARK_AS_ADVANCED(Boost_DIR)

IF (NOT WIN32 AND Boost_FOUND) 
	INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
	ADD_DEFINITIONS( "-DHAS_BOOST" )
ENDIF(NOT WIN32 AND Boost_FOUND)


## OpenCV ##

FIND_PACKAGE(OpenCV REQUIRED)
MARK_AS_ADVANCED(OpenCV_DIR)
MARK_AS_ADVANCED(OPENCV_FOUND)
MARK_AS_ADVANCED(OpenCV_FOUND)

## aravis + Gtk2 ##

IF (WIN32)
	MARK_AS_ADVANCED(USE_ARAVIS)
ENDIF(WIN32)

IF(USE_ARAVIS)
	FIND_PACKAGE(aravis)	
	
	IF (aravis_FOUND)
		add_definitions(-DUSE_aravis)
		SET(USE_aravis TRUE)
		
		FIND_PACKAGE(GTK2 REQUIRED)
	ENDIF(aravis_FOUND)
ENDIF(USE_ARAVIS)


#############
## Options ##
#############

OPTION(USE_ARAVIS "Use aravis library" OFF)

SET(CMAKE_VERBOSE_MAKEFILE ON)

IF (NOT WIN32)
	SET(CMAKE_CXX_FLAGS "-fopenmp")
ENDIF(NOT WIN32)


######################
## Link directories ##
######################

LINK_DIRECTORIES ( 
	${Boost_LIBRARY_DIRS} 
)


#################
## Directories ##
#################

set( DIRECTORIES	
	${tiy_INCLUDE_DIR}
	${Boost_INCLUDE_DIRS}
)

IF (USE_aravis) 
	list (APPEND DIRECTORIES ${aravis_INCLUDE_DIR} )
	list (APPEND DIRECTORIES ${GTK2_GOBJECT_INCLUDE_DIR} )
	list (APPEND DIRECTORIES ${GTK2_GLIBCONFIG_INCLUDE_DIR} )
ENDIF(USE_aravis)

INCLUDE_DIRECTORIES(
	BEFORE
	${DIRECTORIES}
)
  
  
################
## Executable ##
################ 

ADD_EXECUTABLE(
	main
	main.cpp	
)


###############
## Libraries ##
###############

SET( LIBRARIES	
	${tiy_LIBRARY}
	${OpenCV_LIBS}
	${Boost_LIBRARIES}	
)

IF (USE_aravis) 
	list (APPEND LIBRARIES ${aravis_LIBRARIES} )
	list (APPEND LIBRARIES "gthread-2.0")
ENDIF(USE_aravis) 

# Project #

TARGET_LINK_LIBRARIES(
	my_project
	${LIBRARIES}
)
```

# 3. Build and Start Your Project #

## Linux ##
Create a subdirectory in the project folder. Open a terminal, navigate to this folder and use _cmake_ to build the make file:
```
$ ccmake ..
```
Change the options as you want to
  * _USE`_`ARAVIS_ _ON_: only when using the Aravis library for Basler camera support
and set the option _CMAKE`_`BUILD`_`TYPE_ to _Release_.
Press the _C_ key a few times and then the _G_ key.

Now _make_ the project
```
$ make
```
and start it
```
$ ./main
```

## Windows ##
  1. Create a subdirectory in your project directory.
  1. Start _CMake_ (_cmake-gui_) and select your project directory as _source_ and the created subdirectory as _build_
  1. Click _Configure_ and choose _Visual Studio 10_ (or 64-bit version) as generator
  1. Click _Configure_ and then _Generate_
  1. Go into the subdirectory directory and open the Visual Studio project by opening the _.sln_ file
  1. Change the build configuration from _Debug_ to _Release_ and build the project (right click on it and select _Build_)
  1. Now you can start the _.exe_ in the _Release_ subdirectory