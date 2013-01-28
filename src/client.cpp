//============================================================================
// Name        : client.cpp
// Author      : Andreas Pflaum
// Description : Multicast client program, using the MulticastClient class
//				 of the Track-It-Yourself (TIY) library (cross-platform).
//				 Checking for new data in a loop with an update interval of
//				 client_update_intervall_ms [ms] and displaying it.
//				 Connection parameters are read from a xml file without
//				 explicit XML parser.
// Licence	   : see LICENCE.txt
//============================================================================

#include "multicastClient/MulticastClient.h"

#include <fstream>

// HELPFER FUNCTION (=> no XML parser necessary):
// Get the value AS A STRING of the parameter "param_name" from the string "line" (xml format "<param_name>value</param_name>")
// (If the value is a string ("value"), the quotation marks will be cut off)
bool extract_param_from_xml_line(std::string& line, const std::string param_name, std::string& value_str);

int main(int argc, char* argv[])
{
  try
  {
	// -------------------------------------------------------------------------------------
	// Input ARG
	// -------------------------------------------------------------------------------------
	char *arg_run_parameter_config_file = (char *)"config_run_parameters.xml";

	if (argc == 1)
	{
		std::cerr << "USING DEFAULT CONFIG FILES: config_run_parameters.xml" << std::endl;
	}
	else if (argc!=1 && argc!=2)
	{
		std::cerr << "Usage: 	main <run_parameters_config_file>" << std::endl;
		std::cerr << "default:  main config_run_parameters.xml" << std::endl;
		std::cerr << "PRESS A KEY TO EXIT"; 
		std::cin.get();
		return 0;
	}
	else
		arg_run_parameter_config_file = argv[0];


	// -------------------------------------------------------------------------------------
	// Get Run Parameters from XML Config File (with helper function)
	// -------------------------------------------------------------------------------------
	int do_output_debug=-1, client_update_intervall_ms=-1;
	short multicast_port_short=-1;
	std::string multicast_adress, listen_address;

	std::ifstream xml_file(arg_run_parameter_config_file);
	std::string line;
	while(std::getline(xml_file, line))
	{
		std::string value_str;

		if (extract_param_from_xml_line(line, "multicast_port", value_str))
			multicast_port_short = (short)atoi(value_str.c_str());
		if (extract_param_from_xml_line(line, "do_output_debug", value_str))
			do_output_debug = atoi(value_str.c_str());
		if (extract_param_from_xml_line(line, "client_update_intervall_ms", value_str))
			client_update_intervall_ms = atoi(value_str.c_str());

		if (extract_param_from_xml_line(line, "multicast_adress", value_str))
			multicast_adress = value_str;
		if (extract_param_from_xml_line(line, "listen_address", value_str))
			listen_address = value_str;
	}

	if (multicast_port_short==-1 || do_output_debug==-1 || client_update_intervall_ms==-1 ||
		multicast_adress.empty() || listen_address.empty())
	{
		std::cerr << "Read all run parameters from " << arg_run_parameter_config_file << " failed" << std::endl;
		std::cerr << "PRESS A KEY TO EXIT"; std::cin.get();
		return 0;
	}
	
	bool do_debugging = (do_output_debug != 0);

	if (do_debugging)
	{
		std::cout << "multicast_port_short = " << multicast_port_short << std::endl
				  << "do_output_debug = " << do_output_debug << std::endl
				  << "client_update_intervall_ms = " << client_update_intervall_ms << std::endl
				  << "multicast_adress = " << multicast_adress << std::endl
				  << "listen_address = " << listen_address << std::endl;
	}

	
	// -------------------------------------------------------------------------------------
	// Start Client
	// -------------------------------------------------------------------------------------
    boost::asio::io_service io_service;
	
    tiy::MulticastClient multicast_client(io_service,
        boost::asio::ip::address::from_string(listen_address),
        boost::asio::ip::address::from_string(multicast_adress),
        multicast_port_short,
        do_debugging);
		
	// NONBLOCKING service
    boost::system::error_code error_c;
	boost::thread io_service_thread(boost::bind(&boost::asio::io_service::run, &io_service, error_c));


	// -------------------------------------------------------------------------------------
	// Run Update Loop
	// -------------------------------------------------------------------------------------
	std::string data_string;

	for(int i = 0; true; i++)
	{			
		if (multicast_client.getReceivedString(data_string))
		{
			  //////////////////////////////////////////////////
			 //////////////// DO WHAT YOU WANT ////////////////
			//////////////////////////////////////////////////
			std::cout << "Received: " << data_string << std::endl;			
		}
		
		boost::this_thread::sleep(boost::posix_time::milliseconds(client_update_intervall_ms)); 
	}
  }
  // -------------------------------------------------------------------------------------
  // Catch Exceptions
  // -------------------------------------------------------------------------------------
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
  }

  return 0;
}


bool extract_param_from_xml_line(std::string& line, const std::string param_name, std::string& value_str)
{
	value_str = "";

	// find first element of the start tag
	std::string start_tag = "<" + param_name + ">";
	std::size_t found = line.find(start_tag);
	if (found==std::string::npos)
		return false;

	// find last element of the start tag
	std::string aux = line.substr(found);
	found = aux.find(">");
	if (found==std::string::npos)
		return false;
	aux = aux.substr(found + 1);

	// find first element of the end tag
	std::string end_tag = "</" + param_name + ">";
	std::size_t end_found = aux.find(end_tag);
	if (end_found==std::string::npos)
		return false;

	// extract the string between the start tag and end tag
	value_str = aux.substr(0, end_found);
	//std::cout << "value_str = " << value_str << std::endl;

	// distinguish between "string" and number
	if (!value_str.empty() )
	{
		if ((value_str.size () >= 3) && (*value_str.begin()=='\"') && (*value_str.rbegin()=='\"'))
		{
			// cut of first and last character ("")
			value_str = value_str.substr(1,value_str.size()-1);
			value_str.resize(value_str.size()-1);
		}	

		return true;
	}

	std::cerr << "read_param_from_xml() - No value found for parameter \"" << param_name << "\"." << std::endl;
	return false;
}
