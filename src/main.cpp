#include "client.h"
#include "config.h"
#include "curses.h"
#include "log.h"
#include "queue.h"

#include <atomic>
#include <cstdlib>
#include <experimental/filesystem>
#include <iostream>
#include <memory>
#include <queue>
#include <thread>

void client_reader(Client& client, std::queue<std::string>& read_q, std::atomic<bool>& running) {
  while (running) {
    const auto line = client.read();
    if (!line.empty()) {
      read_q.push(line);
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

  auto get_or_die = [&config](const std::string& key) {
    const auto value = config.get(key);
    if (!value) {
      std::cerr << key << " not found in config" << std::endl;
      exit(EXIT_FAILURE);
    }
    return *value;
  };
  const auto host = get_or_die("host");
  const auto port = get_or_die("port");
  const auto user = get_or_die("user");
  const auto pass = get_or_die("pass");

  Client client(host, port);
  Log logfile("courier.log");

  if (!client.connect()) {
    return 1;
  } else {
    logfile.info("Connected to " + host + ":" + port);
  }

  std::atomic<bool> running(true);
  std::queue<std::string> read_q;
  std::thread client_thread([&client]() {
    client.run();
  });
  std::thread client_reader_thread([&client, &read_q, &running]() {
    client_reader(client, read_q, running);
  });

  curses::init();

  const auto dim = curses::dimensions();

  curses::InputWindow input(0, std::get<1>(dim) - 5, std::get<0>(dim), 5);

  client.login(user, pass);

  std::string buf;
  buf.reserve(256);
  while (running) {
    while (!read_q.empty()) {
      logfile.info("< " + read_q.front());
      read_q.pop();
      input.sync_display();
    }

    if (!input.update(client, logfile)) {
      client.stop();
      running = false;
    }
  }

  client_thread.join();
  client_reader_thread.join();

  curses::cleanup();

  return 0;
}
