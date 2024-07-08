#include <conf/config.hpp>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;
GfConfig::GfConfig(std::string config_path) {
  // Create input filestream for config file.
  std::ifstream f(config_path);
  if (!f.fail()) {
    json data = json::parse(f);
    // TODO: Finish this class.
  } else {
    /* TODO: Handle the case where its not able to find the file. (Probably
     * also check if the json could be parsed) */
  }
}
