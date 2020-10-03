
/* ====================================================================
* Functionally Encrypted Datatores - Implementation for project eprint.iacr.org/2019/1262
* Copyright (C) 2019  Rachit Garg Nishant Kumar

* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <https://www.gnu.org/licenses/>.

* ====================================================================
*
*/
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
