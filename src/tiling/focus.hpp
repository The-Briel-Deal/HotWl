#pragma once

#include <includes.hpp>
#include <memory>
#include <scene.hpp>
#include <wlr/util/box.h>

#include "state.hpp"

class GfContainer;
struct GfTilingState;

enum gfwl_tiling_focus_direction {
  GFWL_TILING_FOCUS_LEFT = 1,
  GFWL_TILING_FOCUS_DOWN,
  GFWL_TILING_FOCUS_UP,
  GFWL_TILING_FOCUS_RIGHT,
};

struct GfPoint {
  int x;
  int y;
};

bool    tiling_focus_move_in_dir(enum gfwl_tiling_focus_direction      dir,
                                 const std::shared_ptr<GfTilingState>& state);

GfPoint get_container_origin(const std::shared_ptr<GfContainer>& container);
