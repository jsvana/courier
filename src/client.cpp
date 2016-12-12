#include "client.h"

#include <thread>

void reader(Client& client) {
  while (true) {
    const auto message = client.read();
    std::cout << "[RECV] " << message << std::endl;
  }
}

void Client::run() {
  std::thread reader_thread(reader, std::ref(*this));

  sock_.run();

  reader_thread.join();
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
