#include <algorithm>
#include <cctype>
#include <conf/config.hpp>
#include <cstdlib>
#include <fstream>
#include <includes.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <xkbcommon/xkbcommon.h>

using json = nlohmann::json;

bool set_keybind(xkb_keysym_t& bind, json keyname_json) {
  if (!keyname_json.is_string())
    return false;

  std::string  keyname = keyname_json;
  xkb_keysym_t new_bind =
      xkb_keysym_from_name(keyname.c_str(), XKB_KEYSYM_CASE_INSENSITIVE);

  if (bind && new_bind)
    bind = new_bind;
  else
    return false;
  return true;
}

wlr_keyboard_modifier get_mod_from_string(std::string mod_str) {
  std::transform(mod_str.begin(), mod_str.end(), mod_str.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  if (mod_str == "super" || mod_str == "logo" || mod_str == "meta") {
    return WLR_MODIFIER_LOGO;
  } else if (mod_str == "ctrl" || mod_str == "control") {
    return WLR_MODIFIER_CTRL;
  } else if (mod_str == "shift") {
    return WLR_MODIFIER_SHIFT;
  } else if (mod_str == "alt") {
    return WLR_MODIFIER_ALT;
  }
  return (wlr_keyboard_modifier)NULL;
}

bool set_mod(xkb_mod_mask_t& modmask, json modname_json) {
  if (!modname_json.is_string())
    return false;

  std::string    keyname  = modname_json;
  xkb_mod_mask_t new_mask = get_mod_from_string(keyname);

  if (modmask && new_mask)
    modmask = new_mask;
  else
    return false;
  return true;
}

// Return true if it succeeds.
bool GfConfig::parse_file(std::string config_path) {
  // Create input filestream for config file.

  std::ifstream f(config_path);
  if (!f.fail()) {
    json data = json::parse(f);

    /* Set Main Modifier Key */
    set_mod(this->keybinds.modmask, data["mod"]);

    /* Open Kitty */
    set_keybind(this->keybinds.new_term, data["new_term"]);
    /* Open Launcher */
    set_keybind(this->keybinds.launcher, data["launcher"]);
    /* Exit WM */
    set_keybind(this->keybinds.exit, data["exit"]);
    /* Tiling Focus Directions */
    set_keybind(this->keybinds.tiling_focus_up, data["tiling_focus_up"]);
    set_keybind(this->keybinds.tiling_focus_down, data["tiling_focus_down"]);
    set_keybind(this->keybinds.tiling_focus_left, data["tiling_focus_left"]);
    set_keybind(this->keybinds.tiling_focus_right, data["tiling_focus_right"]);
    /* Flip Split Dir */
    set_keybind(this->keybinds.flip_split_direction,
                data["flip_split_direction"]);
    set_keybind(this->keybinds.close_surface, data["tiling_focus_right"]);

    return true;
  }
  return false;
};

GfConfig::GfConfig() {
  parse_file(std::string(std::getenv("HOME")) + "/.config/gfwl.json");
}

GfConfig::GfConfig(std::string config_path) {
  parse_file(config_path);
}
