//============================================================================
// Name        : MulticastClient.h
// Author      : Andreas Pflaum
// Description : Cross-platform class for setting up a boost udp multicast client
//				 Async waiting for new data from a multicast server. The newest
//				 received data string can be get by getReceivedString().
//				 Connection parameters:
//				 - listen_address (e.g. "0.0.0.0", IP6: "0::0")
//				 - multicast_address (e.g. "239.255.0.1", IP6: "ff31::8000:1234")
//				 - multicast_port (short int, < 32768)
// Licence	   : see LICENCE.txt
//============================================================================
//
// Code based on the boost asio "receiver.cpp":
//
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef MULTICAST_CLIENT_H_
#define MULTICAST_CLIENT_H_

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

class MulticastClient
{

public:

  // Constructor (NOT blocking, as handle_receive_from() called in a new thread)
  MulticastClient(boost::asio::io_service& io_service,
	      const boost::asio::ip::address& listen_address,
	      const boost::asio::ip::address& multicast_address,
	      short& multicast_port,
	      bool do_debugging_);

  // Get newest received data string and return TRUE if not already get before, else FALSE
  bool getReceivedString(std::string& data_string_);

  void stopReceiving();

private:

  // Called by async_receive_from() used in the constructor and itself
  void handle_receive_from(const boost::system::error_code& error,
      size_t bytes_recvd);

private:

  bool do_debugging, is_new_data, go_on;

  boost::asio::ip::udp::socket socket_;
  boost::asio::ip::udp::endpoint sender_endpoint_;
  
  enum { max_length = 512 };
  char data_[max_length];

  std::string data_string;
  
  boost::mutex multicast_client_mutex, io_mutex;

};

} 

#endif // MULTICAST_CLIENT_H_
