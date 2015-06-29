# 1. TIY software installation #
Before installing the hardware, you should start with installing the software and getting familiar with it.

Go to [InstallationOnWindows](http://code.google.com/p/tiy/wiki/InstallationOnWindows) to see how to install the TIY library **and examples** on Windows, or [InstallationOnLinux](http://code.google.com/p/tiy/wiki/InstallationOnLinux) for Linux systems respectively.

# 2. Software tests #
To get a first impression of the software and check if everything works fine, you should start the _client_ and _server_ program and play with the _config`_`run`_`parameters.xml_ configuration file.

On Linux you have to change the _<keyboard`_`device`_`id>_ field in the run configuration file to define your keyboard before starting the _/usr/local/tiy/client_ and _/usr/local/tiy/server_ programs (see therefore _/user/include/tiy/inputDevice/unix/LinuxDevice`_`README.txt_).

On Windows systems you can start _C:/tiy/bin/tiy/client.exe_ and _C:/tiy/bin/tiy/server.exe_ without any preconfigurations.

![http://tiy.googlecode.com/svn/trunk/pics/server_screenshot.jpg](http://tiy.googlecode.com/svn/trunk/pics/server_screenshot.jpg)

Two windows (like that above) with recorded stereo videos should show up and data send from the server to the client. Detected 3D points are marked by a small black circle in the middle of the white shining marker balls and detected objects by a big circle in the middle of the first marker of every object.

The example programs have different features that can be configured by editing the _config`_`run`_`parameters.xml_ file located in the same directory as the programs. As the options are self-explaining or commented, you can start trying some changes.

# 3. Camera setup and configuration #
The next step is to set up the camera system and configure it. All steps are explained in the [CameraSetUp](http://code.google.com/p/tiy/wiki/CameraSetUp) section.

If everything worked well, you can put markers in the sight of your cameras and start the _server_ program. Now you should see small black circles in the middle of the white shining markers detected as 3D points.

# 4. Marker object setup and configuration #
After setting up the tracking system, attach markers to any objects that need to be tracked and configure them afterwards.

Both steps are explained in the [ObjectSetUp](http://code.google.com/p/tiy/wiki/ObjectSetUp) section.

# Integrate the software in your own code #
If all the steps above have been done, the source code of the _client_ program can be downloaded from the [Downloads](http://code.google.com/p/tiy/downloads/list) section and integrated in any other software.