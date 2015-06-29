The WindowsMouse class is the child class of [MouseDevice](http://code.google.com/p/tiy/wiki/ClassMouseDevice) for low level mouse reading in Windows.

It is based on [intercepting](http://oblita.com/Interception.html) mouse events of a specific (mouse) device. This is done in a while loop started in a separate thread.

# Usage #

  * The mouse device is blocked for ALL other applications (incl. OS)
  * Press and release events of the left and right mouse buttons are implemented, as well as turning the mouse wheel
  * Only ONE press and one release event buffered per button since last button status reset
  * Users have to reset the button status manually (usually after each status request)

## Getting the hardware id of a specific mouse ##

In order to use and block only one specific mouse connected, this device needs to be identified by its hardware id. The WindowsMouse class needs a string with the components "VID`_`xxxx" (vendor id) and "PID`_`xxxx" (product id) like "VID\_046DPID\_C52B".

The vid and pid can easily be get by going into the Windows _Device Manager_ -> right clicking on the device -> _Properties_ -> _Details_ tab -> choosing the _Property_: _Hardware-IDs_ and copying one of the hids containing the strings "VID\_xxx" and "PID\_xxx".

## Example ##

See [IncludeLibrary](http://code.google.com/p/tiy/wiki/IncludeLibrary) on how to include the TIY library in your own code (e.g. this example).

```
#include <tiy.h>

int main(int argc, char* argv[])
{
  std::string mouse_device_id = "VID_046DPID_C52B";
  int read_intervall_ms = 1; //Not used
  bool do_debugging = false;

  // 1. Create a WindowsMouse object (here from a MouseDevice object)
  boost::scoped_ptr<tiy::MouseDevice> mouse_device;
  mouse_device.reset(new tiy::WindowsMouse(do_debugging));

  // 2. Connect to a all mouse devices and start reading them
  if (!mouse_device->openAndReadMouse(mouse_device_id, read_intervall_ms))
  {
	std::cout << "MouseDevice::openAndReadMouse() failed" << std::endl;
        std::cin.get();
	return 0;
  }
 
  std::cout << "Waiting for mouse inputs (click RIGHT button to exit)..." << std::endl;

  while(true)
  {
      bool was_left_button_pressed=false, was_left_button_released=false, is_left_button_pressed=false,
	    was_right_button_pressed=false, was_right_button_released=false, is_right_button_pressed=false,
	     has_mouse_wheel_changed=false;
      static int mouse_wheel_position=0;

      // 3. Get the status of the buttons
      mouse_device->getStatusSinceLastReset(was_left_button_pressed, was_left_button_released, is_left_button_pressed,
					     was_right_button_pressed, was_right_button_released, is_right_button_pressed,
					      has_mouse_wheel_changed, mouse_wheel_position);
      // 4. Reset the status of the buttons
      mouse_device->resetStatus();

      if (was_left_button_pressed)
	  std::cout << "LEFT" << std::endl;
      if (is_left_button_pressed)
	  std::cout << "LEFT STILL" << std::endl;
      if (was_left_button_released)
	  std::cout << "LEFT RELEASED" << std::endl;
      if (was_right_button_pressed)
	  std::cout << "RIGHT" << std::endl;
      if (is_right_button_pressed)
	  std::cout << "RIGHT STILL" << std::endl;
      if (was_right_button_released)
      {
	  std::cout << "RIGHT RELEASED" << std::endl;
          std::cin.get();
          break;
      }
      if (has_mouse_wheel_changed)
	  std::cout << "WHEEL: " << mouse_wheel_position << std::endl;

      boost::this_thread::sleep(boost::posix_time::milliseconds(read_intervall_ms/2));
  }

  return 0;
}

```

# Declaration #

```
private:

        InterceptionContext context;
        InterceptionDevice device;
        InterceptionStroke stroke;

	std::string device_hardware_id_str, device_vid, device_pid;
        wchar_t hardware_id[500];

private:
	virtual void startRead(int& read_intervall_us);

public:

	WindowsMouse(bool& do_debugging_) : MouseDevice(do_debugging_) {};

	virtual ~WindowsMouse();

	virtual bool openAndReadMouse(std::string& hardware_id_, int& read_intervall_ms);
```

# Methods #

---

**startRead()**
```
	virtual void startRead(int& read_intervall_ms);
```
Start reading the mouse in an update loop (called by **openAndReadMouse()** in a new thread)
> _read`_`intervall`_`ms_: NOT used

---

**WindowsMouse()**
```
	WindowsMouse(bool& do_debugging_);
```
> _do`_`debugging_: set to true for debug output

---

**`~`WindowsMouse()**
```
	virtual ~WindowsMouse();
```

---

**openAndReadMouse()**
```
	virtual bool openAndReadMouse(std::string& event_file_name, int& read_intervall_ms) = 0;
```
Connect to the keyboard and start reading it (starts **startRead()** in a new thread)
> _hardware`_`id_: hardware id of the device (e.g. "VID\_046DPID\_C52B")

> _read`_`intervall`_`ms_: NOT used

---
