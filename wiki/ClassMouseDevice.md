The MouseDevice class is an abstract base class for low level mouse reading.

Child classes inheriting from MouseDevice are [LinuxMouse](http://code.google.com/p/tiy/wiki/ClassLinuxMouse) and [WindowsMouse](http://code.google.com/p/tiy/wiki/ClassWindowsMouse).

# Usage #

See child classes [LinuxMouse](http://code.google.com/p/tiy/wiki/ClassLinuxMouse) and [WindowsMouse](http://code.google.com/p/tiy/wiki/ClassWindowsMouse).

# Declaration #

```
protected:
	boost::mutex mouse_mutex;

	bool do_debugging;
	bool is_open;

	bool was_left_button_pressed, was_left_button_released, was_right_button_pressed, was_right_button_released, has_mouse_wheel_changed;

	bool is_left_button_pressed, is_right_button_pressed;
	int mouse_wheel_position;

protected:
	virtual void startRead(int& read_intervall_ms) = 0;

	void updateStatus(bool& was_left_button_pressed_, bool& was_left_button_released_, bool& is_left_button_pressed_,
			   bool& was_right_button_pressed_,bool& was_right_button_released_,bool& is_right_button_pressed_,
			    bool& has_mouse_wheel_changed_, int& mouse_wheel_position_);

public:
	MouseDevice(bool& do_debugging_);

	virtual ~MouseDevice() {};

	virtual bool openAndReadMouse(std::string& event_file_name, int& read_intervall_ms) = 0;

	void getStatusSinceLastReset(bool& was_left_button_pressed_, bool& was_left_button_released_, bool& is_left_button_pressed_,
				      bool& was_right_button_pressed_,bool& was_right_button_released_,bool& is_right_button_pressed_,
				       bool& has_mouse_wheel_changed_, int& mouse_wheel_position_);

	void resetStatus();
```

# Methods #

---

**startRead()**
```
	virtual void startRead(int& read_intervall_ms) = 0;
```
Start reading the mouse in an update loop (called by **openAndReadMouse()** in a new thread)
> _read`_`intervall`_`ms_: pause in milliseconds after each mouse update (one loop pass) (Unix)

---

**updateStatus()**
```
	void updateStatus(bool& was_left_button_pressed_, bool& was_left_button_released_, bool& is_left_button_pressed_,
			   bool& was_right_button_pressed_,bool& was_right_button_released_,bool& is_right_button_pressed_,
			    bool& has_mouse_wheel_changed_, int& mouse_wheel_position_);
```
Set the values of the mouse event variables (called by the update loop in **startRead()** and by **resetStatus()**).

---

**MouseDevice()**
```
	MouseDevice(bool& do_debugging_);
```
> _do`_`debugging_: set to true for debug output

---

**`~`MouseDevice()**
```
	virtual ~MouseDevice() {};
```

---

**openAndReadMouse()**
```
	virtual bool openAndReadMouse(std::string& event_file_name, int& read_intervall_ms) = 0;
```
Connect to the mouse and start reading it (starts **startRead()** in a new thread)
> _event`_`file`_`name_: hardware id (win) or event file (Unix) of the device

> _read`_`intervall`_`ms_: pause in milliseconds after each keyboard update (Unix)

---

**getStatusSinceLastReset()**
```
	void getStatusSinceLastReset(bool& was_left_button_pressed_, bool& was_left_button_released_, bool& is_left_button_pressed_,
				      bool& was_right_button_pressed_,bool& was_right_button_released_,bool& is_right_button_pressed_,
				       bool& has_mouse_wheel_changed_, int& mouse_wheel_position_);
```
Get the values of the mouse event variables.

---

**resetStatus()**
```
	void resetStatus();
```
Reset the values of the event variables (should usually be called directly after **getStatusSinceLastReset()**)

---
