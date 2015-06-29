# What hardware components should I use? #
In the following, a small selection of components is listed.

## Two infrared capable cameras ##

If not a very wide setup of more than a few meters is used or not very high precission is needed, cheap "all-in-on" USB infrared webcams can be used such as the [Sabrent WCM-6LNV](http://www.amazon.com/Sabrent-Night-Vision-Webcam-WCM-6LNV/dp/B000BEZVEA/ref=sr_1_1?ie=UTF8&qid=1359396564&sr=8-1&keywords=infrared+webcam) or [Genius ISlim 321R](http://www.geniusnet.com/wSite/ct?xItem=16756&ctNode=160). Then you can skip the following parts until "Set up the cameras for TIY".

### Gigabit Ethernet cameras (Linux) ###
GigE cameras working with the used Aravis library on linux can be seen here: https://live.gnome.org/AravisTestedCameras.

They have high resolutions, provide many different settings and have small distortions.

However they are expensive and additional hardware is needed (e.g. for 2 _Basler acA1300-30GC_ cameras):
  * 2 power over Ethernet (PoE) power supply (e.g. _PHIHONG POE20U-560(G)_)
  * 4 CAT6 gigabit Ethernet network cables (camera`<->`PoE and PoE`<->`pc)
  * 2 gigabit Ethernet network cards (e.g. _Intel PRO/1000 GT Desktop_)
  * 2 IR-pass filters (e.g. from _Baader_ or _Astronomik_)

![http://tiy.googlecode.com/svn/trunk/pics/baseler_camera_with_ir_leds.jpg](http://tiy.googlecode.com/svn/trunk/pics/baseler_camera_with_ir_leds.jpg)

### OpenCV cameras ###
Much cheaper and easier to use are USB (web-)cameras supported by OpenCV (small selection at http://opencv.willowgarage.com/wiki/Welcome/OS).

The more far away the two cameras are, the higher must be the resolution. The faster the objects need to be tracked, the higher must be the FPS rate.

Unfortunately most of the webcams have IR cut filters in it. To remove it, see http://www.instructables.com/id/Infrared-IR-Webcam/ or especially for the well-known Logitech QuickCam Pro 9000 http://www.ce.rit.edu/research/projects/2008_fall/Drowsy_Driver_Warning_System/tutorials/tutorial_ir.html.

To filter out the visible range of light, but see the infrared light, an IR-pass filter is needed. In the first tutorial it is already mentioned to put an IR-pass filter created from an unexposed (black) processed film in the camera after removing the IR cut filter. Another tutorial to build an IR-pass filter by yourself can be seen at http://www.diyphotography.net/take_infrared_pictures_with_digital_camera_ir_filter.

Alternatively, there are web pages where you can buy adapters for webcams, where you then can put IR-pass filters from e.g. _Baader_ or _Astronomik_ in it. One page with a list of supported webcams is e.g. http://webcaddy.com.au/astro/M12Cameras.htm.

## Infrared light ##
Besides the two cameras with IR-pass filters you also need infrared lights. It is very important that they are attached close to the cameras and shine in the same direction.

A common way is to buy LED IR-spotlights/illuminators (e.g. 4 _Kingbright BL0106-15-28_ with (preferably adjustable) power supply _ANSMANN APS 2250L_) and attach them to the cameras.

# Set up the cameras for TIY #
To use the real cameras in the TIY _server_ program instead of the recorded videos, change the option _<input`_`src>_ in the _config`_`run`_`parameters.xml_ configuration file to either _b_ to use the Aravis interface (e.g. for GigE Basler cameras) or to _o_ for using the OpenCV interface (e.g. for USB webcams).

When starting the _server_ program, an error should occur and all connected cameras listed. The appeared names/numbers need to be filled into the first two fields of the _config`_`camera.xml_ configuration file. In addition, the frame width, height and rate must be adapted. The exposure, gain and segmentation areas need to be tested and adjusted when testing the object tracking.

# Camera calibration #
To use the two cameras as one stereo camera, you need both the intrinsic and extrinsic camera parameters.

They can easily get from the MATLAB camera calibration toolbox (http://www.vision.caltech.edu/bouguetj/calib_doc/). The toolbox can also be used with OCTAVE (see http://www.sparetimelabs.com/cameracalib/index.html).

  1. Download and unzip the _tiy`_`calibration_ _.zip_ file from the [Downloads](http://code.google.com/p/tiy/downloads/list) section
  1. Download and unzip the toolbox from http://www.vision.caltech.edu/bouguetj/calib_doc/download/toolbox_calib.zip
  1. Put all the MAT-files from the extracted toolbox into the extracted _tiy`_`calibration/camera`_`calibration/toolbox\_calib_ folder
  1. Print out the _tiy`_`calibration/camera`_`calibration/pattern.pdf_ as large as possible and fix it completely flat onto an as inflexible as possible plate.
  1. Make at least 10-15 snapshots of the checkerboard pattern filling the screen in different angles like these http://www.vision.caltech.edu/bouguetj/calib_doc/htmls/calib_example/index.html for the left and then for the right camera.
> > (Stereo) snapshots can be taken by the TIY _server_ example when activating the _<do`_`log`_`frame>_ option in the _config`_`run`_`parameters.xml_ configuration file and pressing the SPACE key for each snapshot. Perhaps the gain and exposure need to be highly increased to see the chessboard. The files are stored in the _tiy`_`log_ folder in the TIY bin or home directory. Warning: snapshots are overwritten after restarting the _server_ program!
  1. Make one stereo snapshot of the checkboard pattern lying preferably flat on a defined reproducible position in the sight of both cameras.
  1. With this image data do the complete intrinsic and extrinsic calibration as explained here http://www.vision.caltech.edu/bouguetj/calib_doc/htmls/example.html.
> > After each intrinsic calibration, replace the _Calib`_`Results.m_ and _Calib`_`Results.mat_ files in the specific _tiy`_`calibration/camera`_`calibration/left_ or right folder. At the end of the extrinsic calibrations, replace the data in the _Calib`_`Results`_`extrinsic.m_ files.
  1. Replace the _config`_`camera.xml_ file in the _tiy`_`calibration/camera`_`calibration_ folder with your file from the TIY folder. Run the _make`_`camera`_`parameters.m_ script to write the calibration data into the _config`_`camera.xml_ file and copy the changed configuration file back to the TIY folder.
> > Important: If you use Octave instead of Matlab, the complete file is overwritten and all camera settings like frame rate, width, height,... are set to default (see _write`_`XML`_`octave`_`camera.m_ in the _fcn_ subfolder). Therefore backup your _config`_`camera.xml_ file before running the script to be able to change these setting back to your choice.