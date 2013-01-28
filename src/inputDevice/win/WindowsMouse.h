//============================================================================
// Name        : WindowsMouse.h
// Author      : Andreas Pflaum
// Description : See parent class MouseDevice for a general description
//				 - Mouse events intercepted by the "interception" interface
//				   (http://oblita.com/Interception)
//				 - Only the mouse, specified by VID and PID, is considered
//				   ("hardware_id_" must contain "VID_xxx" and "PID_xxx")
//============================================================================

#ifndef WINDOWS_MOUSE_HPP
#define WINDOWS_MOUSE_HPP

#include "../MouseDevice.h"
#include "interception/interception.h"
//#include <interception.h>
#include "interception/utils.h"

#include <string>
#include <fstream>

namespace tiy
{

class WindowsMouse : public MouseDevice
{
private:

    InterceptionContext context;
    InterceptionDevice device;
    InterceptionStroke stroke;

	std::string device_hardware_id_str, device_vid, device_pid;
    wchar_t hardware_id[500];

private:

	virtual void startRead(int& read_intervall_us);

public:

	WindowsMouse(bool& do_debugging_) : MouseDevice(do_debugging_) {};

	virtual ~WindowsMouse();

	virtual bool openAndReadMouse(std::string& hardware_id_, int& read_intervall_us);
};

}

#endif // WINDOWS_MOUSE_HPP
