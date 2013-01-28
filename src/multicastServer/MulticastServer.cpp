//============================================================================
// Name        : MulticastServer.cpp
// Author      : Andreas Pflaum
// Licence	   : see LICENCE.txt
//============================================================================

#include "MulticastServer.h"

namespace tiy
{

MulticastServer::MulticastServer(boost::asio::io_service& io_service,
    const boost::asio::ip::address& multicast_address,
    int& multicast_port,
    bool do_debugging_)
  : endpoint_(multicast_address, multicast_port),
    socket_(io_service, endpoint_.protocol()),
    do_debugging(do_debugging_)
{
	{
	  boost::mutex::scoped_lock multicast_server_lock(multicast_server_mutex);
	  go_on = true;
	}

	if (do_debugging)
	{
		boost::mutex::scoped_lock my_io_lock(io_mutex);
		std::cout << "MulticastServer: STARTED" << std::endl;
	}
}


MulticastServer::~MulticastServer()
{
  {
	boost::mutex::scoped_lock multicast_server_lock(multicast_server_mutex);
	go_on = false;
  }
}

void
MulticastServer::sendString(std::string send_string)
{
	{
	  boost::mutex::scoped_lock multicast_server_lock(multicast_server_mutex);
	  if (!go_on)
		  return;
	}

	if (do_debugging)
	  {
		boost::mutex::scoped_lock io_lock(io_mutex);
		std::cout << "MulticastServer: SENDING \"" << send_string << "\""<< std::endl;
	  }

    socket_.async_send_to(
        boost::asio::buffer(send_string), endpoint_,
        boost::bind(&MulticastServer::handleSend, this,
          boost::asio::placeholders::error));
}


void
MulticastServer::handleSend(const boost::system::error_code& error)
{
  {
    boost::mutex::scoped_lock multicast_server_lock(multicast_server_mutex);
    if (!go_on)
    	return;
  }

  if (error)
  {
	  if (do_debugging)
		{
		  boost::mutex::scoped_lock io_lock(io_mutex);
		  std::cout << "MulticastServer: ERROR async_send_to() (error: " << error << " ("<< boost::system::system_error(error).what() << "))" << std::endl;
		}

	  {
		boost::mutex::scoped_lock multicast_server_lock(multicast_server_mutex);
		go_on = false;
	  }
  }
}

}