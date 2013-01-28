//============================================================================
// Name        : MulticastServer.h
// Author      : Andreas Pflaum
// Description : Cross-platform (boost asio) class for setting up a multicast 
//				 server, sending data to multicast clients. Async. UDP connection
//				 with:
//				 - multicast_address (e.g. "239.255.0.1", IP6: "ff31::8000:1234")
//				 - multicast_port (short int, < 32768)
// Licence	   : see LICENCE.txt
//============================================================================
//
// Code based on the boost asio "sender.cpp":
//
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef MULTICAST_SERVER_H_
#define MULTICAST_SERVER_H_

#ifndef _WIN32_WINNT
	#define _WIN32_WINNT 0x0501
#endif 

#define WIN32_LEAN_AND_MEAN
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <iostream>
#include <string>


namespace tiy
{

class MulticastServer
{

public:

	// Constructor (NOT blocking)
	MulticastServer(boost::asio::io_service& io_service,
	      const boost::asio::ip::address& multicast_address,
	      int& multicast_port,
	      bool do_debugging_);

	~MulticastServer();

	// Send the string send_string with async_send_to() (calls handleSend() in a new thread)
	void sendString(std::string send_string);

private:

	// Called by async_send_to() used in sendString()
	void handleSend(const boost::system::error_code& error);

private:

	boost::asio::ip::udp::endpoint endpoint_;
	boost::asio::ip::udp::socket socket_;

	bool do_debugging, go_on;

	boost::mutex multicast_server_mutex, io_mutex;

};

}

#endif // MULTICAST_SERVER_H_
