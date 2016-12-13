#pragma once

#include <chrono>
#include <fstream>
#include <iomanip>
#include <string>

enum class LogLevel : char {
  DEBUG = 'D',
  INFO = 'I',
  WARNING = 'W',
  ERROR = 'E',
};

class Log {
 private:
  const std::string path_;
  std::ofstream log_f;

 public:
  Log(const std::string& path) : path_(path) {
    log_f.open(path, std::ios::out | std::ios::app);
  }

  ~Log() {
    log_f.close();
  }

  void log(const LogLevel& level, const std::string& line) {
    const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    log_f << std::put_time(std::localtime(&now), "%D %H:%M:%S") << " " << static_cast<char>(level) << " " << line << std::endl;
  }

  void debug(const std::string& line) { log(LogLevel::DEBUG, line); }
  void info(const std::string& line) { log(LogLevel::INFO, line); }
  void warning(const std::string& line) { log(LogLevel::WARNING, line); }
  void error(const std::string& line) { log(LogLevel::ERROR, line); }
};
