#pragma once

#include <experimental/filesystem>
#include <experimental/optional>
#include <string>
#include <tuple>
#include <unordered_map>

namespace fs = std::experimental::filesystem;

class Config {
private:
  const fs::path path_;
  const std::unordered_map<std::string, std::string> items_;

  const std::unordered_map<std::string, std::string>
  build_items(const fs::path &path);

public:
  const bool okay;

  Config(const fs::path &path)
      : path_(path), items_(build_items(path)), okay(!items_.empty()) {}

  const std::experimental::optional<std::string> get(const std::string &key);
};
