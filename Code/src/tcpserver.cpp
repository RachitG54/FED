#include "tcpserver.h"

socket_ptr tcpserver::setupserver(char* port) {
  unsigned short portno = atoi(port);
  try {
    tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), portno));
      socket_ptr sock(new tcp::socket(io_service));
      a.accept(*sock);
      return sock;
  }
  catch (std::exception& e) {
      std::cerr << "Exception: " << e.what() << "\n";
      return NULL;
    }
}