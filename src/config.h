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
  const std::tuple<std::unordered_map<std::string, std::string>, bool> parts_;
  const std::unordered_map<std::string, std::string> items_;

  const std::tuple<std::unordered_map<std::string, std::string>, bool> build_parts(const fs::path& path);

 public:
  const bool okay;

  Config(const fs::path& path) : path_(path), parts_(build_parts(path)),
    items_(std::get<0>(parts_)), okay(std::get<1>(parts_)) {}

  const std::experimental::optional<std::string> get(const std::string& key);
};
