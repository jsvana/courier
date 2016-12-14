#include "log.h"

namespace logger {

std::mutex file_lock_;
std::ofstream log_f;

void init(const std::string& path) {
  log_f.open(path, std::ios::out | std::ios::app);
}

void log(const LogLevel& level, const std::string& line) {
  std::lock_guard<std::mutex> guard(file_lock_);
  const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  log_f << std::put_time(std::localtime(&now), "%D %H:%M:%S") << " " << static_cast<char>(level) << " " << line << std::endl;
}

void debug(const std::string& line) { log(LogLevel::DEBUG, line); }
void info(const std::string& line) { log(LogLevel::INFO, line); }
void warning(const std::string& line) { log(LogLevel::WARNING, line); }
void error(const std::string& line) { log(LogLevel::ERROR, line); }

void cleanup() {
  log_f.close();
}

} // namespace log
