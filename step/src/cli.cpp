#include "step/cli.hpp"

#include <iostream>
#include <sstream>

namespace step {
namespace {

void set_action(Action& chosen, bool& has_action, Action action) {
  chosen = action;
  has_action = true;
}

}

std::string action_name(Action action) {
  switch (action) {
    case Action::Help: return "help";
    case Action::Version: return "version";
    case Action::Info: return "info";
    case Action::Sync: return "sync";
    case Action::Install: return "install";
    case Action::Remove: return "remove";
    case Action::Update: return "update";
    case Action::Search: return "search";
    case Action::Which: return "which";
    case Action::Depclean: return "depclean";
    case Action::Unmask: return "unmask";
    case Action::Mask: return "mask";
    case Action::Use: return "use";
  }
  return "help";
}

void print_help() {
  std::cout <<
    "step 0.0.1 - Strata package manager (pre-alpha)\n"
    "\n"
    "Usage:\n"
    "  step [options] <atom>...\n"
    "  step <command> [args]\n"
    "\n"
    "Commands:\n"
    "  install   resolve and install packages        step -av firefox\n"
    "  remove    uninstall packages                  step -C firefox\n"
    "  update    update the selected set             step -uD @world\n"
    "  sync      sync the repository                 step --sync\n"
    "  search    search package names                step -s rust\n"
    "  which     print the ebuild path for an atom   step which app-editors/neovim\n"
    "  info      print configuration and status      step --info\n"
    "  depclean  remove no-longer-needed packages    step --depclean\n"
    "  unmask    accept a testing keyword            step unmask app-editors/neovim\n"
    "  mask      mask a package                      step mask sys-apps/foo\n"
    "  use       set USE flags for a package         step use app-editors/neovim wayland\n"
    "\n"
    "Options:\n"
    "  -a, --ask       ask before proceeding\n"
    "  -v, --verbose   verbose output\n"
    "  -p, --pretend   show actions without doing them\n"
    "  -u, --update    update request\n"
    "  -D, --deep      deep dependency update\n"
    "  -1, --oneshot   do not add to the world set\n"
    "  -t, --tree      show the dependency tree\n"
    "  -s, --search    search package names\n"
    "  -C, --unmerge   remove packages\n"
    "  -h, --help      show this help\n"
    "      --version   show version\n"
    "\n"
    "Step reads Portage configuration from /etc/portage and consumes Gentoo's\n"
    "ebuild tree unchanged. The resolver, merge engine, and binpkg support are\n"
    "the next milestone.\n";
}

Options parse_cli(int argc, char** argv) {
  Options options;
  std::vector<std::string> args;
  for (int i = 1; i < argc; ++i) args.emplace_back(argv[i]);
  if (args.empty()) return options;

  bool has_action = false;
  Action chosen = Action::Help;
  std::size_t start = 0;

  const std::string& first = args.front();
  if (first == "help") {
    options.action = Action::Help;
    return options;
  }
  if (first == "version") {
    options.action = Action::Version;
    return options;
  }
  if (first == "info") {
    set_action(chosen, has_action, Action::Info);
    start = 1;
  } else if (first == "sync") {
    set_action(chosen, has_action, Action::Sync);
    start = 1;
  } else if (first == "install" || first == "add") {
    set_action(chosen, has_action, Action::Install);
    start = 1;
  } else if (first == "remove" || first == "unmerge" || first == "rm") {
    set_action(chosen, has_action, Action::Remove);
    start = 1;
  } else if (first == "update" || first == "upgrade") {
    set_action(chosen, has_action, Action::Update);
    options.update = true;
    start = 1;
  } else if (first == "search" || first == "find") {
    set_action(chosen, has_action, Action::Search);
    start = 1;
  } else if (first == "which") {
    set_action(chosen, has_action, Action::Which);
    start = 1;
  } else if (first == "depclean") {
    set_action(chosen, has_action, Action::Depclean);
    start = 1;
  } else if (first == "unmask") {
    set_action(chosen, has_action, Action::Unmask);
    start = 1;
  } else if (first == "mask") {
    set_action(chosen, has_action, Action::Mask);
    start = 1;
  } else if (first == "use") {
    set_action(chosen, has_action, Action::Use);
    start = 1;
  }

  bool search_pending = has_action && chosen == Action::Search;
  std::vector<std::string> positionals;

  for (std::size_t i = start; i < args.size(); ++i) {
    const std::string& arg = args[i];

    if (arg == "--") {
      for (++i; i < args.size(); ++i) positionals.push_back(args[i]);
      break;
    }

    if (arg.size() > 2 && arg[0] == '-' && arg[1] == '-') {
      std::string name = arg.substr(2);
      std::string value;
      const std::size_t eq = name.find('=');
      if (eq != std::string::npos) {
        value = name.substr(eq + 1);
        name = name.substr(0, eq);
      }
      if (name == "help") {
        options.action = Action::Help;
        return options;
      }
      if (name == "version") {
        options.action = Action::Version;
        return options;
      }
      if (name == "ask") options.ask = true;
      else if (name == "verbose") options.verbose = true;
      else if (name == "pretend") options.pretend = true;
      else if (name == "update") {
        options.update = true;
        if (!has_action) set_action(chosen, has_action, Action::Update);
      } else if (name == "deep") options.deep = true;
      else if (name == "oneshot") options.oneshot = true;
      else if (name == "tree") options.tree = true;
      else if (name == "sync") set_action(chosen, has_action, Action::Sync);
      else if (name == "info") set_action(chosen, has_action, Action::Info);
      else if (name == "depclean") set_action(chosen, has_action, Action::Depclean);
      else if (name == "search" || name == "searchdesc") {
        set_action(chosen, has_action, Action::Search);
        search_pending = true;
      } else if (name == "unmerge") set_action(chosen, has_action, Action::Remove);
      continue;
    }

    if (arg.size() > 1 && arg[0] == '-') {
      for (std::size_t k = 1; k < arg.size(); ++k) {
        switch (arg[k]) {
          case 'a': options.ask = true; break;
          case 'v': options.verbose = true; break;
          case 'p': options.pretend = true; break;
          case 'u':
            options.update = true;
            if (!has_action) set_action(chosen, has_action, Action::Update);
            break;
          case 'D': options.deep = true; break;
          case '1': options.oneshot = true; break;
          case 't': options.tree = true; break;
          case 's':
            set_action(chosen, has_action, Action::Search);
            search_pending = true;
            break;
          case 'C': set_action(chosen, has_action, Action::Remove); break;
          case 'h':
            options.action = Action::Help;
            return options;
          case 'V':
            options.action = Action::Version;
            return options;
          default: break;
        }
      }
      continue;
    }

    positionals.push_back(arg);
  }

  if (search_pending && chosen == Action::Search) {
    std::ostringstream query;
    for (std::size_t i = 0; i < positionals.size(); ++i) {
      if (i != 0) query << ' ';
      query << positionals[i];
    }
    options.query = query.str();
  } else {
    for (const std::string& p : positionals) {
      if (p == "@world") options.world = true;
      options.packages.push_back(p);
    }
  }

  if (!has_action) {
    if (options.update) chosen = Action::Update;
    else if (!options.packages.empty()) chosen = Action::Install;
    else chosen = Action::Help;
  }

  options.action = chosen;
  return options;
}

}
