#include <map>
#include <nlohmann/json.hpp>
#include <string>

class GfConfig {
  GfConfig(std::string config_path);

public:
  std::string get_option(std::string);
  std::string set_option(std::string);

private:
  std::map<std::string, std::string> data;
};
