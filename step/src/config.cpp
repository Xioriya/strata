#include "step/config.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace step {
namespace {

std::string env_or(const char* key, const std::string& fallback) {
  const char* value = std::getenv(key);
  return (value != nullptr && *value != '\0') ? std::string(value) : fallback;
}

std::string trim(std::string text) {
  const auto not_space = [](unsigned char c) { return !std::isspace(c); };
  text.erase(text.begin(), std::find_if(text.begin(), text.end(), not_space));
  text.erase(std::find_if(text.rbegin(), text.rend(), not_space).base(), text.end());
  return text;
}

std::string unquote(std::string text) {
  if (text.size() >= 2) {
    const char first = text.front();
    const char last = text.back();
    if ((first == '"' && last == '"') || (first == '\'' && last == '\'')) {
      return text.substr(1, text.size() - 2);
    }
  }
  return text;
}

void read_key_values(const fs::path& path, std::map<std::string, std::string>& out) {
  std::ifstream input(path);
  std::string line;
  while (std::getline(input, line)) {
    line = trim(line);
    if (line.empty() || line[0] == '#') continue;
    const std::size_t eq = line.find('=');
    if (eq == std::string::npos) continue;
    std::string key = trim(line.substr(0, eq));
    std::string value = unquote(trim(line.substr(eq + 1)));
    if (key.empty()) continue;
    out[std::move(key)] = std::move(value);
  }
}

}

Config load_config() {
  Config config;
  config.config_root = env_or("STRATA_CONFIG_ROOT", "/etc/portage");
  config.repo_root = env_or("STRATA_REPO_ROOT", "/var/db/repos/gentoo");
  config.distdir = env_or("STRATA_DISTDIR", "/var/cache/distfiles");
  config.vdb_root = env_or("STRATA_VDB_ROOT", "/var/db/pkg");
  config.make_conf_path = config.config_root + "/make.conf";

  read_key_values(config.make_conf_path, config.vars);

  const auto binhost = config.vars.find("PORTAGE_BINHOST");
  if (binhost != config.vars.end()) config.binhost = binhost->second;

  config.repositories.push_back("gentoo");

  std::error_code ec;
  const fs::path repos_conf = fs::path(config.config_root) / "repos.conf";
  if (fs::is_directory(repos_conf, ec)) {
    for (const fs::directory_entry& entry : fs::directory_iterator(repos_conf, ec)) {
      if (entry.path().extension() != ".conf") continue;
      std::ifstream input(entry.path());
      std::string line;
      while (std::getline(input, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#' || line[0] == '[') continue;
        const std::size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        const std::string key = trim(line.substr(0, eq));
        const std::string value = unquote(trim(line.substr(eq + 1)));
        if (key == "location" && !value.empty()) config.repositories.push_back(value);
      }
    }
  }

  return config;
}

std::string expand_value(const Config& config, const std::string& input) {
  std::string out;
  for (std::size_t i = 0; i < input.size();) {
    if (input[i] != '$') {
      out += input[i++];
      continue;
    }
    const std::size_t start = i;
    std::string name;
    if (i + 1 < input.size() && input[i + 1] == '{') {
      const std::size_t close = input.find('}', i + 2);
      if (close == std::string::npos) {
        out += input[i++];
        continue;
      }
      name = input.substr(i + 2, close - i - 2);
      i = close + 1;
    } else {
      std::size_t j = i + 1;
      while (j < input.size() && (std::isalnum(static_cast<unsigned char>(input[j])) || input[j] == '_')) ++j;
      name = input.substr(i + 1, j - i - 1);
      i = j;
    }
    if (name.empty()) {
      out += input[start];
      ++i;
      continue;
    }
    const auto it = config.vars.find(name);
    if (it != config.vars.end()) out += it->second;
  }
  return out;
}

}
