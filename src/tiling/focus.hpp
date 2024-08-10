#pragma once

#include <includes.hpp>
#include <memory>
#include <scene.hpp>
#include <vector>
#include <wlr/util/box.h>

#include "state.hpp"

class GfContainer;
class GfContainerToplevel;
struct GfTilingState;
enum gfwl_tiling_focus_direction : char;

struct GfPoint {
  int x;
  int y;
};

bool    tiling_focus_move_in_dir(enum gfwl_tiling_focus_direction      dir,
                                 const std::shared_ptr<GfTilingState>& state);

GfPoint get_container_origin(const std::shared_ptr<GfContainer>& container);

std::weak_ptr<GfContainerToplevel> find_closest_to_origin_in_dir(
    struct GfPoint origin,
    const std::vector<std::weak_ptr<GfContainerToplevel>>&
                                     toplevel_container_list,
    enum gfwl_tiling_focus_direction dir);
