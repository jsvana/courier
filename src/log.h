#pragma once

#include <chrono>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <string>

namespace logger {

enum class LogLevel : char {
  DEBUG = 'D',
  INFO = 'I',
  WARNING = 'W',
  ERROR = 'E',
};

void init(const std::string &path);

void log(const LogLevel &level, const std::string &line);

void debug(const std::string &line);
void info(const std::string &line);
void warning(const std::string &line);
void error(const std::string &line);

void cleanup();

} // namespace log
