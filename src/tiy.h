//============================================================================
// Name        : tiy.h
// Author      : Andreas Pflaum
// Description : Track-It-Yourself (TIY) - Marker tracking library (see homepage)
// Licence	   : see LICENCE.txt
//============================================================================

#ifndef TIY_H_
#define TIY_H_

#include "multicastServer/MulticastServer.h"
#include "multicastClient/MulticastClient.h"
#include "markerTracking/MarkerTracking.h"
#include "stereoCam/StereoCamera.h"
#include "stereoCam/OpenCVStereoCamera.h"
#include "inputDevice/MouseDevice.h"
#include "inputDevice/KeyboardDevice.h"

#ifdef WIN32
	#include "inputDevice/win/interception/utils.h"
	#include "inputDevice/win/interception/interception.h"
	#include "inputDevice/win/WindowsMouse.h"
	#include "inputDevice/win/WindowsKeyboard.h"
#else
	#include "inputDevice/unix/LinuxMouse.h"
	#include "inputDevice/unix/LinuxKeyboard.h"
#endif

#ifdef USE_aravis
	#include "stereoCam/unix/BaslerGigEStereoCamera.h"
#endif

#endif // TIY_H_
