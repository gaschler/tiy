//============================================================================
// Name        : LinuxMouse.cpp
// Author      : Andreas Pflaum
// Licence	   : see LICENCE.txt
//============================================================================

#include "LinuxMouse.h"

namespace tiy
{

LinuxMouse::~LinuxMouse()
{
	{
		boost::mutex::scoped_lock mouse_lock(mouse_mutex);

		is_open = false;
	}

	// FIXME: only a hack (waiting a short period that read_thread thread finishes, else boost lock error occur)
	boost::this_thread::sleep(boost::posix_time::milliseconds(10));
}

bool
LinuxMouse::openAndReadMouse(std::string& event_file_name_, int& read_intervall_ms)
{
	// Open event file of the input device (NONBLOCKING!)
	const char* event_file_name = event_file_name_.c_str();
	event_file_descriptor = open(event_file_name, O_RDONLY | O_NONBLOCK);
	if(!event_file_descriptor)
	{
		std::cerr << "LinuxMouse: open() of file \"" << event_file_name << "\" failed." << std::endl
				  << "(USB dongle connected? Read permission to file? Correct filename?)" << std::endl;
		return false;
	}

	// Enable exclusive use of events (no other application/OS registeres events during this time)
	if ( ioctl(event_file_descriptor, EVIOCGRAB, 1) == -1 )
	{
		std::cerr << "LinuxMouse: ioctl() of file \"" << event_file_name << "\" failed." << std::endl
				  << "(USB dongle connected? Read permission to file? Correct filename?)" << std::endl;
		return false;
	}

	{
		boost::mutex::scoped_lock mouse_lock(mouse_mutex);
		is_open = true;
	}

	boost::thread read_thread(boost::bind(&LinuxMouse::startRead, this, read_intervall_ms));

	return true;
}

void
LinuxMouse::startRead(int& read_intervall_ms)
{
	bool is_left_button_pressed_buffer = false;
	bool is_right_button_pressed_buffer = false;

	while(true)
	{
		int mouse_wheel_position_buffer;
		{
			boost::mutex::scoped_lock mouse_lock(mouse_mutex);

			if (!is_open)
				return;

			mouse_wheel_position_buffer = mouse_wheel_position;
		}

		bool was_left_button_pressed_buffer = false;
		bool was_left_button_released_buffer = false;
		bool was_right_button_pressed_buffer = false;
		bool was_right_button_released_buffer = false;
		bool has_mouse_wheel_changed_buffer = false;

		{
			boost::mutex::scoped_lock mouse_lock(mouse_mutex);

			is_right_button_pressed = is_right_button_pressed_buffer;
			is_left_button_pressed = is_left_button_pressed_buffer;
		}

		while ((read(event_file_descriptor, &event_start, sizeof(event_start)) > 0) && (read(event_file_descriptor, &event_end, sizeof(event_end)) > 0))
		{
			if (do_debugging)
			{
				std::cout << "LinuxMouse: event_start.type = " <<  event_start.type << ", event_start.code = " << event_start.code << ", event_start.value = " << event_start.value << std::endl
						  << "             event_end.type = " <<  event_end.type << ", event_end.code = " << event_end.code << ", event_end.value = " << event_end.value << std::endl;
			}

			switch (event_start.type)
			{
				case EV_KEY:	// ==1
				case EV_MSC:	// ==4
					// LEFT mouse BUTTON pressed/released
					if 	((event_start.code == 272) || ((event_start.code == 4) && (event_start.value == 589825)))
					{
						if (is_left_button_pressed_buffer == true)
							was_left_button_released_buffer = true;
						else
							was_left_button_pressed_buffer = true;

						is_left_button_pressed_buffer = !is_left_button_pressed_buffer;
					}
					// RIGHT mouse BUTTON pressed/released
					else if ((event_start.code == 273) || ((event_start.code == 4) && (event_start.value == 589826)))
					{
						if (is_right_button_pressed_buffer == true)
							was_right_button_released_buffer = true;
						else
							was_right_button_pressed_buffer = true;

						is_right_button_pressed_buffer = !is_right_button_pressed_buffer;
					}
					break;

				case EV_REL:	// ==2
					// mouse WHEEL moved
					if ((event_start.code == 8) && (event_start.value != 0))
					{
						has_mouse_wheel_changed_buffer = true;
						mouse_wheel_position_buffer = mouse_wheel_position_buffer + event_start.value;
					}
					break;
			}

			updateStatus(was_left_button_pressed_buffer, was_left_button_released_buffer, is_left_button_pressed_buffer,
							was_right_button_pressed_buffer, was_right_button_released_buffer, is_right_button_pressed_buffer,
								has_mouse_wheel_changed_buffer, mouse_wheel_position_buffer);
		}

		boost::this_thread::sleep(boost::posix_time::milliseconds(read_intervall_ms));
	}
}

}