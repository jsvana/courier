#pragma once

#include <string>
#include <tuple>
#include <vector>

struct Email {
  int id;
  bool seen;
  std::string date;
  std::string from;
  std::string subject;

  Email(const std::vector<std::string>& lines) {
    for (const auto& line : lines) {
      if (line[0] == '*') {
        id = std::stoi(line.data() + 2);
        seen = line.find("\\Seen") != std::string::npos;
      } else if (line.find("Subject: ") == 0) {
        subject = line.substr(line.find(' ') + 1);
      } else if (line.find("Date: ") == 0) {
        date = line.substr(line.find(' ') + 1);
      } else if (line.find("From: ") == 0) {
        from = line.substr(line.find(' ') + 1);
      }
    }
  }

  const std::string str() const {
    std::string ret;
    if (seen) {
      ret = "-";
    } else {
      ret = "@";
    }
    return ret + " " + date + " - " + from + ": " + subject;
  }
};
