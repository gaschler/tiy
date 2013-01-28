//============================================================================
// Name        : WindowsMouse.cpp
// Author      : Andreas Pflaum
// Licence	   : see LICENCE.txt
//============================================================================

#include "WindowsMouse.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <setupapi.h>
#include <cfgmgr32.h>

#pragma comment (lib, "setupapi.lib")


// HELPER FUNCTIONS:

// Check if a usb device with the given vid and pid is available, else list available usb device hardware ids
bool find_usb_device(std::string vid_, std::string pid_);

// Search for the strings "VID_XXX" and "PID_XXX" in the given hardware id (hid). If found set the vid and pid (each length 3)
bool extract_vid_and_pid(std::string hardware_id_str_, std::string& vid, std::string& pid);

namespace tiy
{

WindowsMouse::~WindowsMouse()
{
	{
		boost::mutex::scoped_lock mouse_lock(mouse_mutex);

		if (is_open)
			interception_destroy_context(context);

		is_open = false;
	}
}

bool
WindowsMouse::openAndReadMouse(std::string& hardware_id_, int& read_intervall_us)
{
	device_hardware_id_str = hardware_id_;

	if (!extract_vid_and_pid(device_hardware_id_str, device_vid, device_pid))
	{
		std::cerr << "WindowsMouse: extract_vid_and_pid(\"" << device_hardware_id_str << "\") failed " << std::endl
				  << "(The Hardware ID can be get from the the Windows \"Device Manager\" by right clicking on the device"<< std::endl
				  << "	-> \"properties\" -> \"details\" tab -> choose the \"property\": \"Hardware-IDs\"" << std::endl
				  << "  -> take one of the hids containing the strings \"VID_xxx\" and \"PID_xxx\")" << std::endl;
		return false;
	}

	if (!find_usb_device(device_vid, device_pid))
	{
		std::cerr << "WindowsMouse: find_usb_device(\"" << device_vid << ", " << device_pid << "\") failed " << std::endl
				  << "(The Hardware ID can be get from the the Windows \"Device Manager\" by right clicking on the device"<< std::endl
				  << "	-> \"properties\" -> \"details\" tab -> choose the \"property\": \"Hardware-IDs\"" << std::endl
				  << "  -> take one of the hids containing the strings \"VID_xxx\" and \"PID_xxx\")" << std::endl;
		return false;
	}

	raise_process_priority();	

    context = interception_create_context();

    interception_set_filter(context, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_ALL);

	{
		boost::mutex::scoped_lock mouse_lock(mouse_mutex);
		is_open = true;
	}

	boost::thread read_thread(boost::bind(&WindowsMouse::startRead, this, read_intervall_us));

	return true;
}

void
WindowsMouse::startRead(int& read_intervall_us)
{
	bool is_left_button_pressed_buffer = false;
	bool is_right_button_pressed_buffer = false;

	bool is_other_mouse = true;

	while(interception_receive(context, device = interception_wait(context), &stroke, 1) > 0)
	{
		{
			boost::mutex::scoped_lock mouse_lock(mouse_mutex);

			if (!is_open)
				return;
		}

		if(interception_is_mouse(device))
		{			
			size_t length = interception_get_hardware_id(context, device, hardware_id, sizeof(hardware_id));

			if(length > 0 && length < sizeof(hardware_id))
			{
				std::wstring hardware_id_wstr( hardware_id );
				std::string hardware_id_str( hardware_id_wstr.begin(), hardware_id_wstr.end() );

				std::string vid, pid;
				if (extract_vid_and_pid(hardware_id_str, vid, pid) && (vid == device_vid) && (pid == device_pid))
				{					
					int mouse_wheel_position_buffer;
					{
						boost::mutex::scoped_lock mouse_lock(mouse_mutex);

						mouse_wheel_position_buffer = mouse_wheel_position;
					}

					bool was_left_button_pressed_buffer = false;
					bool was_left_button_released_buffer = false;
					bool was_right_button_pressed_buffer = false;
					bool was_right_button_released_buffer = false;
					bool has_mouse_wheel_changed_buffer = false;

					{
						boost::mutex::scoped_lock mouse_lock(mouse_mutex);

						is_right_button_pressed = is_right_button_pressed_buffer;
						is_left_button_pressed = is_left_button_pressed_buffer;
					}

					InterceptionMouseStroke &mousestroke = *(InterceptionMouseStroke *) &stroke;

					switch(mousestroke.state)
					{							
						case INTERCEPTION_MOUSE_LEFT_BUTTON_DOWN:
							was_left_button_pressed_buffer = true;
							is_left_button_pressed_buffer = true;
							break;
						case INTERCEPTION_MOUSE_LEFT_BUTTON_UP:
							was_left_button_released_buffer = true;
							is_left_button_pressed_buffer = false;
							break;
						case INTERCEPTION_MOUSE_RIGHT_BUTTON_DOWN:
							was_right_button_pressed_buffer = true;
							is_right_button_pressed_buffer = true;
							break;
						case INTERCEPTION_MOUSE_RIGHT_BUTTON_UP:
							was_right_button_released_buffer = true;
							is_right_button_pressed_buffer = false;
							break;
						case INTERCEPTION_MOUSE_WHEEL:	
							has_mouse_wheel_changed_buffer = true;
							(mousestroke.rolling > 0) ? mouse_wheel_position_buffer++ : mouse_wheel_position_buffer--;
							break;
					}

					updateStatus(was_left_button_pressed_buffer, was_left_button_released_buffer, is_left_button_pressed_buffer,
									was_right_button_pressed_buffer, was_right_button_released_buffer, is_right_button_pressed_buffer,
										has_mouse_wheel_changed_buffer, mouse_wheel_position_buffer);

					// Do NOT "free" intercepted events from this mouse for other applications/OS
					continue; 
				}
			}
		}
       
		// "Free" intercepted events of other usb/hid devices (for other applications/OS)
		interception_send(context, device, &stroke, 1);
	}
}

}


