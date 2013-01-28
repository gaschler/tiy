//============================================================================
// Name        : LinuxKeyboard.h
// Author      : Andreas Pflaum
// Description : See parent class KeyboardDevice for a general description
//				 - Keyboard events read directly out of the specified event-file
//				   (see LinuxDevice_README.txt for setting up a udev rule)
//				 - "event_file_name" contains the path to the event-file
//				   (e.g. "/dev/input/event4" or from an own udev rule e.g.
//					"/dev/input/dell-keyboard-event")
// Licence	   : see LICENCE.txt
//============================================================================

#ifndef LINUX_KEYBOARD_H_
#define LINUX_KEYBOARD_H_

#include "../KeyboardDevice.h"

#include <cstring>
#include <linux/input.h>
#include <fcntl.h>

namespace tiy
{

class LinuxKeyboard : public KeyboardDevice
{
private:

	int event_file_descriptor;

	struct input_event event_start, event_end;

private:

	virtual void startRead(int& read_intervall_ms);

public:

	LinuxKeyboard(bool& do_debugging_) : KeyboardDevice(do_debugging_)
	{
		memset(&event_start, 0, sizeof(event_start));
		memset(&event_start, 0, sizeof(event_end));
	}

	virtual ~LinuxKeyboard();

	virtual bool openAndReadKeyboard(std::string& event_file_name, int& read_intervall_ms);
};

}

#endif // LINUX_KEYBOARD_H_
