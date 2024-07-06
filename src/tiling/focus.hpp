#pragma once

// This header describes the interface of how to change focus via keyboard.

#include "scene.hpp"
#include <includes.hpp>
#include <stdbool.h>
#include <wlr/util/box.h>

struct gfwl_tiling_state {
public:
  void insert(gfwl_container *container);
  void flip_split_direction();
  struct gfwl_container *root;
  struct gfwl_container *active_toplevel_container;
  enum gfwl_split_direction split_dir;

private:
  struct gfwl_server *server;
};

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
                                      struct gfwl_tiling_state *state);
