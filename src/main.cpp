#include "client.h"
#include "curses.h"

#include <iostream>
#include <memory>
#include <queue>
#include <thread>

void client_runner(Client& client) {
  client.run();
}

void client_reader(Client& client, std::queue<std::string>& read_q) {
  while (true) {
    read_q.push(client.read());
  }
}

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <host> <port>" << std::endl;
    return 1;
  }

  Client client{argv[1], argv[2]};

  if (!client.connect()) {
    std::cerr << "Error connecting to " << argv[1] << ":" << argv[2] << std::endl;
    return 1;
  } else {
    std::cout << "Connected to " << argv[1] << ":" << argv[2] << std::endl;
  }

  std::thread client_thread(client_runner, std::ref(client));
  std::queue<std::string> read_q;
  std::thread client_reader_thread(client_reader, std::ref(client), std::ref(read_q));

  curses::init();

  const auto dim = curses::dimensions();

  // TODO(jsvana): arrange into screens that get pushed/poped instead of this
  // nonsense
  curses::Window log(0, 0, std::get<0>(dim), std::get<1>(dim) - 5);
  log.add_line("foobar");
  log.add_line("baz");
  log.enable_scroll();

  curses::Window input(0, std::get<1>(dim) - 5, std::get<0>(dim), 5);
  input.add_border();
  input.write_string(1, 1, "> ");
  input.disable_delay();

  bool running = true;
  std::string buf;
  buf.reserve(256);
  while (running) {
    while (!read_q.empty()) {
      log.add_line(read_q.front());
      read_q.pop();
    }
    char c = input.get_char();
    if (c < 0) {
      continue;
    } else if (c == 27) { // Escape
      running = false;
    } else if (c == 13) { // Enter
      if (buf == "") {
        continue;
      }
      client.write(buf);
      log.add_line("[SEND] " + buf);
      buf.clear();
    } else {
      buf += c;
    }
    //input.move_cursor(1, 1);
  }

  client_thread.join();
  client_reader_thread.join();

  curses::cleanup();

  return 0;
}
