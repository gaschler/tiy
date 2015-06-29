The LinuxKeyboard class is the child class of [KeyboardDevice](http://code.google.com/p/tiy/wiki/ClassKeyboardDevice) for low level keyboard reading in Linux.

It is based on reading out the event file of a specific (keyboard) device. This is done in a while loop started in a separate thread.

# Usage #

  * The keyboard device is NOT blocked for other applications/OS
  * Only SPACE and ESC keys are implemented (but easily extendible in the code)
  * Only ONE key press-event is buffered per key since last reset
  * Users have to reset the key event variables manually

## Creating a specific event-file (optional) ##
As the event files _event0_,_event1_,... created by Unix in _/dev/input/_ can change at every start or re-plug, the best way is to build a _udev_ rule to connect a specific device to a specific event-file:

> First get the vendor id (vid) and product id (pid) of your device. A good way is to plug the device out, run
```
$ lsusb
```
> , plug the device in, run _lsusb_ again and watch for a new line like
```
Bus 004 Device 005: ID 0458:00c5 KYE Systems Corp. (Keyboard Systems)
```
> vid: 0458, pid: 00c5

> Second add and set a new _udev_ rule (e.g. "66-my-keyboard-usb.rules")
```
$ sudo gedit /etc/udev/rules.d/66-my-keyboard-usb.rules
```
> and write (e.g. of the vid and pid above and the new event-file _/dev/input/my-keyboard-event_
```
KERNEL=="event*",SUBSYSTEM=="input",ATTRS{idVendor}=="0458",ATTRS{idProduct}=="00c5",MODE="0644"
KERNEL=="event*",SUBSYSTEM=="input",ATTRS{idVendor}=="0458",ATTRS{idProduct}=="00c5",SYMLINK+="input/my-keyboard-event"
```

> Third save the file and restart the computer to activate the rule.

## Example ##

See [IncludeLibrary](http://code.google.com/p/tiy/wiki/IncludeLibrary) on how to include the TIY library in your own code (e.g. this example).

```
#include <tiy.h>
//#include <pwd.h>
 

int main(int argc, char* argv[])
{
  std::string keyboard_device_id = "/dev/input/my-keyboard-event";
  int read_intervall_ms = 1;  
  bool do_debugging = false;

  // 1. Create a LinuxKeyboard object (here from an KeyboardDevice object)
  boost::scoped_ptr<tiy::KeyboardDevice> keyboard_device;
  keyboard_device.reset(new tiy::LinuxKeyboard(do_debugging));

  // 2. Connect to a specific keyboard device and start reading it
  if (!keyboard_device->openAndReadKeyboard(keyboard_device_id, read_intervall_ms))
  {
	std::cout << "KeyboardDevice::openAndReadKeyboard() failed" << std::endl;
        std::cin.get();
	return 0;
  }
 
  std::cout << "Reading SPACE or ESC from keyboard (press ESC to exit) ..." << std::endl;

  while(true)
  {
      bool was_SPACE_pressed=false, was_ESC_pressed=false;

      // 3. Get the status of the keys
      keyboard_device->getStatusSinceLastReset(was_SPACE_pressed, was_ESC_pressed);
      // 4. Reset the status of the keys
      keyboard_device->resetStatus();

      if (was_SPACE_pressed)
          std::cout << "SPACE key was pressed." << std::endl;				

      if (was_ESC_pressed)
      {
          std::cout << "ESC key was pressed." << std::endl;
    	  std::cin.get();
    	  break;
      }

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

	LinuxKeyboard(bool& do_debugging_) : KeyboardDevice(do_debugging_);

	virtual ~LinuxKeyboard();

	virtual bool openAndReadKeyboard(std::string& event_file_name, int& read_intervall_ms);
```

# Methods #

---

**startRead()**
```
	virtual void startRead(int& read_intervall_ms);
```
Start reading the keyboard in an update loop (called by **openAndReadKeyboard()** in a new thread)
> _read`_`intervall`_`ms_: pause in milliseconds after each keyboard update (one loop pass)

---

**LinuxKeyboard()**
```
	LinuxKeyboard(bool& do_debugging_);
```
> _do`_`debugging_: set to true for debug output

---

**`~`LinuxKeyboard()**
```
	virtual ~LinuxKeyboard();
```

---

**openAndReadKeyboard()**
```
	virtual bool openAndReadKeyboard(std::string& event_file_name, int& read_intervall_ms) = 0;
```
Connect to the keyboard and start reading it (starts **startRead()** in a new thread)
> _event`_`file`_`name_: path to event file of the device (e.g. "/dev/input/event3")

> _read`_`intervall`_`ms_: pause in milliseconds after each keyboard update

---
