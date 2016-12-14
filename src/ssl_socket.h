#pragma once

#include "queue.h"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/bind.hpp>

#include <array>
#include <string>

class SslSocket {
 public:
  const std::string host;
  const std::string port;

 private:
  queue<std::string> read_q_;

  std::array<char, 1024> buffer_;

  boost::asio::io_service io_service_;
  boost::asio::ssl::context ctx_;

  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;

  const std::string LINE_SEP = "\r\n";

 public:
  SslSocket(const std::string& in_host, const std::string& in_port)
      : host(in_host),
        port(in_port),
        ctx_(io_service_, boost::asio::ssl::context::sslv23),
        socket_(io_service_, ctx_) {}

  bool connect();

  void run();

  void stop() { io_service_.stop(); }

  void write(const std::string& message);

  void write_lines(const std::vector<std::string>& lines);

  void read(const boost::system::error_code& error, std::size_t);

  void close();

  queue<std::string>& read_queue() { return read_q_; }
};
