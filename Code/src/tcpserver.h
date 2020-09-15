#ifndef SSEtcpserver
#define SSEtcpserver

#include "SSEincludes.h"
#include "tcpclass.h"
#include "cliquery.h"
#include "authtok.h"
#include "SSEtime.h"

using boost::asio::ip::tcp;

typedef boost::shared_ptr<tcp::socket> socket_ptr;

class tcpserver: public tcpclass {
  private:
  public:
	socket_ptr setupserver(char* port);
};

#endif // SSEtcpserver