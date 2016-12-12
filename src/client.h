#pragma once

#include "ssl_socket.h"

#include <iostream>

class Client {
 private:
  SslSocket sock_;

  std::string unfinished_line_;

 public:
  Client(const std::string& host, const std::string& port) : sock_(host, port) {}

  bool connect() { return sock_.connect(); }

  void run();

  const std::string read();

  void write(const std::string& message) {
    std::cout << "[SEND] " << message << std::endl;
    sock_.write(message);
  }

};
