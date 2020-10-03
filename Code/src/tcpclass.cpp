
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
#include "tcpclass.h"

void tcpclass::writewithsize(socket_ptr sock, string &message) {
  SSEtime timer;
  boost::system::error_code ignored_error;
  this->currBytesWritten = 0;


  string messagesize = std::to_string(message.size());
  messagesize.resize(constlen,0);
  boost::asio::write(*sock, boost::asio::buffer(messagesize), ignored_error);
  boost::asio::write(*sock, boost::asio::buffer(message), ignored_error);
  timer.donetime();
  this->totalTimeSpentInMicroSec += timer.getTime();
  this->totalBytesWritten += message.size() + constlen;
  this->currBytesWritten += message.size() + constlen;
}

void tcpclass::readstructure(socket_ptr sock,string &message) {
  try {
    SSEtime timer;

    this->currBytesRead = 0;
    ll messagesize;
  	{
  		char msgSizeArr[constlen+1];
  		size_t len = boost::asio::read(*sock, boost::asio::buffer(msgSizeArr, constlen));
  		msgSizeArr[constlen] = '\0';
  		this->totalBytesRead += len;
  		this->currBytesRead += len;
   		messagesize = atoll(msgSizeArr);
  	}

    //Following prevents an extra copy from streambuffer to string
    message.resize(messagesize);
    size_t len = boost::asio::read(*sock, boost::asio::buffer(&message.front(), messagesize));

    timer.donetime();
    this->totalTimeSpentInMicroSec += timer.getTime();
    this->totalBytesRead += len;
    this->currBytesRead += len;
    return;

  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
  return;
}

void tcpclass::writemessage(socket_ptr sock, string message) {
  SSEtime timer;
  int prevmsgsize = message.size();
  message.resize(constlen,0);
  boost::system::error_code ignored_error;
  boost::asio::write(*sock, boost::asio::buffer(message), ignored_error);

  timer.donetime();
  this->totalTimeSpentInMicroSec += timer.getTime();
  this->totalBytesWritten += message.size();
}

long long tcpclass::readlenbytes(socket_ptr sock) {
  try{

    SSEtime timer;
    stringstream ss;
    boost::asio::streambuf read_buffer;
    size_t len = boost::asio::read(*sock, read_buffer,boost::asio::transfer_exactly(constlen));
    string s;
    make_string(read_buffer,s,len);

    ss.write(s.data(), len);
    long long sizestream;
    ss >> sizestream;

    timer.donetime();
    this->totalTimeSpentInMicroSec += timer.getTime();
    this->totalBytesRead += len;

    return sizestream;
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}

void tcpclass::make_string(boost::asio::streambuf& streambuf,string &message, size_t &len)
{
  message.resize(len);
  std::copy(boost::asio::buffers_begin(streambuf.data()),boost::asio::buffers_begin(streambuf.data()) + len,message.begin());
}
