##############################################
## CMakeLists.txt - TIY - by Andreas Pflaum ##
##############################################

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(CMAKE_PROJECT_NAME "tiy")

PROJECT(${CMAKE_PROJECT_NAME})

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})


##############
## Packages ##
##############

MARK_AS_ADVANCED(CMAKE_INSTALL_PREFIX)

## INTERCEPTION ##

IF (WIN32 AND NOT UNIX) 
#	FIND_PACKAGE(interception REQUIRED)		
	ADD_DEFINITIONS("-DINTERCEPTION_STATIC")
ENDIF(WIN32 AND NOT UNIX)


## BOOST ##

#set(Boost_USE_MULTITHREADED ON)
#set(Boost_DEBUG TRUE)

IF(WIN32)
	set(Boost_USE_STATIC_LIBS   ON)
	#set(Boost_USE_STATIC   ON)
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

IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
	OPTION(BUILD_x64 "Build in amd64" ON)
ELSE(CMAKE_SIZEOF_VOID_P EQUAL 8)
	OPTION(BUILD_x64 "Build in amd64" OFF)
ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 8)  
MARK_AS_ADVANCED(BUILD_x64)

OPTION(BUILD_server "Build also the server example" ON)
OPTION(BUILD_client "Build also the client example" ON)

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
	${Boost_INCLUDE_DIR}
)

#IF (WIN32 AND NOT UNIX)
#	list (APPEND DIRECTORIES ${interception_INCLUDE_DIR} )
#ENDIF(WIN32 AND NOT UNIX)

IF (USE_aravis) 
	list (APPEND DIRECTORIES ${aravis_INCLUDE_DIR} )
	list (APPEND DIRECTORIES ${GTK2_GOBJECT_INCLUDE_DIR} )
	list (APPEND DIRECTORIES ${GTK2_GLIBCONFIG_INCLUDE_DIR} )
ENDIF(USE_aravis)

INCLUDE_DIRECTORIES(
	BEFORE
	${DIRECTORIES}
)


#############
## Sources ##
#############

set( SOURCES 
	multicastServer/MulticastServer.cpp
	multicastClient/MulticastClient.cpp
	markerTracking/MarkerTracking.cpp
	stereoCam/StereoCamera.cpp	
	stereoCam/OpenCVStereoCamera.cpp
	inputDevice/MouseDevice.cpp
	inputDevice/KeyboardDevice.cpp
    )    
    
set( WIN_SRCS
	inputDevice/win/interception/utils.c
	inputDevice/win/interception/interception.c
	inputDevice/win/WindowsMouse.cpp
	inputDevice/win/WindowsKeyboard.cpp
    )
    
set( UNIX_SRCS
	inputDevice/unix/LinuxMouse.cpp
	inputDevice/unix/LinuxKeyboard.cpp
    )
    
set( aravis_SRCS
	stereoCam/unix/BaslerGigEStereoCamera.cpp
    )  
    
IF (WIN32) 
	list (APPEND SOURCES ${WIN_SRCS})
ELSE(WIN32)
	list (APPEND SOURCES ${UNIX_SRCS})
ENDIF(WIN32)

IF (USE_aravis) 
	list (APPEND SOURCES ${aravis_SRCS} )
ENDIF(USE_aravis)  
   
   
#############
## Headers ##
#############

set( HEADERS 	
	multicastServer/MulticastServer.h
	multicastClient/MulticastClient.h
	markerTracking/MarkerTracking.h
	stereoCam/StereoCamera.h
	stereoCam/OpenCVStereoCamera.h
	inputDevice/MouseDevice.h
	inputDevice/KeyboardDevice.h
    )  
    
set( WIN_HDRS
	inputDevice/win/interception/utils.h
	inputDevice/win/interception/interception.h
	inputDevice/win/WindowsMouse.h
	inputDevice/win/WindowsKeyboard.h
    )
    
set( UNIX_HDRS
	inputDevice/unix/LinuxMouse.h	
	inputDevice/unix/LinuxKeyboard.h	
    )
    
set( aravis_HDRS
	stereoCam/unix/BaslerGigEStereoCamera.h
    )
    
IF (WIN32) 
	list (APPEND HEADERS ${WIN_HDRS})
ELSE(WIN32)
	list (APPEND HEADERS ${UNIX_HDRS})
ENDIF(WIN32)

IF (USE_aravis) 
	list (APPEND HEADERS ${aravis_HDRS} )
ENDIF(USE_aravis) 
  
  
################
## Executable ##
################ 

IF(BUILD_server)
	ADD_EXECUTABLE(
		server
		server.cpp
		${SOURCES}
		${HEADERS}
	)
ENDIF(BUILD_server)

IF(BUILD_client)
	ADD_EXECUTABLE(
		client
		client.cpp
		${SOURCES}
		${HEADERS}
	)
