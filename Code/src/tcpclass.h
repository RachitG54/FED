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
