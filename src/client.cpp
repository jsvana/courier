#include "client.h"

#include <thread>

void Client::stop() {
  sock_.read_queue().push("\r\n");
  sock_.stop();
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

void Client::login(const std::string& username, const std::string& password) {
  write(next_id() + " LOGIN " + username + " " + password);
}
