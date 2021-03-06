
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