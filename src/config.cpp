#include "config.h"

#include <fstream>
#include <iostream>

const std::unordered_map<std::string, std::string>
Config::build_items(const fs::path &path) {
  std::unordered_map<std::string, std::string> items;
  if (!fs::exists(path)) {
    std::cerr << "Config \"" << path << "\" does not exist" << std::endl;
    return items;
  }

  std::ifstream config_f(path);
  if (!config_f.is_open()) {
    std::cerr << "Unable to open config file \"" << path << "\"" << std::endl;
    return items;
  }

  std::string line;
  while (std::getline(config_f, line)) {
    if (line[0] == '#') {
      continue;
    }

    auto split = line.find('=');
    items.emplace(line.substr(0, split), line.substr(split + 1));
  }

  return items;
}

const std::experimental::optional<std::string>
Config::get(const std::string &key) {
  auto iter = items_.find(key);
  if (iter == items_.end()) {
    return std::experimental::nullopt;
  }
  return iter->second;
}
