#include "client.h"

#include <thread>

void client_reader(Client* client, std::atomic<bool>& running) {
  while (running) {
    const auto line = client->read();

    logger::debug("[RECV] " + line);

    if (line.find(client->MESSAGE_ID_PREFIX) == 0) {
      const auto line_id = line.substr(0, line.find(' '));
      if (line.find(line_id + " OK ") == std::string::npos) {
        // Assume new data
        client->current_parsing_id = line_id;
      } else {
        // Completing existing request"
        auto iter = client->callbacks().find(line_id);
        if (iter != client->callbacks().end()) {
          iter->second(client->current_lines);
          client->callbacks().erase(iter);
        }
        client->current_lines.clear();
      }
    } else {
      // Assume parsing current data
      client->current_lines.push_back(line);
    }
  }
}

bool Client::connect() {
  try {
    return sock_.connect();
  } catch (const std::exception& e) {
    std::cerr << "Error connecting to " << sock_.host << ":" << sock_.port
      << ": " << e.what() << std::endl;
    return false;
  }
}

void Client::run(std::atomic<bool>& running) {
  sock_thread_ = std::make_unique<std::thread>([this]() {
    this->socket().run();
  });

  client_reader_thread_ = std::make_unique<std::thread>([this, &running]() {
    client_reader(this, running);
  });
}

void Client::stop() {
  sock_.read_queue().push("\r\n\r\n");
  sock_.stop();

  sock_thread_->join();
  client_reader_thread_->join();
}

const std::string Client::read() {
  // Read in batches because not all lines will end in \r\n
  std::size_t end;
  std::string line;
  while (end = unfinished_line_.find("\r\n"), end == std::string::npos) {
    unfinished_line_ += sock_.read_queue().pop();
  }

  line = unfinished_line_.substr(0, end);
  unfinished_line_ = unfinished_line_.substr(end + 2);

  return line;
}

void Client::login(const std::string& username, const std::string& password, const ImapCallback& callback) {
  send("LOGIN " + username + " " + password, callback);
}