ENDIF(BUILD_client)


###############
## Libraries ##
###############

SET( LIBRARIES	
	${OpenCV_LIBS}
	${Boost_LIBRARIES}	
)

#IF (WIN32 AND NOT UNIX)
#	list (APPEND LIBRARIES ${interception_LIBRARY} )
#ENDIF(WIN32 AND NOT UNIX)

IF (USE_aravis) 
	list (APPEND LIBRARIES ${aravis_LIBRARIES} )
	list (APPEND LIBRARIES "gthread-2.0")
ENDIF(USE_aravis) 


# Shared Lib #

SET(PROJECT_SHARED_LIBRARY_NAME "${CMAKE_PROJECT_NAME}_shared")
ADD_LIBRARY(${PROJECT_SHARED_LIBRARY_NAME} SHARED ${SOURCES})
SET_TARGET_PROPERTIES(
     ${PROJECT_SHARED_LIBRARY_NAME}
   PROPERTIES
     SOVERSION 0
     VERSION 0.1
   )
   
TARGET_LINK_LIBRARIES(
	${PROJECT_SHARED_LIBRARY_NAME}
	${LIBRARIES}
)
   
   
# Static Lib #

SET(PROJECT_STATIC_LIBRARY_NAME "${CMAKE_PROJECT_NAME}_static")  
ADD_LIBRARY(${PROJECT_STATIC_LIBRARY_NAME} STATIC ${SOURCES})

TARGET_LINK_LIBRARIES(
	${PROJECT_STATIC_LIBRARY_NAME}
	${LIBRARIES}
)


# Project #

IF(BUILD_server)
	TARGET_LINK_LIBRARIES(
		server
		${LIBRARIES}
	)
ENDIF(BUILD_server)

IF(BUILD_client)
	TARGET_LINK_LIBRARIES(
		client
		${LIBRARIES}
	)
ENDIF(BUILD_client)


###########
## Files ##
###########

IF(BUILD_server)
	FILE(
		COPY 
		${CMAKE_CURRENT_SOURCE_DIR}/config_camera.xml
		${CMAKE_CURRENT_SOURCE_DIR}/config_object.xml
		${CMAKE_CURRENT_SOURCE_DIR}/config_run_parameters.xml
		${CMAKE_CURRENT_SOURCE_DIR}/video_left.avi
		${CMAKE_CURRENT_SOURCE_DIR}/video_right.avi
		DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
	)
ENDIF(BUILD_server)

IF(BUILD_client AND NOT BUILD_server)
	FILE(
		COPY 
		${CMAKE_CURRENT_SOURCE_DIR}/config_run_parameters.xml
		DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
	)
ENDIF(BUILD_client AND NOT BUILD_server)


#############
## Install ##
#############

IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	IF (WIN32 AND NOT UNIX) 
		SET(CMAKE_INSTALL_PREFIX "C:/tiy/" CACHE PATH "Windows Install Prefix" FORCE) # $ENV{HOMEDRIVE}
	ELSE(WIN32 AND NOT UNIX)
		SET(CMAKE_INSTALL_PREFIX "/usr/" CACHE PATH "Unix Install Prefix" FORCE)
	ENDIF(WIN32 AND NOT UNIX)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT) 
    
IF (WIN32 AND NOT UNIX) 
	SET(BIN_REL_PATH "bin/tiy")
	SET(INCLUDE_REL_PATH "include/tiy")
	SET(LIB_REL_PATH "lib/tiy")
ELSE(WIN32 AND NOT UNIX)
	SET(BIN_REL_PATH "local/tiy")
	SET(INCLUDE_REL_PATH "include/tiy")  # NOT in /usr/local/... !
	SET(LIB_REL_PATH "lib/") # NOT in /usr/local/... !
ENDIF(WIN32 AND NOT UNIX)

# Server executable #

IF(BUILD_server)
	INSTALL (
		TARGETS server 
		DESTINATION ${BIN_REL_PATH}
	)
ENDIF(BUILD_server)

# Client executable #

IF(BUILD_client)
	INSTALL (
		TARGETS client 
		DESTINATION ${BIN_REL_PATH} 
	)
ENDIF(BUILD_client)

# Libraries #

INSTALL(
	TARGETS ${PROJECT_SHARED_LIBRARY_NAME} ${PROJECT_STATIC_LIBRARY_NAME}
	RUNTIME DESTINATION ${BIN_REL_PATH} #RUNTIME executable targets and DLL (Windows) style shared libraries.
	LIBRARY DESTINATION ${LIB_REL_PATH} #LIBRARY (non DLL) shared libraries and modules.		
	ARCHIVE DESTINATION ${LIB_REL_PATH} #ARCHIVE static libraries and the import parts of DLL libraries (LIB files). 
)

