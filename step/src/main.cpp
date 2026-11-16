#include "step/actions.hpp"
#include "step/cli.hpp"
#include "step/config.hpp"

int main(int argc, char** argv) {
  step::Options options = step::parse_cli(argc, argv);
  step::Config config = step::load_config();
  return step::run(options, config);
}
