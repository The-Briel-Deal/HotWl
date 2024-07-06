
// This header describes the interface of how to change focus via keyboard.

#include <wlr/util/box.h>
#ifdef __cplusplus
#include <includes.hpp>
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

#include "scene.hpp"
#include <stdbool.h>

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
EXTERNC bool tiling_focus_move_in_dir(enum gfwl_tiling_focus_direction dir,
                                      struct gfwl_tiling_state *state);
