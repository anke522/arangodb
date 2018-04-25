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
/// @author Dr. Frank Celler
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGOD_SCHEDULER_SOCKET_H
#define ARANGOD_SCHEDULER_SOCKET_H 1

#include "Basics/Common.h"

#include <boost/asio/basic_stream_socket.hpp>
#include <boost/asio/serial_port_service.hpp>
#include <boost/asio/ssl.hpp>

#include <thread>
#include <chrono>

#include "Basics/StringBuffer.h"
#include "Basics/asio-helper.h"
#include "Logger/Logger.h"

namespace arangodb {

typedef std::function<void(const boost::system::error_code& ec,
                           std::size_t transferred)>
    AsyncHandler;

class Socket {
 public:
  Socket(boost::asio::io_service& ioService, bool encrypted)
      : _strand(ioService), _encrypted(encrypted) {}
  Socket(Socket const& that) = delete;
  Socket(Socket&& that) = delete;
  virtual ~Socket() {}
  
  bool isEncrypted() const { return _encrypted; }
  bool handshake();
  boost::asio::io_service::strand& strand() { return _strand; }
  
  virtual std::string peerAddress() const = 0;
  virtual int peerPort() const = 0;
  
  /// Enable non-blocking mode
  virtual void setNonBlocking(bool) = 0;
  
  virtual size_t writeSome(basics::StringBuffer* buffer,
                           boost::system::error_code& ec) = 0;
  virtual void asyncWrite(boost::asio::mutable_buffers_1 const& buffer,
                          AsyncHandler const& handler) = 0;
  virtual size_t readSome(boost::asio::mutable_buffers_1 const& buffer,
                          boost::system::error_code& ec) = 0;
  virtual std::size_t available(boost::system::error_code& ec) = 0;
  virtual void asyncRead(boost::asio::mutable_buffers_1 const& buffer,
                         AsyncHandler const& handler) = 0;

  virtual void close(boost::system::error_code& ec) = 0;
  
  void shutdown(boost::system::error_code& ec, bool mustCloseSend, bool mustCloseReceive) {
    if (mustCloseSend) {
      this->shutdownSend(ec);
      if (ec && ec != boost::asio::error::not_connected) {
        LOG_TOPIC(DEBUG, Logger::COMMUNICATION)
            << "shutdown send stream failed with: " << ec.message();
      }
    }

    if (mustCloseReceive) {
      this->shutdownReceive(ec);
      if (ec && ec != boost::asio::error::not_connected) {
        LOG_TOPIC(DEBUG, Logger::COMMUNICATION)
            << "shutdown receive stream failed with: " << ec.message();
      }
    }
  }

 protected:
  virtual bool sslHandshake() = 0;
  virtual void shutdownReceive(boost::system::error_code& ec) = 0;
  virtual void shutdownSend(boost::system::error_code& ec) = 0;

 private:
   /// The io_context used to perform socket operations.
  //boost::asio::io_service& _ioService;
  /// Strand to ensure the connection's handlers are not called concurrently.
  boost::asio::io_service::strand _strand;
  bool const _encrypted;
  bool _handshakeDone = false;
};
}

#endif
