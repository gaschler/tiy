With the MulticastServer class a UDP multicast server can be created,  to asynchronously send a string to all "connected" multicast clients (e.g. [MulticastClient](http://code.google.com/p/tiy/wiki/ClassMulticastClient)).

It is based on a cross-platform asio library [example](http://www.boost.org/doc/libs/1_41_0/doc/html/boost_asio/example/multicast/sender.cpp).

# Usage #

  * The client can stay open before and after the server was started
  * The server does not need a client to run
  * Multiple clients can run simultaneously

## Example ##

See [IncludeLibrary](http://code.google.com/p/tiy/wiki/IncludeLibrary) on how to include the TIY library in your own code (e.g. this example).

```
#include <tiy.h>

int main(int argc, char* argv[])
{
  int sending_intervall_ms=50;

  short multicast_port_short=30000;
  std::string multicast_adress = "239.255.0.1";
	
  bool do_debugging = false;

  // 1. Create and start a multicast server
  boost::asio::io_service server_io_service;

  tiy::MulticastServer multicast_server(server_io_service,
                                         boost::asio::ip::address::from_string(multicast_adress),
                                          multicast_port, do_debugging);

  boost::system::error_code error_c;
  boost::thread server_io_service_thread(boost::bind(&boost::asio::io_service::run, &server_io_service, error_c));

  std::string send_string;

  for(int i = 0; true; i++)
  {	
      send_string = (boost::format("%d") % i).str() ;
		
      std::cout << "Sending: " << send_string << std::endl;	

      // 2. Send data (string)
      multicast_server.sendString(send_string);
      
      boost::this_thread::sleep(boost::posix_time::milliseconds(sending_intervall_ms)); 
  }

  return 0;
}

```

# Declaration #

```
public:
	MulticastServer(boost::asio::io_service& io_service,
	                 const boost::asio::ip::address& multicast_address,
	                  int& multicast_port,
	                   bool do_debugging_);

	~MulticastServer();

	void sendString(std::string send_string);

private:
	void handleSend(const boost::system::error_code& error);

private:
	boost::asio::ip::udp::endpoint endpoint_;
	boost::asio::ip::udp::socket socket_;

	bool do_debugging, go_on;

	boost::mutex multicast_server_mutex, io_mutex;
```

# Methods #

---

**MulticastServer()**
```
	MulticastServer(boost::asio::io_service& io_service,
	                 const boost::asio::ip::address& multicast_address,
	                  int& multicast_port,
	                   bool do_debugging_);
```
Creates and opens an UDP socket with the given network parameters. Starts receiving (waiting for data) in a new thread and returns.

> _io`_`service_: boost asio io\_service

> _multicast`_`address_: socket join group address used by the client and server (e.g. "239.255.0.1", IP6: "ff31::8000:1234")

> _multicast`_`port_: port used by the client and server (e.g. 30000, short int, < 32768)

> _do`_`debugging`_`_: set to true to get debug output

---

**~MulticastServer()**
```
	~MulticastServer();
```

---

**sendString()**
```
	 void sendString(std::string send_string);
```
Sends the string _send`_`string with_async`_`send`_`to()_to all "connected" clients. Calls **handleSend()** in a new thread._

> _send`_`string_: string that should be send

---

**stopReceiving()**
```
	  void stopReceiving();
```
Calls the client to stop.

---

**handleSend()**
```
	 void handleSend(const boost::system::error_code& error);
```
Checks if errors occur during the sending.

> _error_: contains information about a (potentially) error

---
