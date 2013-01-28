//============================================================================
// Name        : MouseDevice.cpp
// Author      : Andreas Pflaum
// Licence	   : see LICENCE.txt
//============================================================================

#include "MouseDevice.h"

namespace tiy
{

MouseDevice::MouseDevice(bool& do_debugging_) :
		do_debugging(do_debugging_)
{
	{
		boost::mutex::scoped_lock mouse_lock(mouse_mutex);

		is_open = false;
		was_left_button_pressed = false;
		was_left_button_released = false;
		was_right_button_pressed = false;
		was_right_button_released = false;
		has_mouse_wheel_changed = false;
		is_right_button_pressed = false;
		is_left_button_pressed = false;
		mouse_wheel_position = 0;
	}
}


void
MouseDevice::getStatusSinceLastReset(bool& was_left_button_pressed_, bool& was_left_button_released_, bool& is_left_button_pressed_,
								     bool& was_right_button_pressed_,bool& was_right_button_released_,bool& is_right_button_pressed_,
								     bool& has_mouse_wheel_changed_, int& mouse_wheel_position_)
{
	{
		boost::mutex::scoped_lock mouse_lock(mouse_mutex);

		if (is_open)
		{
			was_left_button_pressed_ = was_left_button_pressed;
			was_left_button_released_ = was_left_button_released;
			was_right_button_pressed_ = was_right_button_pressed;
			was_right_button_released_ = was_right_button_released;
			has_mouse_wheel_changed_ = has_mouse_wheel_changed;
			mouse_wheel_position_ = mouse_wheel_position;
			is_right_button_pressed_ = is_right_button_pressed;
			is_left_button_pressed_ = is_left_button_pressed;
			return;
		}
	}

	std::cerr << "MouseDevice: getStatusSinceLastReset() - mouse NOT open" << std::endl;
}

void
MouseDevice::updateStatus(bool& was_left_button_pressed_, bool& was_left_button_released_, bool& is_left_button_pressed_,
						 	 bool& was_right_button_pressed_,bool& was_right_button_released_,bool& is_right_button_pressed_,
						 	 	 bool& has_mouse_wheel_changed_, int& mouse_wheel_position_)
{
	{
		boost::mutex::scoped_lock mouse_lock(mouse_mutex);

		// logical OR mouse event variables (-> only ONE "press"- and "release"-event is buffered per button)
		was_left_button_pressed = was_left_button_pressed || was_left_button_pressed_;
		was_left_button_released = was_left_button_released || was_left_button_released_;
		was_right_button_pressed = was_right_button_pressed || was_right_button_pressed_;
		was_right_button_released = was_right_button_released || was_right_button_released_;
		has_mouse_wheel_changed = has_mouse_wheel_changed || has_mouse_wheel_changed_;
		mouse_wheel_position = mouse_wheel_position_;
		is_right_button_pressed = is_right_button_pressed_;
		is_left_button_pressed = is_left_button_pressed_;
	}

	if (do_debugging)
	{
		std::cout << "MouseDevice: was_left_button_pressed = " << was_left_button_pressed << std::endl
				  << "			   was_left_button_released = " << was_left_button_released << std::endl
				  << "			   was_right_button_pressed = " << was_right_button_pressed << std::endl
				  << "			   was_right_button_released = " << was_right_button_released << std::endl
				  << "			   has_mouse_wheel_changed = " << has_mouse_wheel_changed << std::endl
				  << "			   mouse_wheel_position = " << mouse_wheel_position << std::endl
				  << "			   is_right_button_pressed = " << is_right_button_pressed << std::endl
				  << "			   is_left_button_pressed = " << is_left_button_pressed << std::endl;
	}
}


void
MouseDevice::resetStatus()
{
	{
		boost::mutex::scoped_lock mouse_lock(mouse_mutex);

			was_left_button_pressed = false;
			was_left_button_released = false;
			was_right_button_pressed = false;
			was_right_button_released = false;
			has_mouse_wheel_changed = false;
	}
}

}