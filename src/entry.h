#pragma once

#include <string>

class Entry {
 private:
  const std::string body_;

 public:
  bool selected = false;

  Entry(const std::string& body) : body_(body) {}

  const std::string str() {
    std::string ret;
    if (selected) {
      ret = "*";
    } else {
      ret = " ";
    }
    return ret + " " + body_;
  }
};
