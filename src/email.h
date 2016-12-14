#pragma once

#include <string>
#include <tuple>
#include <vector>

struct EmailParts {
  int id;
  bool seen;
  std::string date;
  std::string from;
  std::string subject;
};

class Email {
 private:
  const EmailParts parts_;

  const EmailParts get_parts(const std::vector<std::string>& lines) {
    EmailParts parts;
    for (const auto& line : lines) {
      if (line[0] == '*') {
        parts.id = std::stoi(line.data() + 2);
        parts.seen = line.find("\\Seen") != std::string::npos;
      } else if (line.find("Subject: ") == 0) {
        parts.subject = line.substr(line.find(' ') + 1);
      } else if (line.find("Date: ") == 0) {
        parts.date = line.substr(line.find(' ') + 1);
      } else if (line.find("From: ") == 0) {
        parts.from = line.substr(line.find(' ') + 1);
      }
    }

    return parts;
  }

 public:
  bool selected = false;

  const int id;
  const bool seen;
  const std::string subject;
  const std::string date;
  const std::string from;

  Email(const std::vector<std::string>& lines) : parts_(get_parts(lines)),
    id(parts_.id), seen(parts_.id), subject(parts_.subject), date(parts_.date), from(parts_.from) {}

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
