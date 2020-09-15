#include "tcpclient.h"

socket_ptr tcpclient::initialize(char* hostname, char* port) {

	tcp::resolver resolver(io_service);
    tcp::resolver::query query(hostname, port);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    tcp::resolver::iterator end;

    socket_ptr sock(new tcp::socket(io_service));
    boost::system::error_code error = boost::asio::error::host_not_found;
    
    SSEtime timeout;
    while(error) {
	    while (endpoint_iterator != end)
	    {
	      (*sock).close();
	      (*sock).connect(*endpoint_iterator++, error);
	    }
	    endpoint_iterator = resolver.resolve(query);
	    uint64_t timedone = timeout.getTimefromstart();
	    if(timedone >= 3600) break;
	    else if(timedone>=10) {
	    	sleep(10);
		}
	}

    if (error)
      throw boost::system::system_error(error);

    return sock;
  // tcp::resolver resolver(io_service);
  //   boost::asio::ip::tcp::resolver::query query(hostname, port);
  // tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

  // socket_ptr sock(new tcp::socket(io_service));
  // boost::asio::connect(*sock, endpoint_iterator);
  // return sock;
}