// Needed for find_usb_device()
typedef BOOL (WINAPI *FN_SetupDiGetDeviceProperty)(
  __in       HDEVINFO DeviceInfoSet,
  __in       PSP_DEVINFO_DATA DeviceInfoData,
  __in       const DEVPROPKEY *PropertyKey,
  __out      DEVPROPTYPE *PropertyType,
  __out_opt  PBYTE PropertyBuffer,
  __in       DWORD PropertyBufferSize,
  __out_opt  PDWORD RequiredSize,
  __in       DWORD Flags
);

bool find_usb_device(std::string vid_, std::string pid_)
{
#ifdef UNICODE
	FN_SetupDiGetDeviceProperty fn_SetupDiGetDeviceProperty = (FN_SetupDiGetDeviceProperty)
		GetProcAddress (GetModuleHandle (TEXT("Setupapi.dll")), "SetupDiGetDevicePropertyW");
#else
	FN_SetupDiGetDeviceProperty fn_SetupDiGetDeviceProperty = (FN_SetupDiGetDeviceProperty)
		GetProcAddress(GetModuleHandle(TEXT("Setupapi.dll")), "SetupDiGetDevicePropertyA");
#endif

    // Get all connected USB devices
    HDEVINFO hDevInfo = SetupDiGetClassDevs (NULL, TEXT("USB"), NULL, DIGCF_PRESENT|DIGCF_ALLCLASSES);
    if (hDevInfo == INVALID_HANDLE_VALUE)
	{
		std::cerr << "Invalid handle value" << std::endl;
        return false;
	}

	CONFIGRET r;
	SP_DEVINFO_DATA DeviceInfoData;
	TCHAR szDeviceInstanceID [MAX_DEVICE_ID_LEN];
	std::vector<std::string> hardware_id_str;
	bool found_device = false;

	// Find the ones that are driverless
    for (unsigned int i = 0; ; i++)  {
        DeviceInfoData.cbSize = sizeof (DeviceInfoData);
        if (!SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData))
            break;

        r = CM_Get_Device_ID(DeviceInfoData.DevInst, szDeviceInstanceID , MAX_PATH, 0);
        if (r != CR_SUCCESS)
            continue;

#ifdef UNICODE
		size_t size = wcslen(szDeviceInstanceID);
		char * buffer = new char [2*size+2];
		wcstombs(buffer,szDeviceInstanceID,2*size+2);
		hardware_id_str.push_back(buffer);
		delete [] buffer;
#else
		hardware_id_str.push_back(szDeviceInstanceID);
#endif

		std::string vid, pid;
		if (extract_vid_and_pid(hardware_id_str[hardware_id_str.size()-1], vid, pid) && (vid == vid_) && (pid == pid_))
				found_device = true;
    }

	if (!found_device)
	{
		std::cout << "Could not find a USB device with a hardware id containing \"VID" << vid_ << "\" and \"PID_" << pid_ << "\", available USB devices are: " << std::endl;
		for (unsigned int i=0; i < hardware_id_str.size(); i++)
			std::cout << "	" << hardware_id_str[i] << std::endl;
		return false;
	}

    return true;
}

bool extract_vid_and_pid(std::string hardware_id_str_, std::string& vid_, std::string& pid_)
{
	int found = hardware_id_str_.find("VID_");
	if ((found==std::string::npos) || ((int)hardware_id_str_.size()<= found+7))
	{
		vid_ = "";
		pid_ = "";
		return false;
	}
	vid_ = hardware_id_str_.substr(found+4,4);

	found = hardware_id_str_.find("PID_");
	if ((found==std::string::npos) || ((int)hardware_id_str_.size()<=found+7))
	{
		vid_ = "";
		pid_ = "";
		return false;
	}
	pid_ = hardware_id_str_.substr(found+4,4);

	return true;
}