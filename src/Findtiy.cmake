#####################################
## Findtiy.txt - by Andreas Pflaum ##
#####################################

# Include dir
find_path(tiy_INCLUDE_DIR
	NAMES 
	tiy.h
	PATHS 
	IF (WIN32 AND NOT UNIX) 
		"C:/tiy/include/tiy"
	ELSE(WIN32 AND NOT UNIX)
		/usr/include/tiy
		/usr/local/include/tiy
	ENDIF(WIN32 AND NOT UNIX)
)

# The library itself
find_library(tiy_LIBRARY
	NAMES
	tiy_static
	PATHS 
	IF (WIN32 AND NOT UNIX) 
		"C:/tiy/lib/tiy"
	ELSE(WIN32 AND NOT UNIX)
		/usr/lib
		/usr/local/lib
	ENDIF(WIN32 AND NOT UNIX)
)

#SET(tiy_LIBRARIES tiy_LIBRARY)
