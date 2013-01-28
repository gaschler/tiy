//============================================================================
// Name        : WindowsKeyboard.cpp
// Author      : Andreas Pflaum
// Licence	   : see LICENCE.txt
//============================================================================

#include "WindowsKeyboard.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <setupapi.h>
#include <cfgmgr32.h>

#pragma comment (lib, "setupapi.lib")

namespace tiy
{

WindowsKeyboard::~WindowsKeyboard()
{
	{
		boost::mutex::scoped_lock keyboard_lock(keyboard_mutex);

		if (is_open)
			interception_destroy_context(context);

		is_open = false;
	}
}

bool
WindowsKeyboard::openAndReadKeyboard(std::string& hardware_id_, int& read_intervall_ms)
{
	raise_process_priority();	

    context = interception_create_context();

    interception_set_filter(context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_DOWN);

	{
		boost::mutex::scoped_lock keyboard_lock(keyboard_mutex);
		is_open = true;
	}

	boost::thread read_thread(boost::bind(&WindowsKeyboard::startRead, this, read_intervall_ms));

	return true;
}

void
WindowsKeyboard::startRead(int& read_intervall_ms)
{
	while(interception_receive(context, device = interception_wait(context), &stroke, 1) > 0)
	{
		{
			boost::mutex::scoped_lock keyboard_lock(keyboard_mutex);

			if (!is_open)
				return;
		}

		if(interception_is_keyboard(device))
		{
			bool was_SPACE_pressed_buffer = false;
			bool was_ESC_pressed_buffer = false;

			InterceptionKeyStroke &keyboard_stroke = *(InterceptionKeyStroke *) &stroke;

			switch(keyboard_stroke.code)
			{
				case SCANCODE_ESC:
					was_ESC_pressed_buffer = true;
					break;
				case SCANCODE_SPACE:
				case SCANCODE_SPACE_2:
					was_SPACE_pressed_buffer = true;
					break;
				default:
					break;
			}

			updateStatus(was_SPACE_pressed_buffer, was_ESC_pressed_buffer);
		}
       
		// "Free" all intercepted events (for other applications/OS)
		interception_send(context, device, &stroke, 1);
	}
}

}