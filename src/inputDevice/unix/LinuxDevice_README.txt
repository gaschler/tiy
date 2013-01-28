HowTo add udev rule for accessing always the correct event-file for one specific input device

This example was used for the "Genius 2.4G Wireless Mouse"

1. 	check vendor and product id of the device

		$ lsusb
		
		ONE OUTPUT LINE: "Bus 004 Device 005: ID 0458:00c5 KYE Systems Corp. (Mouse Systems)" 
			=> idVendor == 0458, idProduct == 00c5
		
	(perhaps plug the device in/out in between to know, which one is the correct)
	
2. 	add and set udev rule (here for example new rule called "66-genius-mouse-usb.rules")

		$ sudo gedit /etc/udev/rules.d/66-genius-mouse-usb.rules
		
		WRITE:	#Genius 2.4G Wireless Mouse
				KERNEL=="event*",SUBSYSTEM=="input",ATTRS{idVendor}=="0458",ATTRS{idProduct}=="00c5",MODE="0644"
				KERNEL=="event*",SUBSYSTEM=="input",ATTRS{idVendor}=="0458",ATTRS{idProduct}=="00c5",SYMLINK+="input/genius-event"
				
3. 	SAVE and RESTART to activate rule!
