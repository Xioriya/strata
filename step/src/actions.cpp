#include "step/actions.hpp"
#include "step/version.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

namespace step {
namespace {

std::string shell_quote(const std::string& text) {
  std::string out = "'";
  for (const char c : text) {
    if (c == '\'') out += "'\\''";
    else out += c;
  }
  out += "'";
  return out;
}

int run_command(const std::string& command, bool dry) {
  std::cerr << "$ " << command << '\n';
  if (dry) return 0;
  return std::system(command.c_str());
}

void append_unique(const fs::path& path, const std::vector<std::string>& lines, bool dry) {
  std::set<std::string> existing;
  {
    std::ifstream input(path);
    std::string line;
    while (std::getline(input, line)) {
      if (!line.empty() && line[0] != '#') existing.insert(line);
    }
  }

  std::vector<std::string> additions;
  for (const std::string& line : lines) {
    if (existing.insert(line).second) additions.push_back(line);
  }

  if (additions.empty()) {
    std::cout << "no changes for " << path << '\n';
    return;
  }

  std::cout << (dry ? "would append to " : "appending to ") << path << '\n';
  for (const std::string& line : additions) std::cout << "  " << line << '\n';
  if (dry) return;

  std::error_code ec;
  fs::create_directories(path.parent_path(), ec);
  std::ofstream output(path, std::ios::app);
  for (const std::string& line : additions) output << line << '\n';
}

std::string lowercase(std::string text) {
  std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  return text;
}

int command_info(const Config& config) {
  std::cout << "step " << kVersion << " (" << kCodename << ")\n";
  std::cout << "config root : " << config.config_root << '\n';
  std::cout << "repo root   : " << config.repo_root << '\n';
  std::cout << "make.conf   : " << config.make_conf_path << '\n';
  std::cout << "distdir     : " << config.distdir << '\n';
  std::cout << "vdb root    : " << config.vdb_root << '\n';
  std::cout << "binhost     : " << (config.binhost.empty() ? "(none)" : config.binhost) << '\n';
  std::cout << "repositories:";
  for (const std::string& repo : config.repositories) std::cout << ' ' << repo;
  std::cout << '\n';

  std::cout << "make.conf variables:\n";
  for (const auto& [key, value] : config.vars) {
    std::cout << "  " << key << '=' << expand_value(config, value) << '\n';
  }

  std::error_code ec;
  std::size_t installed = 0;
  if (fs::is_directory(config.vdb_root, ec)) {
    for (const fs::directory_entry& category : fs::directory_iterator(config.vdb_root, ec)) {
      if (!category.is_directory()) continue;
      for (const fs::directory_entry& pkg : fs::directory_iterator(category.path(), ec)) {
        if (pkg.is_directory()) ++installed;
      }
    }
  }
  std::cout << "installed   : " << installed << " packages in " << config.vdb_root << '\n';
  std::cout << "repo status : "
            << (fs::exists(fs::path(config.repo_root) / ".git", ec) ? "git checkout present" : "not synced")
            << '\n';
  return 0;
}

int command_sync(const Config& config, const Options& options) {
  std::error_code ec;
  const fs::path root(config.repo_root);

  if (fs::exists(root / ".git", ec)) {
    const std::string command = "git -C " + shell_quote(root.string()) + " pull --ff-only";
    if (run_command(command, options.pretend) != 0) {
      std::cerr << "step: sync failed\n";
      return 1;
    }
    std::cout << "synced " << root << '\n';
    return 0;
  }

  fs::create_directories(root.parent_path(), ec);
  const std::string command = "git clone --depth=1 https://github.com/gentoo/gentoo " + shell_quote(root.string());
  if (run_command(command, options.pretend) != 0) {
    std::cerr << "step: clone failed\n";
    return 1;
  }
  std::cout << "cloned the Gentoo tree into " << root << '\n';
  return 0;
}

long number_at(const std::string& text, std::size_t& index) {
  long value = 0;
  while (index < text.size() && std::isdigit(static_cast<unsigned char>(text[index]))) {
    value = value * 10 + (text[index] - '0');
    ++index;
  }
  return value;
}

int suffix_rank(const std::string& word) {
  if (word == "alpha") return 1;
  if (word == "beta") return 2;
  if (word == "pre") return 3;
  if (word == "rc") return 4;
  if (word == "p") return 5;
  return 4;
}

struct Version {
  std::vector<long> numbers;
  char letter = '\0';
  std::vector<std::pair<int, long>> suffixes;
  long revision = 0;
};

Version parse_version(const std::string& text) {
  Version version;
  std::string rest = text;

  const std::size_t revision = rest.rfind("-r");
  if (revision != std::string::npos && revision + 2 < rest.size()) {
    bool digits = true;
    for (std::size_t i = revision + 2; i < rest.size(); ++i) {
      if (!std::isdigit(static_cast<unsigned char>(rest[i]))) digits = false;
    }
    if (digits) {
      version.revision = std::stol(rest.substr(revision + 2));
      rest = rest.substr(0, revision);
    }
  }

  std::string base = rest;
  const std::size_t underscore = rest.find('_');
  if (underscore != std::string::npos) {
    base = rest.substr(0, underscore);
    const std::string suffix_text = rest.substr(underscore + 1);
    std::size_t start = 0;
    while (start <= suffix_text.size()) {
      const std::size_t next = suffix_text.find('_', start);
      const std::string item = suffix_text.substr(start, next == std::string::npos ? std::string::npos : next - start);
      std::size_t index = 0;
      std::string word;
      while (index < item.size() && std::isalpha(static_cast<unsigned char>(item[index]))) {
        word += item[index];
        ++index;
      }
      long number = 0;
      if (index < item.size() && std::isdigit(static_cast<unsigned char>(item[index]))) {
        number = number_at(item, index);
      }
      version.suffixes.emplace_back(suffix_rank(word), number);
      if (next == std::string::npos) break;
      start = next + 1;
    }
  }

  std::size_t index = 0;
  while (index < base.size()) {
    if (std::isdigit(static_cast<unsigned char>(base[index]))) {
      version.numbers.push_back(number_at(base, index));
    } else if (std::isalpha(static_cast<unsigned char>(base[index]))) {
      version.letter = base[index];
      ++index;
    } else {
      ++index;
    }
  }

  return version;
}

int compare_versions(const std::string& left, const std::string& right) {
  const Version a = parse_version(left);
  const Version b = parse_version(right);

  const std::size_t number_count = std::max(a.numbers.size(), b.numbers.size());
  for (std::size_t i = 0; i < number_count; ++i) {
    const long x = i < a.numbers.size() ? a.numbers[i] : 0;
    const long y = i < b.numbers.size() ? b.numbers[i] : 0;
    if (x != y) return x < y ? -1 : 1;
  }

  if (a.letter != b.letter) return a.letter < b.letter ? -1 : 1;

  const std::size_t suffix_count = std::max(a.suffixes.size(), b.suffixes.size());
  for (std::size_t i = 0; i < suffix_count; ++i) {
    const bool has_a = i < a.suffixes.size();
    const bool has_b = i < b.suffixes.size();
    if (has_a && has_b) {
      const auto& [rank_a, number_a] = a.suffixes[i];
      const auto& [rank_b, number_b] = b.suffixes[i];
      if (rank_a != rank_b) return rank_a < rank_b ? -1 : 1;
      if (number_a != number_b) return number_a < number_b ? -1 : 1;
      continue;
    }
    if (has_a) return a.suffixes[i].first == 5 ? 1 : -1;
    if (has_b) return b.suffixes[i].first == 5 ? -1 : 1;
  }

  if (a.revision != b.revision) return a.revision < b.revision ? -1 : 1;
  return 0;
}

int command_search(const Config& config, const std::string& query, std::size_t limit) {
  std::error_code ec;
  const fs::path root(config.repo_root);
  if (!fs::is_directory(root, ec)) {
    std::cerr << "step: repository not synced: " << root << '\n';
    return 1;
  }
  if (query.empty()) {
    std::cerr << "step: search needs a query\n";
    return 2;
  }

  const std::string needle = lowercase(query);
  std::vector<std::string> hits;

  for (const fs::directory_entry& category : fs::directory_iterator(root, ec)) {
    if (!category.is_directory()) continue;
    const std::string category_name = category.path().filename().string();
    if (category_name.empty() || category_name[0] == '.') continue;
    for (const fs::directory_entry& pkg : fs::directory_iterator(category.path(), ec)) {
      if (!pkg.is_directory()) continue;
      const std::string name = pkg.path().filename().string();
      if (name.empty() || name[0] == '.') continue;
      const std::string atom = category_name + "/" + name;
      if (lowercase(atom).find(needle) != std::string::npos) {
        hits.push_back(atom);
        if (hits.size() >= limit) break;
      }
    }
    if (hits.size() >= limit) break;
  }

  std::sort(hits.begin(), hits.end());
  for (const std::string& hit : hits) std::cout << hit << '\n';
  if (hits.empty()) {
    std::cout << "no packages matched '" << query << "'\n";
    return 1;
  }
  std::cout << "(" << hits.size() << " shown, limit " << limit << ")\n";
  return 0;
}

int command_which(const Config& config, const Options& options) {
  if (options.packages.empty()) {
    std::cerr << "step: which needs an atom\n";
    return 2;
  }

  const std::string& atom = options.packages.front();
  const std::size_t slash = atom.find('/');
  if (slash == std::string::npos || slash + 1 >= atom.size()) {
    std::cerr << "step: expected category/package, got '" << atom << "'\n";
    return 2;
  }

  const std::string category = atom.substr(0, slash);
  const std::string package = atom.substr(slash + 1);
  std::string name = package;
  std::string version;
  const std::size_t dash = package.rfind('-');
  if (dash != std::string::npos && dash + 1 < package.size() &&
      std::isdigit(static_cast<unsigned char>(package[dash + 1]))) {
    name = package.substr(0, dash);
    version = package.substr(dash + 1);
  }

  std::error_code ec;
  const fs::path root(config.repo_root);
  const fs::path directory = root / category / name;
  if (!fs::is_directory(directory, ec)) {
    std::cerr << "step: package not found: " << category << '/' << name << '\n';
    return 1;
  }

  std::vector<fs::path> ebuilds;
  for (const fs::directory_entry& entry : fs::directory_iterator(directory, ec)) {
    if (!entry.is_regular_file()) continue;
    if (entry.path().extension() != ".ebuild") continue;
    ebuilds.push_back(entry.path());
  }
  if (ebuilds.empty()) {
    std::cerr << "step: no ebuilds in " << directory << '\n';
    return 1;
  }

  const std::string prefix = name + '-';
  auto version_of = [&](const fs::path& path) {
    const std::string stem = path.stem().string();
    if (stem.rfind(prefix, 0) != 0) return std::string();
    return stem.substr(prefix.size());
  };

  fs::path chosen;
  if (!version.empty()) {
    for (const fs::path& path : ebuilds) {
      if (version_of(path) == version) {
        chosen = path;
        break;
      }
    }
    if (chosen.empty()) {
      std::cerr << "step: no ebuild for " << atom << '\n';
      return 1;
    }
  } else {
    chosen = *std::max_element(ebuilds.begin(), ebuilds.end(), [&](const fs::path& a, const fs::path& b) {
      return compare_versions(version_of(a), version_of(b)) < 0;
    });
  }

  const fs::path relative = fs::relative(chosen, root, ec);
  std::cout << (ec ? chosen.string() : relative.string()) << '\n';
  return 0;
}

int command_unmask(const Config& config, const Options& options) {
  if (options.packages.empty()) {
    std::cerr << "step: unmask needs at least one atom\n";
    return 2;
  }
  std::vector<std::string> lines;
  for (const std::string& atom : options.packages) lines.push_back(atom + " ~amd64");
  const fs::path path = fs::path(config.config_root) / "package.accept_keywords" / "strata.conf";
  append_unique(path, lines, options.pretend);
  return 0;
}

int command_mask(const Config& config, const Options& options) {
  if (options.packages.empty()) {
    std::cerr << "step: mask needs at least one atom\n";
    return 2;
  }
  const fs::path path = fs::path(config.config_root) / "package.mask" / "strata.conf";
  append_unique(path, options.packages, options.pretend);
  return 0;
}

int command_use(const Config& config, const Options& options) {
  if (options.packages.size() < 2) {
    std::cerr << "step: use needs an atom and at least one flag\n";
    return 2;
  }
  std::ostringstream line;
  line << options.packages.front();
  for (std::size_t i = 1; i < options.packages.size(); ++i) line << ' ' << options.packages[i];
  const fs::path path = fs::path(config.config_root) / "package.use" / "strata.conf";
  append_unique(path, {line.str()}, options.pretend);
  return 0;
}

int not_implemented(const char* name) {
  std::cerr << "step: " << name << " is not implemented in " << kVersion << " (pre-alpha)\n";
  std::cerr << "the resolver, merge engine, and binpkg layer are the next milestone\n";
  return 2;
}

}

int run(const Options& options, const Config& config) {
  switch (options.action) {
    case Action::Help:
      print_help();
      return 0;
    case Action::Version:
      std::cout << kName << ' ' << kVersion << " (" << kCodename << ")\n";
      return 0;
    case Action::Info:
      return command_info(config);
    case Action::Sync:
      return command_sync(config, options);
    case Action::Search:
      return command_search(config, options.query, 50);
    case Action::Which:
      return command_which(config, options);
    case Action::Unmask:
      return command_unmask(config, options);
    case Action::Mask:
      return command_mask(config, options);
    case Action::Use:
      return command_use(config, options);
    case Action::Install:
      return not_implemented("install");
    case Action::Remove:
      return not_implemented("remove");
    case Action::Update:
      return not_implemented("update");
    case Action::Depclean:
      return not_implemented("depclean");
  }
  print_help();
  return 2;
}

}
