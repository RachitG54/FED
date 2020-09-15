#ifndef SSEtcpclient
#define SSEtcpclient

#include "SSEincludes.h"
#include "tcpclass.h"
#include "cliquery.h"
#include "SSEtime.h"

using boost::asio::ip::tcp;

typedef boost::shared_ptr<tcp::socket> socket_ptr;

class tcpclient: public tcpclass {
  private:
  public:
    socket_ptr initialize(char* hostname, char* port);
};
#endif // SSEtcpclient