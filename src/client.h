#pragma once

#include "ssl_socket.h"
#include "log.h"

#include <atomic>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

typedef std::function<void(std::vector<std::string>&)> ImapCallback;

class Client {
 private:
  SslSocket sock_;

  std::string unfinished_line_;
  unsigned int message_id_ = 0;

  // Map of message ID -> callback
  std::unordered_map<std::string, ImapCallback> callbacks_;

  std::unique_ptr<std::thread> sock_thread_;
  std::unique_ptr<std::thread> client_reader_thread_;

 public:
  const std::string MESSAGE_ID_PREFIX = "DEADBEEF";

  std::string current_parsing_id;
  std::vector<std::string> current_lines;

  Client(const std::string& host, const std::string& port) : sock_(host, port) {}

  bool connect();

  void run(std::atomic<bool>& running);

  void stop();

  const std::string read();

  const std::string next_id() {
    std::stringstream s;
    s << std::setfill('0') << std::setw(4) << message_id_;
    ++message_id_;
    return MESSAGE_ID_PREFIX + s.str();
  }

  void write(const std::string& id, const std::string& message) {
    logger::debug("[SEND] " + id + " " + message);
    sock_.write(id + " " + message + "\r\n");
  }

  void send(const std::string& message, const ImapCallback& callback) {
    const auto id = next_id();
    callbacks_[id] = callback;
    write(id, message);
  }

  void send(const std::string& message) {
    write(next_id(), message);
  }

  void login(const std::string& username, const std::string& password, const ImapCallback& callback);

  SslSocket& socket() { return sock_; }

  std::unordered_map<std::string, ImapCallback>& callbacks() {
    return callbacks_;
  }
};
