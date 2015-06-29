The LinuxMouse class is the child class of [MouseDevice](http://code.google.com/p/tiy/wiki/ClassMouseDevice) for low level mouse reading in Linux.

It is based on reading out the event file of a specific (mouse) device. This is done in a while loop started in a separate thread.

# Usage #

  * The mouse device is blocked for ALL other applications (incl. OS)
  * Press and release events of the left and right mouse buttons are implemented, as well as turning the mouse wheel
  * Only ONE press and one release event buffered per button since last button status reset
  * Users have to reset the button status manually (usually after each status request)

## Creating a specific event-file (optional) ##
As the event files _event0_,_event1_,... created by unix in _/dev/input/_ can change at every start or re-plug, the best way is to build a udev rule to connect a specific device to a specific event-file:

> First get the vendor id (vid) and product id (pid) of your device. A good way is to plug the device out, run
```
$ lsusb
```
> , plug the device in, run _lsusb_ again and watch for a new line like
```
Bus 004 Device 005: ID 0458:00c5 KYE Systems Corp. (Mouse Systems)
```
> vid: 0458, pid: 00c5

> Second add and set a new udev rule (e.g. "66-my-mouse-usb.rules")
```
$ sudo gedit /etc/udev/rules.d/66-genius-mouse-usb.rules
```
> and write (e.g. of the vid and pid above and the new event-file _/dev/input/my-mouse-event_
```
KERNEL=="event*",SUBSYSTEM=="input",ATTRS{idVendor}=="0458",ATTRS{idProduct}=="00c5",MODE="0644"
KERNEL=="event*",SUBSYSTEM=="input",ATTRS{idVendor}=="0458",ATTRS{idProduct}=="00c5",SYMLINK+="input/my-mouse-event"
```

> Third save the file and restart the computer to activate the rule.

## Example ##

See [IncludeLibrary](http://code.google.com/p/tiy/wiki/IncludeLibrary) on how to include the TIY library in your own code (e.g. this example).

```
#include <tiy.h>
//#include <pwd.h>
 

int main(int argc, char* argv[])
{
  std::string mouse_device_id = "/dev/input/my-mouse-event";
  int read_intervall_ms = 1;  
  bool do_debugging = false;

  // 1. Create a LinuxMouse object (here from an MouseDevice object)
  boost::scoped_ptr<tiy::MouseDevice> mouse_device;
  mouse_device.reset(new tiy::LinuxMouse(do_debugging));

  // 2. Connect to a specific mouse device and start reading it
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
	int event_file_descriptor;

	struct input_event event_start, event_end;

private:

	virtual void startRead(int& read_intervall_ms);

public:

	LinuxMouse(bool& do_debugging_) : MouseDevice(do_debugging_);

	virtual ~LinuxMouse();

	virtual bool openAndReadMouse(std::string& event_file_name, int& read_intervall_ms);
```

# Methods #

---

**startRead()**
```
	virtual void startRead(int& read_intervall_ms);
```
Start reading the mouse in an update loop (called by **openAndReadMouse()** in a new thread)
> _read`_`intervall`_`ms_: pause in milliseconds after each mouse update (one loop pass)

---

**LinuxMouse()**
```
	LinuxMouse(bool& do_debugging_);
```
> _do`_`debugging_: set to true for debug output

---

**`~`LinuxMouse()**
```
	virtual ~LinuxMouse();
```

---

**openAndReadMouse()**
```
	virtual bool openAndReadMouse(std::string& event_file_name, int& read_intervall_ms) = 0;
```
Connect to the mouse and start reading it (starts **startRead()** in a new thread)
> _event`_`file`_`name_: path to event file of the device (e.g. "/dev/input/event3")

> _read`_`intervall`_`ms_: pause in milliseconds after each mouse update

---
