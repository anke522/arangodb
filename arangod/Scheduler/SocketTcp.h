////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2016-2018 ArangoDB GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Andreas Streichardt
/// @author Simon Grätzer
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGOD_SCHEDULER_SOCKET_TCP_H
#define ARANGOD_SCHEDULER_SOCKET_TCP_H 1

//#include "Basics/MutexLocker.h"
#include "Scheduler/Socket.h"

#include <boost/asio/ip/tcp.hpp>

namespace arangodb {

class SocketTcp final : public Socket {
 public:
  SocketTcp(boost::asio::io_service& ioService)
      : Socket(ioService, /*encrypted*/false),  _socket(ioService),
        _peerEndpoint() {}

  SocketTcp(SocketTcp const& that) = delete;
  SocketTcp(SocketTcp&& that) = delete;
  
  std::string peerAddress() const override {
    return _peerEndpoint.address().to_string();
  }

  int peerPort() const override { return _peerEndpoint.port(); }
  
  void setNonBlocking(bool v) override {
    //MUTEX_LOCKER(guard, _lock);
    _socket.non_blocking(v);
  }

  size_t writeSome(basics::StringBuffer* buffer, boost::system::error_code& ec) override {
    return _socket.write_some(boost::asio::buffer(buffer->begin(), buffer->length()), ec);
  }
  
  void asyncWrite(boost::asio::mutable_buffers_1 const& buffer,
                  AsyncHandler const& handler) override {
    return boost::asio::async_write(_socket, buffer, handler);
  }
  
  size_t readSome(boost::asio::mutable_buffers_1 const& buffer,
                  boost::system::error_code& ec) override {
    return _socket.read_some(buffer, ec);
  }
  
  void asyncRead(boost::asio::mutable_buffers_1 const& buffer,
                 AsyncHandler const& handler) override {
    return _socket.async_read_some(buffer, handler);
  }

  void close(boost::system::error_code& ec) override;
  std::size_t available(boost::system::error_code& ec) override {
    return static_cast<size_t>(_socket.available(ec));
  }

 protected:
  
  bool sslHandshake() override { return false; }
  void shutdownReceive(boost::system::error_code& ec) override;
  void shutdownSend(boost::system::error_code& ec) override;

 public:
  boost::asio::ip::tcp::socket _socket;
  boost::asio::ip::tcp::acceptor::endpoint_type _peerEndpoint;
};
  
class SocketSslTcp final : public Socket {
  
public:
  SocketSslTcp(boost::asio::io_service& ioService,
               boost::asio::ssl::context&& context)
  : Socket(ioService, /*encrypted*/true),
    _sslContext(std::move(context)),
    _sslSocket(ioService, _sslContext),
    _socket(_sslSocket.next_layer()),
    _peerEndpoint() {}
  
  SocketSslTcp(SocketSslTcp const& that) = delete;
  SocketSslTcp(SocketSslTcp&& that) = delete;
  
  std::string peerAddress() const override {
    return _peerEndpoint.address().to_string();
  }
  
  int peerPort() const override { return _peerEndpoint.port(); }
  
  void setNonBlocking(bool v) override {
    //MUTEX_LOCKER(guard, _lock);
    _socket.non_blocking(v);
  }
  
  size_t writeSome(basics::StringBuffer* buffer,
                   boost::system::error_code& ec) override {
    return _sslSocket.write_some(boost::asio::buffer(buffer->begin(), buffer->length()), ec);
  }
  
  void asyncWrite(boost::asio::mutable_buffers_1 const& buffer,
                  AsyncHandler const& handler) override {
    return boost::asio::async_write(_sslSocket, buffer, handler);
  }
  
  size_t readSome(boost::asio::mutable_buffers_1 const& buffer,
                  boost::system::error_code& ec) override {
    return _sslSocket.read_some(buffer, ec);
  }
  
  void asyncRead(boost::asio::mutable_buffers_1 const& buffer,
                 AsyncHandler const& handler) override {
    return _sslSocket.async_read_some(buffer, handler);
  }
  
  // mop: these functions actually only access the underlying socket. The
  // _sslSocket is actually just an additional layer around the socket. These low level
  // functions access the _socket only and it is ok that they are not implemented for
  // _sslSocket in the children
  
  std::size_t available(boost::system::error_code& ec) override {
    return static_cast<size_t>(_socket.available(ec));
  }
  
  //void shutdown(boost::system::error_code& ec, bool closeSend, bool closeReceive) override;
  void close(boost::system::error_code& ec) override;
  
protected:
  
  bool sslHandshake() override;
  void shutdownReceive(boost::system::error_code& ec) override;
  void shutdownSend(boost::system::error_code& ec) override;
  
public:
  
  boost::asio::ssl::context _sslContext;
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> _sslSocket;
  
  boost::asio::ip::tcp::socket& _socket;
  boost::asio::ip::tcp::acceptor::endpoint_type _peerEndpoint;
};
  
}

#endif
