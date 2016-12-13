#pragma once

#include "ssl_socket.h"

#include <iomanip>
#include <iostream>
#include <sstream>

class Client {
 private:
  SslSocket sock_;

  std::string unfinished_line_;
  unsigned int message_id_ = 0;

  // Map of message ID -> callback
  std::unordered_map<std::string, std::function<Client&,

 public:
  Client(const std::string& host, const std::string& port) : sock_(host, port) {}

  bool connect();

  void run() { sock_.run(); }

  void stop();

  const std::string read();

  const std::string next_id() {
    std::stringstream s;
    s << std::setfill('0') << std::setw(4) << message_id_;
    ++message_id_;
    return "A" + s.str();
  }

  void write(const std::string& message) {
    sock_.write(message + "\r\n");
  }

  void login(const std::string& username, const std::string& password);

};
