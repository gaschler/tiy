//============================================================================
// Name        : KeyboardDevice.h
// Author      : Andreas Pflaum
// Description : Parent class for keyboard reading, based on keystroke event 
//				 variables, updated in a keyboard reading loop (own thread). 
//				 - Users have to reset the variables manually
//				 - Only ONE "press"-event is buffered per key since last reset			 
//				 - Only SPACE and ESC key implemented (but easily extendible)
//				 - The keyboard is NOT blocked for other apllications/OS
//				 Child classes: WindowsKeyboard and LinuxKeyboard
//============================================================================

#ifndef KEYBOARD_DEVICE_H_
#define KEYBOARD_DEVICE_H_

#include <boost/thread.hpp>

#include <iostream>


namespace tiy
{

class KeyboardDevice
{
protected:

	boost::mutex keyboard_mutex;

	bool do_debugging;
	bool is_open;

	// Keystroke event variables
	bool was_SPACE_pressed, was_ESC_pressed;

protected:
	// Start reading the keyboard in a loop (called by openAndReadKeyboard() in a new thread)
	virtual void startRead(int& read_intervall_ms) = 0;

	// Update (set) the event variables
	void updateStatus(bool& was_SPACE_pressed_, bool& was_ESC_pressed_);

public:

	KeyboardDevice(bool& do_debugging_);

	virtual ~KeyboardDevice() {};

	// Connect to the keyboard and start reading it (starts "startRead()" in a new thread)
	virtual bool openAndReadKeyboard(std::string& event_file_name, int& read_intervall_ms) = 0;

	// Get the values of the event variables (without resetting!)
	void getStatusSinceLastReset(bool& was_SPACE_pressed_, bool& was_ESC_pressed_);

	// Reset the values of the event variables (should usually be called directly after getStatusSinceLastReset())
	void resetStatus();
};

}

#endif // KEYBOARD_DEVICE_H_
