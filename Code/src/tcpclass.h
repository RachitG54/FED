
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
#ifndef tcpclass_H
#define tcpclass_H

#include "SSEincludes.h"
#include "cliquery.h"
#include "authtok.h"
#include "SSEtime.h"

using boost::asio::ip::tcp;

typedef boost::shared_ptr<tcp::socket> socket_ptr;

class tcpclass {
  protected:
  	uint64_t totalBytesRead = 0;
	uint64_t totalBytesWritten = 0;
	uint64_t totalTimeSpentInMicroSec = 0;
	uint64_t currBytesRead = 0;
	uint64_t currBytesWritten = 0;
	
  public:
    boost::asio::io_service io_service;
    void make_string(boost::asio::streambuf& streambuf, string &message, size_t &len);
    long long readlenbytes(socket_ptr sock);
    void readstructure(socket_ptr sock, string &message);
    void writewithsize(socket_ptr sock, string &message);
    void writemessage(socket_ptr sock, string message);

	uint64_t getTotalBytesRead(){
		return totalBytesRead;
	}

	uint64_t getTotalBytesWritten(){
		return totalBytesWritten;
	}

	uint64_t getAndResetTotalBytesRead(){
		uint64_t temp = totalBytesRead;
		totalBytesRead = 0;
		return temp;
	}

	uint64_t getAndResetTotalBytesWritten(){
		uint64_t temp = totalBytesWritten;
		totalBytesWritten = 0;
		return temp;
	}

	uint64_t getTotalTimeSpentInMicroSec(){
		return totalTimeSpentInMicroSec;
	}

	uint64_t getAndResetTotalTimeSpentInMicroSec(){
		uint64_t temp = totalTimeSpentInMicroSec;
		totalTimeSpentInMicroSec = 0;
		return temp;
	}
	
	uint64_t getcurrBytesRead(){
		return currBytesRead;
	}

	uint64_t getcurrBytesWritten(){
		return currBytesWritten;
	}
};

#endif // tcpclass_H
