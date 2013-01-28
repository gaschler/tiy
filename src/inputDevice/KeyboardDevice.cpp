//============================================================================
// Name        : KeyboardDevice.cpp
// Author      : Andreas Pflaum
// Licence	   : see LICENCE.txt
//============================================================================

#include "KeyboardDevice.h"

namespace tiy
{

KeyboardDevice::KeyboardDevice(bool& do_debugging_) :
		do_debugging(do_debugging_)
{
	{
		boost::mutex::scoped_lock keyboard_lock(keyboard_mutex);

		is_open = false;
		was_SPACE_pressed = false;
		was_ESC_pressed = false;
	}
}


void
KeyboardDevice::getStatusSinceLastReset(bool& was_SPACE_pressed_, bool& was_ESC_pressed_)
{
	{
		boost::mutex::scoped_lock keyboard_lock(keyboard_mutex);

		if (is_open)
		{
			was_SPACE_pressed_ = was_SPACE_pressed;
			was_ESC_pressed_ = was_ESC_pressed;
			return;
		}
	}

	std::cerr << "KeyboardDevice: getStatusSinceLastReset() - keyboard NOT open" << std::endl;
}


void
KeyboardDevice::updateStatus(bool& was_SPACE_pressed_, bool& was_ESC_pressed_)
{
	{
		boost::mutex::scoped_lock keyboard_lock(keyboard_mutex);

		// logical OR key event variables (-> only ONE "press"-event is buffered per key)
		was_SPACE_pressed = was_SPACE_pressed || was_SPACE_pressed_;
		was_ESC_pressed = was_ESC_pressed || was_ESC_pressed_;
	}

	if (do_debugging)
	{
		std::cout << "KeyboardDevice: was_SPACE_pressed = " << was_SPACE_pressed << std::endl
				  << "			   was_ESC_pressed = " << was_ESC_pressed << std::endl;
	}
}


void
KeyboardDevice::resetStatus()
{
	{
		boost::mutex::scoped_lock keyboard_lock(keyboard_mutex);

		was_SPACE_pressed = false;
		was_ESC_pressed = false;
	}
}

}