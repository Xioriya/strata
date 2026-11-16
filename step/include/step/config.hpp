#pragma once

#include <map>
#include <string>
#include <vector>

namespace step {

struct Config {
  std::string config_root = "/etc/portage";
  std::string repo_root = "/var/db/repos/gentoo";
  std::string distdir = "/var/cache/distfiles";
  std::string vdb_root = "/var/db/pkg";
  std::string binhost;
  std::string make_conf_path;
  std::map<std::string, std::string> vars;
  std::vector<std::string> repositories;
};

Config load_config();
std::string expand_value(const Config& config, const std::string& input);

}
