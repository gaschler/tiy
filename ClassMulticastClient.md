With the MulticastClient class a UDP multicast client can be created,  waiting asynchronously to receive strings from a multicast server (e.g. [MulticastServer](http://code.google.com/p/tiy/wiki/ClassMulticastServer)).

It is based on a cross-platform asio library [example](http://www.boost.org/doc/libs/1_41_0/doc/html/boost_asio/example/multicast/receiver.cpp).

# Usage #

  * The client can stay open before and after the server was started
  * Multiple clients can run simultaneously (multiple threads/objects)
  * Only the latest received data string is stored

## Example ##

See [IncludeLibrary](http://code.google.com/p/tiy/wiki/IncludeLibrary) on how to include the TIY library in your own code (e.g. this example).

```
#include <tiy.h>

int main(int argc, char* argv[])
{
  try
  {
        int client_update_intervall_ms=1;

	short multicast_port_short=30000;
	std::string multicast_adress = "239.255.0.1";
        std::string listen_address = "0.0.0.0";
	
	bool do_debugging = false;
	
	// 1. Create and start a multicast client
        boost::asio::io_service io_service;
	
        tiy::MulticastClient multicast_client(io_service,
               boost::asio::ip::address::from_string(listen_address),
                boost::asio::ip::address::from_string(multicast_adress),
                 multicast_port_short,
                  do_debugging);
		
	// 2. Start nonblocking service
        boost::system::error_code error_c;
	boost::thread io_service_thread(boost::bind(&boost::asio::io_service::run, &io_service, error_c));

	// 3. Run Update Loop
	std::string data_string;

	for(int i = 0; true; i++)
	{			
		if (multicast_client.getReceivedString(data_string))
		{
			std::cout << "Received string = " << data_string << std::endl;			
		}		
		boost::this_thread::sleep(boost::posix_time::milliseconds(client_update_intervall_ms)); 
	}
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
  }

  return 0;
}

```

# Declaration #

```
public:
  MulticastClient(boost::asio::io_service& io_service,
	           const boost::asio::ip::address& listen_address,
	            const boost::asio::ip::address& multicast_address,
	             short& multicast_port,
	              bool do_debugging_);

  bool getReceivedString(std::string& data_string_);

  void stopReceiving();

private:
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
```

# Methods #

---

**MulticastClient()**
```
	MulticastClient(boost::asio::io_service& io_service,
	                 const boost::asio::ip::address& listen_address,
	                  const boost::asio::ip::address& multicast_address,
	                   short& multicast_port,
	                    bool do_debugging_);
```
Creates and opens an UDP socket with the given network parameters. Starts receiving (waiting for data) in a new thread and returns.

> _io`_`service_: boost asio io\_service

> _listen`_`address_: endpoint address used by the client (e.g. "0.0.0.0", IP6: "0::0")

> _multicast`_`address_: socket join group address used by the client and server (e.g. "239.255.0.1", IP6: "ff31::8000:1234")

> _multicast`_`port_: port used by the client and server (e.g. 30000, short int, < 32768)

> _do`_`debugging`_`_: set to true to get debug output

---

**getReceivedString()**
```
	 bool getReceivedString(std::string& data_string_);
```
If _true_ returned, the newest received data string is stored in _data`_`string_ and the internal status is set to "no new data". Otherwise, an _false_ and an empty string is returned.

> _data`_`string_: contains the latest received string, that has not been get yet (if no new data, it is set to an empty string)

---

**stopReceiving()**
```
	  void stopReceiving();
```
Calls the client to stop.

---

**handle\_receive\_from()**
```
	 void handle_receive_from(const boost::system::error_code& error, size_t bytes_recvd);
```
Handles the received data (copy into a string) or errors and starts waiting for new data (in a new thread).

> _error_: contains the information about an error, if occurred

> _bytes`_`recvd_: contains the information on how many bytes received

---
