//============================================================================
// Name        : LinuxMouse.h
// Author      : Andreas Pflaum
// Description : See parent class MouseDevice for a general description
//				 - Mouse events read directly out of the specified event-file
//				   (see LinuxDevice_README.txt for setting up a udev rule)
//				 - "event_file_name" contains the path to the event-file
//				   (e.g. "/dev/input/event4" or from an own udev rule e.g.
//					"/dev/input/genius-event")
// Licence	   : see LICENCE.txt
//============================================================================

#ifndef LINUX_MOUSE_H_
#define LINUX_MOUSE_H_

#include "../MouseDevice.h"

#include <cstring>
#include <linux/input.h>
#include <fcntl.h>

namespace tiy
{

class LinuxMouse : public MouseDevice
{
private:

	int event_file_descriptor;

	struct input_event event_start, event_end;

private:

	virtual void startRead(int& read_intervall_ms);

public:

	LinuxMouse(bool& do_debugging_) : MouseDevice(do_debugging_)
	{
		memset(&event_start, 0, sizeof(event_start));
		memset(&event_start, 0, sizeof(event_end));
	}

	virtual ~LinuxMouse();

	virtual bool openAndReadMouse(std::string& event_file_name, int& read_intervall_ms);
};

}

#endif // LINUX_MOUSE_H_
