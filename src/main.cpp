#include "client.h"
#include "config.h"
#include "curses.h"
#include "log.h"
#include "queue.h"

#include <cstdlib>
#include <experimental/filesystem>
#include <iostream>
#include <memory>
#include <queue>
#include <thread>

void client_runner(Client& client) {
  client.run();
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
}

int main(int argc, char** argv) {
  fs::path config_path;
  if (argc > 1) {
    config_path = argv[1];
  } else {
    config_path = fs::path(getenv("HOME")) / ".config/courier/courier.conf";
  }

  Config config(config_path);
  if (!config.okay) {
    return 1;
  }

  const auto host = config.get("host");
  const auto port = config.get("port");

  if (!host) {
    std::cerr << "IMAP host not found in config" << std::endl;
    return 1;
  }

  if (!port) {
    std::cerr << "IMAP port not found in config" << std::endl;
    return 1;
  }

  const auto user = config.get("user");
  const auto pass = config.get("pass");

  if (!user || !pass) {
    std::cerr << "IMAP user or password not found in config" << std::endl;
    return 1;
  }

  Client client(*host, *port);
  Log logfile("courier.log");

  if (!client.connect()) {
    return 1;
  } else {
    logfile.info("Connected to " + *host + ":" + *port);
  }

  queue<bool> end_signal;
  std::thread client_thread(client_runner, std::ref(client));
  std::queue<std::string> read_q;
  std::thread client_reader_thread(client_reader, std::ref(client), std::ref(read_q), std::ref(end_signal));

  curses::init();

  const auto dim = curses::dimensions();

  curses::Window input(0, std::get<1>(dim) - 5, std::get<0>(dim), 5);
  input.add_border();
  input.write_string(1, 1, "> ");
  input.disable_delay();

  client.login(*user, *pass);

  bool running = true;
  std::string buf;
  buf.reserve(256);
  while (running) {
    while (!read_q.empty()) {
      logfile.info("< " + read_q.front());
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
      logfile.info("> " + line);
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
