#pragma once

// This header describes the interface of how to change focus via keyboard.

#include "state.hpp"
#include <includes.hpp>
#include <memory>
#include <scene.hpp>
#include <stdbool.h>
#include <wlr/util/box.h>

enum gfwl_tiling_focus_direction {
  GFWL_TILING_FOCUS_LEFT = 1,
  GFWL_TILING_FOCUS_DOWN,
  GFWL_TILING_FOCUS_UP,
  GFWL_TILING_FOCUS_RIGHT,
};

struct gfwl_point {
  int x;
  int y;
};

/*
 * Shift the focus to the next window in any of 4 directions.
 */
bool tiling_focus_move_in_dir(enum gfwl_tiling_focus_direction dir,
                              std::shared_ptr<GfTilingState> state);

gfwl_point get_container_origin(std::shared_ptr<GfContainer> container);
