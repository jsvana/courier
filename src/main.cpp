#include "client.h"
#include "curses.h"
#include "queue.h"

#include <iostream>
#include <memory>
#include <queue>
#include <thread>

void client_runner(Client& client) {
  client.run();
  std::cout << "runner done" << std::endl;
}

void client_reader(Client& client, std::queue<std::string>& read_q, queue<bool>& end_signal) {
  while (true) {
    const auto line = client.read();
    if (line != "") {
      read_q.push(line);
    }
    if (!end_signal.empty()) {
      end_signal.pop();
      break;
    }
  }
  std::cout << "reader done" << std::endl;
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

  queue<bool> end_signal;
  std::thread client_thread(client_runner, std::ref(client));
  std::queue<std::string> read_q;
  std::thread client_reader_thread(client_reader, std::ref(client), std::ref(read_q), std::ref(end_signal));

  curses::init();

  const auto dim = curses::dimensions();

  // TODO(jsvana): arrange into screens that get pushed/poped instead of this
  // nonsense
  curses::Window log(0, 0, std::get<0>(dim), std::get<1>(dim) - 5);
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
      log.add_line("< " + read_q.front());
      read_q.pop();
      input.move_cursor(3 + buf.length(), 1);
    }

    char c = input.get_char();
    if (c < 0) {
      continue;
    } else if (c == 27) { // Escape
      client.stop();
      end_signal.push(true);
      running = false;
    } else if (c == 13) { // Enter
      if (buf == "") {
        continue;
      }
      const auto line = client.next_id() + " " + buf;
      client.write(line);
      log.add_line("> " + line);
      buf.clear();
      input.clear_line(1);
      input.write_string(1, 1, "> ");
    } else if (c == 8 || c == 127) { // Backspace
      buf = buf.substr(0, buf.length() - 1);
      input.delete_char(3 + buf.length(), 1);
    } else {
      buf += c;
      // "|> "
      input.write_char(2 + buf.length(), 1, c);
    }
  }

  client_thread.join();
  client_reader_thread.join();

  curses::cleanup();

  return 0;
}
