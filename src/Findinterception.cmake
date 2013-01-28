##############################################
## Findinterception.txt - by Andreas Pflaum ##
##############################################

# Include dir
find_path(interception_INCLUDE_DIR
	NAMES 
	interception.h
	PATHS 
	IF (WIN32 AND NOT UNIX) 
		"C:/Interception/include"
	ENDIF(WIN32 AND NOT UNIX)
)

# The library itself
find_library(tiy_LIBRARY
	NAMES
	interception
	PATHS 
	IF (WIN32 AND NOT UNIX) 
		"C:/Interception/lib/x86/release"
	ENDIF(WIN32 AND NOT UNIX)
)

#SET(interception_LIBRARIES interception_LIBRARY)