# Files #

IF(BUILD_server)
	INSTALL(
		FILES 
		${CMAKE_CURRENT_SOURCE_DIR}/config_camera.xml
		${CMAKE_CURRENT_SOURCE_DIR}/config_object.xml
		${CMAKE_CURRENT_SOURCE_DIR}/config_run_parameters.xml
		${CMAKE_CURRENT_SOURCE_DIR}/video_left.avi
		${CMAKE_CURRENT_SOURCE_DIR}/video_right.avi
		DESTINATION ${BIN_REL_PATH} 
		PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ WORLD_WRITE
	)
ENDIF(BUILD_server)

IF(BUILD_client AND NOT BUILD_server)
	INSTALL(
		FILES 
		${CMAKE_CURRENT_SOURCE_DIR}/config_run_parameters.xml
		DESTINATION ${BIN_REL_PATH} 
		PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ WORLD_WRITE
	)
ENDIF(BUILD_client AND NOT BUILD_server)

# Include dirs #

FOREACH(f ${HEADERS})
	GET_FILENAME_COMPONENT(DEST_DIR ${f} PATH)
  	INSTALL(FILES  ${CMAKE_CURRENT_SOURCE_DIR}/${f} DESTINATION ${INCLUDE_REL_PATH}/${DEST_DIR})
ENDFOREACH(f)

GET_FILENAME_COMPONENT(DEST_DIR inputDevice/unix/LinuxDevice_README.txt PATH)
INSTALL(FILES  ${CMAKE_CURRENT_SOURCE_DIR}/inputDevice/unix/LinuxDevice_README.txt DESTINATION ${INCLUDE_REL_PATH}/${DEST_DIR})

INSTALL(
	FILES 
	${CMAKE_CURRENT_SOURCE_DIR}/tiy.h
	${CMAKE_CURRENT_SOURCE_DIR}/LICENCE.txt
  	DESTINATION ${INCLUDE_REL_PATH}
)


###########
## CPack ##
###########

SET(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)

IF(WIN32 AND NOT UNIX)
	SET(CPACK_NSIS_INSTALL_ROOT "C:")
ENDIF(WIN32 AND NOT UNIX)

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Track-It-Yourself (TIY) - A Marker Tracking Library")
SET(CPACK_PACKAGE_VENDOR "Andreas Pflaum")

SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "1")
SET(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${CMAKE_PROJECT_NAME}")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION}") 

IF(WIN32 AND NOT UNIX)
	SET(CPACK_GENERATOR "NSIS")
	SET(CPACK_SOURCE_GENERATOR "ZIP")	
  	SET(CPACK_NSIS_DISPLAY_NAME "Track-It-Yourself(TIY)")
  	SET(CPACK_NSIS_MODIFY_PATH ON)
	IF(BUILD_x64 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
		SET(CPACK_SYSTEM_NAME "win-amd64")
	ELSE(BUILD_x64 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
		SET(CPACK_SYSTEM_NAME "win-i386")
	ENDIF(BUILD_x64 AND CMAKE_SIZEOF_VOID_P EQUAL 8) 
ELSE(WIN32 AND NOT UNIX)
	SET(CPACK_GENERATOR "DEB")
	SET(CPACK_SOURCE_GENERATOR "TGZ")
	SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Andreas Pflaum <email@example.com> ") #required
 	SET(DEBIAN_PACKAGE_SECTION "Video" )
	# TODO: Test (older/less) package dependencies
	SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libopencv-dev (>= 2.2), libboost-dev (>= 1.46), libboost-thread-dev (>= 1.46), libboost-system-dev (>= 1.46), libboost-filesystem-dev (>= 1.46), libboost-date-time1.46-dev (>= 1.46)")
	IF(BUILD_x64 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
		SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64" )
		SET(CPACK_SYSTEM_NAME "linux-amd64")
	ELSE(BUILD_x64 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
		SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386" )
		SET(CPACK_SYSTEM_NAME "linux-i386")
	ENDIF(BUILD_x64 AND CMAKE_SIZEOF_VOID_P EQUAL 8)  	 
ENDIF(WIN32 AND NOT UNIX)

SET(CPACK_PACKAGE_EXECUTABLES "${CMAKE_PROJECT_NAME};Track-It-Yourself(TIY)") #	List of pairs of executables and labels. Used by the NSIS generator to create Start Menu shortcuts.
SET(CPACK_PACKAGE_FILE_NAME "${CPACK_SOURCE_PACKAGE_FILE_NAME}-${CPACK_SYSTEM_NAME}")  
SET(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_BINARY_DIR};${CMAKE_PROJECT_NAME};ALL;/") #Build directory, Project Name, Project Component, Directory in the package 

INCLUDE(CPack)
