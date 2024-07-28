#pragma once

#include <includes.hpp>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <xkbcommon/xkbcommon-keysyms.h>
#include <xkbcommon/xkbcommon-names.h>
#include <xkbcommon/xkbcommon.h>

class GfConfig {
public:
  GfConfig();
  explicit GfConfig(const std::string& config_path);
  std::string get_option(std::string);
  std::string set_option(std::string);
  bool        parse_file(const std::string&);

  struct GfKeybinds {
    xkb_mod_mask_t modmask = WLR_MODIFIER_CTRL;
    /* Open Kitty */
    xkb_keysym_t new_term = XKB_KEY_q;
    /* Open Launcher */
    xkb_keysym_t launcher = XKB_KEY_r;
    /* Exit WM */
    xkb_keysym_t exit = XKB_KEY_m;
    /* Tiling Focus Directions */
    xkb_keysym_t tiling_focus_left  = XKB_KEY_h;
    xkb_keysym_t tiling_focus_down  = XKB_KEY_j;
    xkb_keysym_t tiling_focus_up    = XKB_KEY_k;
    xkb_keysym_t tiling_focus_right = XKB_KEY_l;
    /* Flip Split Dir */
    xkb_keysym_t flip_split_direction = XKB_KEY_s;
    /* Go to Next And Previous Monitor */
    xkb_keysym_t next_monitor  = XKB_KEY_n;
    xkb_keysym_t prev_monitor  = XKB_KEY_p;
    xkb_keysym_t close_surface = XKB_KEY_c;
  } keybinds;

private:
  std::map<std::string, std::string> data;
};
