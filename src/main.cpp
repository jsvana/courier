#include "client.h"
#include "config.h"
#include "curses.h"
#include "email.h"
#include "log.h"
#include "queue.h"

#include <atomic>
#include <cstdlib>
#include <experimental/filesystem>
#include <iostream>
#include <memory>
#include <queue>
#include <thread>

const int FETCH_COUNT = 10;

void fetch_inbox(Client& client, Log& logfile, int start, int end, curses::EmailWindow& list) {
  logfile.debug("FETCHING FROM " + std::to_string(start) + " TO " + std::to_string(end));
  client.send("FETCH " + std::to_string(start) + ":" + std::to_string(end)
      + " (FLAGS BODY[HEADER.FIELDS (DATE FROM SUBJECT)])", [&client, &logfile, &list](std::vector<std::string>& lines) {
    // Parse emails
    std::vector<std::string> email_lines;
    for (const auto& line : lines) {
      logfile.debug(line);
      if (line == ")") {
        list.add_email({email_lines});
        email_lines.clear();
        continue;
      }
      email_lines.push_back(line);
    }
  });
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

  Log logfile("courier.log");
  Client client(host, port, logfile);

  if (!client.connect()) {
    return 1;
  } else {
    logfile.info("Connected to " + host + ":" + port);
  }

  std::atomic<bool> running(true);
  std::queue<std::string> read_q;
  std::thread client_thread([&client, &logfile, &running]() {
    client.run(running);
  });

  curses::init();

  const auto dim = curses::dimensions();

  curses::EmailWindow list(0, 0, std::get<0>(dim), std::get<1>(dim) - 5);

  client.login(user, pass, [&client, &logfile, &list](std::vector<std::string>&) {
    client.send("SELECT \"INBOX\"", [&client, &logfile, &list](std::vector<std::string>& lines) {
      int start, end = -1;
      for (const auto& line : lines) {
        if (line.find("EXISTS") == std::string::npos) {
          continue;
        }
        end = std::stoi(line.data() + 2);
        start = end - FETCH_COUNT;
      }
      if (end == -1) {
        logfile.error("Unable to find EXISTS in LOGIN response");
        return;
      }

      fetch_inbox(client, logfile, start, end, list);
    });
  });

  std::string buf;
  buf.reserve(256);
  while (running) {
    while (!read_q.empty()) {
      logfile.info("< " + read_q.front());
      read_q.pop();
    }

    if (!list.update(client, logfile)) {
      client.stop();
      running = false;
    }

    list.sync_display();
  }

  client_thread.join();

  curses::cleanup();

  return 0;
}
