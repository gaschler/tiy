//============================================================================
// Name        : MulticastClient.cpp
// Author      : Andreas Pflaum
// Licence	   : see LICENCE.txt
//============================================================================

#include "MulticastClient.h"

namespace tiy
{

MulticastClient::MulticastClient(boost::asio::io_service& io_service,
	      const boost::asio::ip::address& listen_address,
	      const boost::asio::ip::address& multicast_address,
	      short& multicast_port,
	      bool do_debugging_)
    : socket_(io_service),
      do_debugging(do_debugging_)
  {
	{
		boost::mutex::scoped_lock multicast_client_lock(multicast_client_mutex);
		is_new_data = false;
		go_on = true;
	}

		// Create the socket so that multiple may be bound to the same address.
		boost::asio::ip::udp::endpoint listen_endpoint(
			listen_address, multicast_port);
		socket_.open(listen_endpoint.protocol());
		socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
		socket_.bind(listen_endpoint);

		// Join the multicast group.
		socket_.set_option(
			boost::asio::ip::multicast::join_group(multicast_address));

		socket_.async_receive_from(
			boost::asio::buffer(data_, max_length), sender_endpoint_,
			boost::bind(&MulticastClient::handle_receive_from, this,
			  boost::asio::placeholders::error,
			  boost::asio::placeholders::bytes_transferred));
  }


void
MulticastClient::handle_receive_from(const boost::system::error_code& error,
    size_t bytes_recvd)
{
  if (!error)
  {
    std::string data_string_buffer;
	{
		boost::mutex::scoped_lock multicast_client_lock(multicast_client_mutex);

		if (!go_on)
			return;

		is_new_data = true;
		data_string = std::string(data_, bytes_recvd);
		data_string_buffer = data_string;
	}

	if (do_debugging)
	{
		boost::mutex::scoped_lock my_io_lock(io_mutex);
		std::cout << "MulticastClient: Received string \"" << data_string_buffer << "\"" << std::endl;
	}

    socket_.async_receive_from(
        boost::asio::buffer(data_, max_length), sender_endpoint_,
        boost::bind(&MulticastClient::handle_receive_from, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }
  else
  {
	  {
		boost::mutex::scoped_lock my_io_lock(io_mutex);

		std::cout << "MulticastClient: ERROR async_receive_from() (error: " << error << " ("<< boost::system::system_error(error).what() << "))" << std::endl;
		
		if (error.value() == 234)
		{
			std::cout << "		Maximal char buffer size (max_length = " << max_length << ", defined in MulticastClient.h)" << std::endl
					  << "		too small for the received data string." << std::endl;
		}
	  }
  }
}

bool
MulticastClient::getReceivedString(std::string& data_string_)
  {
	{
		boost::mutex::scoped_lock multicast_client_lock(multicast_client_mutex);
		
		if (is_new_data)
		{
			data_string_ = data_string;
			is_new_data = false;
			return true;
		}
		else
		{
			data_string_ = "";
			return false;
		}		
	}
  }

void
MulticastClient::stopReceiving()
  {
	{
		boost::mutex::scoped_lock multicast_client_lock(multicast_client_mutex);
		
		go_on = false;
	}
  }

}
