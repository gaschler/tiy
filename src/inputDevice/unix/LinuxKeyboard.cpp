//============================================================================
// Name        : LinuxKeyboard.cpp
// Author      : Andreas Pflaum
// Licence	   : see LICENCE.txt
//============================================================================

#include "LinuxKeyboard.h"

namespace tiy
{

LinuxKeyboard::~LinuxKeyboard()
{
	{
		boost::mutex::scoped_lock keyboard_lock(keyboard_mutex);

		is_open = false;
	}

	// FIXME: only a hack (waiting a short period that read_thread thread finishes, else boost lock error occur)
	boost::this_thread::sleep(boost::posix_time::milliseconds(10));
}

bool
LinuxKeyboard::openAndReadKeyboard(std::string& event_file_name_, int& read_intervall_ms)
{
	// Open event file of input device (NONBLOCKING!)
	const char* event_file_name = event_file_name_.c_str();
	event_file_descriptor = open(event_file_name, O_RDONLY | O_NONBLOCK);
	if(!event_file_descriptor)
	{
		std::cerr << "LinuxKeyboard: open() of file \"" << event_file_name << "\" failed." << std::endl
				  << "(USB keyboard connected? Read permission to file? Correct filename?)" << std::endl;
		return false;
	}

//	// Enable exclusive use of events
//	if ( ioctl(event_file_descriptor, EVIOCGRAB, 1) == -1 )
//	{
//		std::cerr << "LinuxKeyboard: ioctl() of file \"" << event_file_name << "\" failed." << std::endl
//				  << "(USB keyboard connected? Read permission to file? Correct filename?)" << std::endl;
//		return false;
//	}

	{
		boost::mutex::scoped_lock keyboard_lock(keyboard_mutex);
		is_open = true;
	}

	boost::thread read_thread(boost::bind(&LinuxKeyboard::startRead, this, read_intervall_ms));

	return true;
}

void
LinuxKeyboard::startRead(int& read_intervall_ms)
{
	while(true)
	{
		{
			boost::mutex::scoped_lock keyboard_lock(keyboard_mutex);

			if (!is_open)
				return;
		}

		bool was_SPACE_pressed_buffer = false;
		bool was_ESC_pressed_buffer = false;

		while ((read(event_file_descriptor, &event_start, sizeof(event_start)) > 0) && (read(event_file_descriptor, &event_end, sizeof(event_end)) > 0))
		{
			if (do_debugging)
			{
				std::cout << "LinuxKeyboard: event_start.type = " <<  event_start.type << ", event_start.code = " << event_start.code << ", event_start.value = " << event_start.value << std::endl
						  << "             event_end.type = " <<  event_end.type << ", event_end.code = " << event_end.code << ", event_end.value = " << event_end.value << std::endl;
			}

			switch (event_end.type)
			{
				case EV_KEY:	// ==4
					// SPACE key pressed
					if 	((event_end.code == KEY_SPACE) && (event_end.value == 1))
						was_SPACE_pressed_buffer = true;

					// ESC key pressed
					else if ((event_end.code == KEY_ESC) && (event_end.value == 1))
						was_ESC_pressed_buffer = true;

					updateStatus(was_SPACE_pressed_buffer, was_ESC_pressed_buffer);
					break;
			}
		}

		boost::this_thread::sleep(boost::posix_time::milliseconds(read_intervall_ms));
	}
}

}