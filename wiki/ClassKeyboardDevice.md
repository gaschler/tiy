The KeyboardDevice class is an abstract base class for low level keyboard reading.

Child classes inheriting from KeyboardDevice are [LinuxKeyboard](http://code.google.com/p/tiy/wiki/ClassLinuxKeyboard) and [WindowsKeyboard](http://code.google.com/p/tiy/wiki/ClassWindowsKeyboard).

# Usage #

See child classes [LinuxKeyboard](http://code.google.com/p/tiy/wiki/ClassLinuxKeyboard) and [WindowsKeyboard](http://code.google.com/p/tiy/wiki/ClassWindowsKeyboard).

# Declaration #

```
protected:

	boost::mutex keyboard_mutex;

	bool do_debugging;
	bool is_open;

	bool was_SPACE_pressed, was_ESC_pressed;

protected:
	
	virtual void startRead(int& read_intervall_ms) = 0;

	void updateStatus(bool& was_SPACE_pressed_, bool& was_ESC_pressed_);

public:

	KeyboardDevice(bool& do_debugging_);

	virtual ~KeyboardDevice() {};

	virtual bool openAndReadKeyboard(std::string& event_file_name, int& read_intervall_ms) = 0;

	void getStatusSinceLastReset(bool& was_SPACE_pressed_, bool& was_ESC_pressed_);

	void resetStatus();
```

# Methods #

---

**startRead()**
```
	virtual void startRead(int& read_intervall_ms) = 0;
```
Start reading the keyboard in an update loop (called by **openAndReadKeyboard()** in a new thread)
> _read`_`intervall`_`ms_: pause in milliseconds after each keyboard update (one loop pass) (Unix)

---

**updateStatus()**
```
	void updateStatus(bool& was_SPACE_pressed_, bool& was_ESC_pressed_);
```
Set the values of the key event variables (called by the update loop in **startRead()** and by **resetStatus()**).

---

**KeyboardDevice()**
```
	KeyboardDevice(bool& do_debugging_);
```
> _do`_`debugging_: set to true for debug output

---

**`~`KeyboardDevice()**
```
	virtual ~KeyboardDevice() {};
```

---

**openAndReadKeyboard()**
```
	virtual bool openAndReadKeyboard(std::string& event_file_name, int& read_intervall_ms) = 0;
```
Connect to the keyboard and start reading it (starts **startRead()** in a new thread)
> _event`_`file`_`name_: event file (Unix) of the device

> _read`_`intervall`_`ms_: pause in milliseconds after each keyboard update (Unix)

---

**getStatusSinceLastReset()**
```
	void getStatusSinceLastReset(bool& was_SPACE_pressed_, bool& was_ESC_pressed_);
```
Get the values of the key event variables.

---

**resetStatus()**
```
	void resetStatus();
```
Reset the values of the event variables (should usually be called directly after **getStatusSinceLastReset()**)

---
