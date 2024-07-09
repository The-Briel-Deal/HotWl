#include <conf/config.hpp>
#include <cstdlib>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <xkbcommon/xkbcommon.h>

using json = nlohmann::json;

bool set_keybind(xkb_keysym_t *bind, json keyname_json) {
  if (!keyname_json.is_string())
    return false;

  std::string keyname = keyname_json;
  xkb_keysym_t new_bind =
      xkb_keysym_from_name(keyname.c_str(), XKB_KEYSYM_CASE_INSENSITIVE);

  if (bind && new_bind)
    *bind = new_bind;
  else
    return false;
  return true;
}

// TODO: Add Ability to close window and move.
// Return true if it succeeds.
bool GfConfig::parse_file(std::string config_path) {
  // Create input filestream for config file.

  std::ifstream f(config_path);
  if (!f.fail()) {
    json data = json::parse(f);

    /* Open Kitty */
    set_keybind(&this->keybinds.new_term, data["new_term"]);
    /* Open Launcher */
    set_keybind(&this->keybinds.launcher, data["launcher"]);
    /* Exit WM */
    set_keybind(&this->keybinds.exit, data["exit"]);
    /* Tiling Focus Directions */
    set_keybind(&this->keybinds.tiling_focus_up, data["tiling_focus_up"]);
    set_keybind(&this->keybinds.tiling_focus_down, data["tiling_focus_down"]);
    set_keybind(&this->keybinds.tiling_focus_left, data["tiling_focus_left"]);
    set_keybind(&this->keybinds.tiling_focus_right, data["tiling_focus_right"]);
    /* Flip Split Dir */
    set_keybind(&this->keybinds.flip_split_direction,
                data["flip_split_direction"]);

    return true;
  }
  return false;
};

GfConfig::GfConfig() {
  parse_file(std::string(std::getenv("HOME")) + "/.config/gfwl.json");
}
GfConfig::GfConfig(std::string config_path) { parse_file(config_path); }
