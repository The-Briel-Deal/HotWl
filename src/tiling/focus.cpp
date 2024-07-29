#include <cassert>
#include <climits>
#include <deque>
#include <memory>
#include <output.hpp>
#include <server.hpp>
#include <tiling/focus.hpp>
#include <vector>
#include <xdg_shell.hpp>

#include "tiling/container/base.hpp"
#include "tiling/container/root.hpp"
#include "tiling/container/toplevel.hpp"
#include "tiling/state.hpp"

extern "C" {
#include "wayland-util.h"
#include "wlr/types/wlr_compositor.h"
#include "wlr/types/wlr_xdg_shell.h"
#include "wlr/util/box.h"
#include "wlr/util/log.h"
}

// TODO(gabe): Make tiling_state object oriented in cpp.
static std::shared_ptr<GfContainerToplevel>
            get_container_in_dir(enum gfwl_tiling_focus_direction      dir,
                                 const std::shared_ptr<GfTilingState>& state);

static bool focus_and_warp_to_container(
    const std::shared_ptr<GfContainerToplevel>& container);

static struct wl_list*
get_toplevel_container_list(std::shared_ptr<GfContainer> head,
                            struct wl_list*              list);

static std::weak_ptr<GfContainerToplevel> find_closest_to_origin_in_dir(
    struct GfPoint                                 origin,
    const std::vector<std::weak_ptr<GfContainer>>& toplevel_container_list,
    enum gfwl_tiling_focus_direction               dir) {

  std::weak_ptr<GfContainer> closest_valid_toplevel;
  switch (dir) {
    case GFWL_TILING_FOCUS_LEFT: {
      int shortest_distance = INT_MAX;

      for (const auto& toplevel_weak : toplevel_container_list) {
        auto toplevel = toplevel_weak.lock();
        if (!toplevel) {
          continue;
        }
        auto tl_box                  = toplevel->get_box();
        int  distance_left_of_origin = origin.x - tl_box.x;
        if (tl_box.x < origin.x && tl_box.y <= origin.y &&
            tl_box.y + tl_box.height >= origin.y &&
            distance_left_of_origin < shortest_distance &&
            toplevel != g_Server.active_toplevel_container.front().lock()) {
          assert(distance_left_of_origin > 0);
          closest_valid_toplevel = toplevel;
          shortest_distance      = distance_left_of_origin;
        }
      }
      break;
    }
    case GFWL_TILING_FOCUS_RIGHT: {
      int shortest_distance = INT_MAX;

      for (const auto& toplevel_weak : toplevel_container_list) {
        auto toplevel = toplevel_weak.lock();
        if (!toplevel) {
          continue;
        }
        auto tl_box                   = toplevel->get_box();
        int  distance_right_of_origin = tl_box.x - origin.x;
        if (tl_box.x > origin.x && tl_box.y <= origin.y &&
            tl_box.y + tl_box.height >= origin.y &&
            distance_right_of_origin < shortest_distance &&
            toplevel != g_Server.active_toplevel_container.front().lock()) {
          assert(distance_right_of_origin > 0);
          closest_valid_toplevel = toplevel;
          shortest_distance      = distance_right_of_origin;
        }
      }
      break;
    }
    case GFWL_TILING_FOCUS_UP: {
      int shortest_distance = INT_MAX;

      for (const auto& toplevel_weak : toplevel_container_list) {
        auto toplevel = toplevel_weak.lock();
        if (!toplevel) {
          continue;
        }
        auto tl_box                = toplevel->get_box();
        int  distance_above_origin = origin.y - tl_box.y;
        if (tl_box.y < origin.y && tl_box.x <= origin.x &&
            tl_box.x + tl_box.width >= origin.x &&
            distance_above_origin < shortest_distance &&
            toplevel != g_Server.active_toplevel_container.front().lock()) {
          assert(distance_above_origin > 0);
          closest_valid_toplevel = toplevel;
          shortest_distance      = distance_above_origin;
        }
      }
      break;
    }
    case GFWL_TILING_FOCUS_DOWN: {
      int shortest_distance = INT_MAX;

      for (const auto& toplevel_weak : toplevel_container_list) {
        auto toplevel = toplevel_weak.lock();
        if (!toplevel) {
          continue;
        }
        auto tl_box                = toplevel->get_box();
        int  distance_below_origin = tl_box.y - origin.y;
        if (tl_box.y > origin.y && tl_box.x <= origin.x &&
            tl_box.x + tl_box.width >= origin.x &&
            distance_below_origin < shortest_distance &&
            toplevel != g_Server.active_toplevel_container.front().lock()) {
          assert(distance_below_origin > 0);
          closest_valid_toplevel = toplevel;
          shortest_distance      = distance_below_origin;
        }
      }
      break;
    }
  }

  return std::dynamic_pointer_cast<GfContainerToplevel>(
      closest_valid_toplevel.lock());
}

bool tiling_focus_move_in_dir(enum gfwl_tiling_focus_direction      dir,
                              const std::shared_ptr<GfTilingState>& state) {
  std::shared_ptr<GfContainerToplevel> container_to_focus =
      get_container_in_dir(dir, state);
  if (container_to_focus == nullptr) {
    return false;
  }

  if (!focus_and_warp_to_container(container_to_focus)) {
    return false;
  };
  return true;
}

static std::shared_ptr<GfContainerToplevel>
get_container_in_dir(enum gfwl_tiling_focus_direction      dir,
                     const std::shared_ptr<GfTilingState>& state) {
  assert(state);

  if (g_Server.active_toplevel_container.empty() ||
      g_Server.active_toplevel_container.front().expired()) {
    return nullptr;
  }
  std::shared_ptr<GfContainer> curr_focused =
      g_Server.active_toplevel_container.front().lock();
  assert(curr_focused);

  struct GfPoint curr_focused_origin = get_container_origin(curr_focused);

  auto toplevel_container_list = state->root->get_top_level_container_list();

  std::shared_ptr<GfContainerToplevel> to_focus =
      find_closest_to_origin_in_dir(
          curr_focused_origin, toplevel_container_list, dir)
          .lock();

  if (to_focus) {
    assert(to_focus->e_type == GFWL_CONTAINER_TOPLEVEL);
    return to_focus;
  }

  return nullptr;
}

static bool focus_and_warp_to_container(
    const std::shared_ptr<GfContainerToplevel>& container) {
  assert(container && container->e_type == GFWL_CONTAINER_TOPLEVEL);

  const GfToplevel* toplevel = container->toplevel;
  assert(toplevel);

  struct wlr_surface* surface = toplevel->xdg_toplevel->base->surface;
  assert(surface);

  if (toplevel == nullptr || surface == nullptr) {
    wlr_log(WLR_ERROR, "Couldn't focus because of null toplevel or surface.");
    return false;
  }

  container->set_focused_toplevel_container();

  focus_output_from_container(container);

  return true;
}

GfPoint get_container_origin(const std::shared_ptr<GfContainer>& container) {
  auto           box    = container->get_box();
  struct GfPoint center = {.x = (box.width / 2) + box.x,
                           .y = (box.height / 2) + box.y};
  return center;
}
