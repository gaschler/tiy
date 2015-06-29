The WindowsKeyboard class is the child class of [KeyboardDevice](http://code.google.com/p/tiy/wiki/ClassKeyboardDevice) for low level keyboard reading in Windows.

It is based on [intercepting](http://oblita.com/Interception.html) keystrokes of ALL connected keyboards. This is done in a while loop started in a separate thread.

# Usage #

  * The keyboard devices are NOT blocked for other applications/OS
  * Only SPACE and ESC keys are implemented (but easily extendible in the code)
  * Only ONE key press-event is buffered per key since last reset
  * Users have to reset the key event variables manually

## Example ##

See [IncludeLibrary](http://code.google.com/p/tiy/wiki/IncludeLibrary) on how to include the TIY library in your own code (e.g. this example).

```
#include <tiy.h>

int main(int argc, char* argv[])
{
  std::string keyboard_device_id = ""; //Not used
  int read_intervall_ms = 1; //Not used
  bool do_debugging = false;

  // 1. Create a WindowsKeyboard object (here from a KeyboardDevice object)
  boost::scoped_ptr<tiy::KeyboardDevice> keyboard_device;
  keyboard_device.reset(new tiy::WindowsKeyboard(do_debugging));

  // 2. Connect to a all keyboard devices and start reading them
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

        InterceptionContext context;
        InterceptionDevice device;
        InterceptionStroke stroke;

private:
	virtual void startRead(int& read_intervall_us);

public:

	WindowsKeyboard(bool& do_debugging_) : KeyboardDevice(do_debugging_) {};

	virtual ~WindowsKeyboard();

	virtual bool openAndReadKeyboard(std::string& hardware_id_, int& read_intervall_ms);
```

Outside the class:
```
enum ScanCode
{
    SCANCODE_ESC = 0x01,
    SCANCODE_SPACE = 0x5e,
    SCANCODE_SPACE_2 = 0x39
};
```
Standard key codes of the specific keys (here ESC and SPACE).

# Methods #

---

**startRead()**
```
	virtual void startRead(int& read_intervall_ms);
```
Start reading the keyboard in an update loop (called by **openAndReadKeyboard()** in a new thread)
> _read`_`intervall`_`ms_: NOT used

---

**WindowsKeyboard()**
```
	WindowsKeyboard(bool& do_debugging_);
```
> _do`_`debugging_: set to true for debug output

---

**`~`WindowsKeyboard()**
```
	virtual ~WindowsKeyboard();
```

---

**openAndReadKeyboard()**
```
	virtual bool openAndReadKeyboard(std::string& event_file_name, int& read_intervall_ms) = 0;
```
Connect to the keyboard and start reading it (starts **startRead()** in a new thread)
> _hardware`_`id_: NOT used

> _read`_`intervall`_`ms_: NOT used

---
