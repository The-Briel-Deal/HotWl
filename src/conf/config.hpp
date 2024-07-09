#pragma once

#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <xkbcommon/xkbcommon-keysyms.h>
#include <xkbcommon/xkbcommon.h>

class GfConfig {
public:
  GfConfig();
  GfConfig(std::string config_path);
  std::string get_option(std::string);
  std::string set_option(std::string);
  bool parse_file(std::string);

  struct GfKeybinds {
    /* Open Kitty */
    xkb_keysym_t new_term = XKB_KEY_q;
    /* Open Launcher */
    xkb_keysym_t launcher = XKB_KEY_r;
    /* Exit WM */
    xkb_keysym_t exit = XKB_KEY_m;
    /* Tiling Focus Directions */
    xkb_keysym_t tiling_focus_left = XKB_KEY_h;
    xkb_keysym_t tiling_focus_down = XKB_KEY_j;
    xkb_keysym_t tiling_focus_up = XKB_KEY_k;
    xkb_keysym_t tiling_focus_right = XKB_KEY_l;
    /* Flip Split Dir */
    xkb_keysym_t flip_split_direction = XKB_KEY_s;
  } keybinds;

private:
  std::map<std::string, std::string> data;
};
