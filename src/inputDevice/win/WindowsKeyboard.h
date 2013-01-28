//============================================================================
// Name        : WindowsKeyboard.h
// Author      : Andreas Pflaum
// Description : See parent class KeyboardDevice for a general description
//				 - Keyboard events intercepted by the "interception" interface
//				   (http://oblita.com/Interception) and "freed" afterwards
// Licence	   : see LICENCE.txt
//============================================================================

#ifndef WINDOWS_KEYBOARD_HPP
#define WINDOWS_KEYBOARD_HPP

#include "../KeyboardDevice.h"
#include "interception/interception.h"
//#include <interception.h>
#include "interception/utils.h"

#include <string>
#include <fstream>

namespace tiy
{

// Key codes
enum ScanCode
{
    SCANCODE_ESC = 0x01,
    SCANCODE_SPACE = 0x5e,
	SCANCODE_SPACE_2 = 0x39
};
//# [esc]       [ f1] [ f2] [ f3] [ f4] [ f5] [ f6] [ f7] [ f8] [ f9] [f10] [f11] [f12]
//#  0x01        0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d
//#
//# [ ` ] [ 1 ] [ 2 ] [ 3 ] [ 4 ] [ 5 ] [ 6 ] [ 7 ] [ 8 ] [ 9 ] [ 0 ] [ - ] [ = ] [bck]
//#  0x11  0x12  0x13  0x14  0x15  0x16  0x17  0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e
//#
//# [tab] [ q ] [ w ] [ e ] [ r ] [ t ] [ y ] [ u ] [ i ] [ o ] [ p ] [ [ ] [ ] ] [ \ ]
//#  0x26  0x27  0x28  0x29  0x2a  0x2b  0x2c  0x2d  0x2e  0x2f  0x30  0x31  0x32  0x33
//#
//# [cap] [ a ] [ s ] [ d ] [ f ] [ g ] [ h ] [ j ] [ k ] [ l ] [ ; ] [ ' ] [  enter  ]
//#  0x3b  0x3c  0x3d  0x3e  0x3f  0x40  0x41  0x42  0x43  0x44  0x45  0x46     0x47
//#
//# [shift]     [ z ] [ x ] [ c ] [ v ] [ b ] [ n ] [ m ] [ , ] [ . ] [ / ]     [shift]
//#   0x4b       0x4c  0x4d  0x4e  0x4f  0x50  0x51  0x52  0x53  0x54  0x55       0x56
//#
//# [ctr]             [cmd]             [  space  ]             [cmd]             [ctr]
//#  0x5c              0x5d                 0x5e                 0x5f              0x60


class WindowsKeyboard : public KeyboardDevice
{
private:

    InterceptionContext context;
    InterceptionDevice device;
    InterceptionStroke stroke;

private:
	virtual void startRead(int& read_intervall_us);

public:

	WindowsKeyboard(bool& do_debugging_) : KeyboardDevice(do_debugging_) {};

	virtual ~WindowsKeyboard();

	virtual bool openAndReadKeyboard(std::string& hardware_id_, int& read_intervall_ms);
};

}

#endif // WINDOWS_KEYBOARD_HPP
