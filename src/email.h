#pragma once

#include <string>
#include <tuple>
#include <vector>

class Email {
 private:
  const std::tuple<int, bool, std::string, std::string, std::string> parts_;

  const std::tuple<int, bool, std::string, std::string, std::string> get_parts(const std::vector<std::string>& lines) {
    int id;
    bool seen;
    std::string subject, date, from;
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

    return std::make_tuple(id, seen, subject, date, from);
  }

 public:
  bool selected = false;

  const int id;
  const bool seen;
  const std::string subject;
  const std::string date;
  const std::string from;

  Email(const std::vector<std::string>& lines) : parts_(get_parts(lines)),
    id(std::get<0>(parts_)), seen(std::get<1>(parts_)),
    subject(std::get<2>(parts_)), date(std::get<3>(parts_)), from(std::get<4>(parts_)) {}

  const std::string str() {
    std::string ret;
    if (selected) {
      ret = "*";
    } else {
      ret = " ";
    }
    ret += " ";
    if (seen) {
      ret += "-";
    } else {
      ret += "@";
    }
    return ret + " " + date + " - " + from + ": " + subject;
  }
};
