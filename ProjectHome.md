![http://tiy.googlecode.com/svn/trunk/pics/TIY_screenshot.jpg](http://tiy.googlecode.com/svn/trunk/pics/TIY_screenshot.jpg)

The TIY library ([Downloads](http://code.google.com/p/tiy/downloads/list)) enables you to easily build your own 3D marker (object) tracking system.

  * **Open-source** (with documentation)
  * **Windows** and **Linux** supported
  * **Multiple objects** can be detected simultaneously.
  * **No expensive** vision system required
  * **Ready-to-use examples** (for first testing, NO hardware needed)
  * **Tutorial** for software and hardware (see TrackItYourself)
  * **Precision** of about **1 mm** (setup with two 1296 x 966 GigE cameras)
  * Software **cycle time** (of the given full example tracking software) of **~10ms** (with a common setup)

# What is 3D marker (object) tracking? #
Markers are at most reflecting or shining little balls attached to an object that needs to be tracked (followed/find). If at least two cameras can see one marker at the same time, its 3D position relative to the fixed cameras can be computed by "triangulation", as the cameras see the markers from different positions and angles.

To find a specific object and detect its **3D pose** (position and orientation relative to the camera), multiple markers need to be attached to it and their arrangement saved as a template once. At runtime, the software compares all the actual detected markers (3D points) at every time instance with the arrangement of the 3D points saved in the template before. By comparison, the actual object pose can be computed.

# What can I do with the TIY library (and example)? #
After camera calibration ([CameraSetUp](http://code.google.com/p/tiy/wiki/CameraSetUp)), any objects can be calibrated ([ObjectSetUp](http://code.google.com/p/tiy/wiki/ObjectSetUp)) and their 3D poses tracked by the system.

Therefore the system can, for instance, be used in form of a **3D input device**. It is already used in an application of controlling a robot and interacting with a 2D user interface projected on a tracked surface.

Some features of TIY, used in the available _tiy`_`client_ and _tiy`_`server_ examples are:
  * OpenCV and Aravis GigE (Linux) camera support
  * Send data over network to multiple other computers
  * Log data, capture image frames and record videos
  * Use recorded videos or 2D data as test-input source (instead of cameras)
  * Keyboard- and mouse-interaction

# What hardware do I need to build such a system? #
For first TIY software tests, NO additional hardware ist needed.

For building an own tracking system, the only necessary hardware components are **two cameras** (infrared capable) connected to the computer and an **object to track** (with reflecting marker balls mounted on it).

A complete soft- and hardware tutorial in 4 steps is given on the [TrackItYourself](http://code.google.com/p/tiy/wiki/TrackItYourself) Wiki page.

# Any Questions? #
Subscribe to the mailing list _tiy@freelists.org_ ( http://www.freelists.org/list/tiy ).

## How can I cite TIY in a scientific paper? ##
Thank you! We highly appreciate it if you can cite our paper where we use TIY in a intuitive robot workcell:

Andre Gaschler, Maximilian Springer, Markus Rickert, and Alois Knoll. _[Intuitive robot tasks with augmented reality and virtual obstacles](http://www6.in.tum.de/Main/Publications/Gaschler2014b.pdf)_. In IEEE International Conference on Robotics and Automation (ICRA), June 2014.
```
@inproceedings{Gaschler2014b,
	author = {Andre Gaschler and Maximilian Springer and Markus Rickert and Alois Knoll},
	booktitle = {IEEE International Conference on Robotics and Automation (ICRA)},
	title = {Intuitive Robot Tasks with Augmented Reality and Virtual Obstacles},
	year = {2014}, month = {June}
}
```