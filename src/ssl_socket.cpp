#include "ssl_socket.h"

#include <iostream>

bool SslSocket::connect() {
  boost::asio::ip::tcp::resolver resolver(io_service_);
  boost::asio::ip::tcp::resolver::query query(host, port);
  auto iterator = resolver.resolve(query);

  // No verification!
  ctx_.set_verify_mode(boost::asio::ssl::context::verify_none);

  auto& socket = socket_.lowest_layer();

  boost::asio::ip::tcp::resolver::iterator end;
  boost::system::error_code error = boost::asio::error::host_not_found;
  while (error && iterator != end) {
    socket.close();
    socket.connect(*iterator++, error);
  }

  if (error) {
    return false;
  }

  socket_.handshake(boost::asio::ssl::stream_base::client);

  return true;
}

void SslSocket::run() {
  socket_.async_read_some(
      boost::asio::buffer(buffer_),
      boost::bind(&SslSocket::read, this, boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));

  io_service_.run();
}

void SslSocket::write(const std::string& message) {
  boost::asio::write(socket_, boost::asio::buffer(message));
}

void SslSocket::write_lines(const std::vector<std::string>& lines) {
  for (const auto& line : lines) {
    write(line);
  }
}

void SslSocket::read(const boost::system::error_code& error,
                     std::size_t bytes) {
  if (error) {
    close();
    return;
  }

  std::string data(buffer_.data(), bytes);
  read_q_.push(data);
  socket_.async_read_some(
      boost::asio::buffer(buffer_),
      boost::bind(&SslSocket::read, this, boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
}

void SslSocket::close() {
  socket_.lowest_layer().close();
  io_service_.stop();
}
