//============================================================================
// Name        : MouseDevice.h
// Author      : Andreas Pflaum
// Description : Parent class for mouse reading, based on mouse button event/ 
//				 status variables, updated in a mouse reading loop (own thread). 
//				 - Users have to reset the variables manually
//				 - Only ONE "press"- and "release"-event is buffered since 
//				   last reset
//				 - While opened, the mouse is blocked for all other applications
//				 Child classes: WindowsMouse and LinuxMouse
// Licence	   : see LICENCE.txt
//============================================================================

#ifndef MOUSE_DEVICE_H_
#define MOUSE_DEVICE_H_

#include <boost/thread.hpp>

#include <iostream>

namespace tiy
{

class MouseDevice
{
protected:

	boost::mutex mouse_mutex;

	bool do_debugging;
	bool is_open;

	// Mouse event variables
	bool was_left_button_pressed, was_left_button_released, was_right_button_pressed, was_right_button_released, has_mouse_wheel_changed;
	// Mouse status variables
	bool is_left_button_pressed, is_right_button_pressed;
	int mouse_wheel_position;

protected:
	// Start reading the mouse in a loop (called by openAndReadMouse() in a new thread)
	virtual void startRead(int& read_intervall_ms) = 0;

	// Update (set) the event/status variables
	void updateStatus(bool& was_left_button_pressed_, bool& was_left_button_released_, bool& is_left_button_pressed_,
						 bool& was_right_button_pressed_,bool& was_right_button_released_,bool& is_right_button_pressed_,
						 	 bool& has_mouse_wheel_changed_, int& mouse_wheel_position_);

public:

	MouseDevice(bool& do_debugging_);

	virtual ~MouseDevice() {};

	// Connect to the mouse and start reading it (starts "startRead()" in a new thread)
	virtual bool openAndReadMouse(std::string& event_file_name, int& read_intervall_ms) = 0;

	// Get the values of the event/status variables (without resetting!)
	void getStatusSinceLastReset(bool& was_left_button_pressed_, bool& was_left_button_released_, bool& is_left_button_pressed_,
								 	 bool& was_right_button_pressed_,bool& was_right_button_released_,bool& is_right_button_pressed_,
								 	 	 bool& has_mouse_wheel_changed_, int& mouse_wheel_position_);

	// Reset the values of the event (NOT status) variables (should usually be called directly after getStatusSinceLastReset())
	void resetStatus();
};

}

#endif // MOUSE_DEVICE_H_
