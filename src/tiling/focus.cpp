#include <assert.h>
#include <climits>
#include <includes.hpp>
#include <memory>
#include <output.hpp>
#include <server.hpp>
#include <stdlib.h>
#include <tiling/focus.hpp>
#include <vector>
#include <wayland-util.h>
#include <xdg_shell.hpp>

// TODO: Make tiling_state object oriented in cpp.
static std::shared_ptr<GfContainer>
get_container_in_dir(enum gfwl_tiling_focus_direction dir,
                     std::shared_ptr<GfTilingState> state);

static bool focus_and_warp_to_container(std::shared_ptr<GfContainer> contaiener,
                                        std::shared_ptr<GfTilingState> state);

static struct wl_list *
get_toplevel_container_list(std::shared_ptr<GfContainer> head,
                            struct wl_list *list);

static std::shared_ptr<GfContainer> find_closest_to_origin_in_dir(
    struct gfwl_point origin,
    const std::vector<std::shared_ptr<GfContainer>> &toplevel_container_list,
    enum gfwl_tiling_focus_direction dir) {
  // Iterate through all Toplevel Containers, if we are going left, we
  // should look for a container where the curr focused container's y value
  // is within (new_focused_y < curr_focused_center_y &&
  //         new_focused_y + new_focusedheight > curr_focused_center_y &&
  //         new_focused_x < curr_focused_center_x)
  // With this container save it along with the distance
  //        (new_focused_x - curr_focused_center_x)
  // And return the one with the closest distance.

  std::shared_ptr<GfContainer> closest_valid_toplevel = NULL;
  switch (dir) {
  case GFWL_TILING_FOCUS_LEFT: {
    int shortest_distance = INT_MAX;

    for (auto toplevel : toplevel_container_list) {
      int distance_left_of_origin = origin.x - toplevel->box.x;
      if (toplevel->box.x < origin.x && toplevel->box.y <= origin.y &&
          toplevel->box.y + toplevel->box.height >= origin.y &&
          distance_left_of_origin < shortest_distance &&
          toplevel != toplevel->server->active_toplevel_container) {
        // TODO: Actually pass in the currently focused toplevel.
        // If distance is negative something went wrong.
        assert(distance_left_of_origin > 0);
        closest_valid_toplevel = toplevel;
        shortest_distance = distance_left_of_origin;
      }
    }
    break;
  }
  case GFWL_TILING_FOCUS_RIGHT: {
    int shortest_distance = INT_MAX;

    for (auto toplevel : toplevel_container_list) {
      int distance_right_of_origin = toplevel->box.x - origin.x;
      if (toplevel->box.x > origin.x && toplevel->box.y <= origin.y &&
          toplevel->box.y + toplevel->box.height >= origin.y &&
          distance_right_of_origin < shortest_distance &&
          toplevel != toplevel->server->active_toplevel_container) {
        // TODO: Actually pass in the currently focused toplevel.
        // If distance is negative something went wrong.
        assert(distance_right_of_origin > 0);
        closest_valid_toplevel = toplevel;
        shortest_distance = distance_right_of_origin;
      }
    }
    break;
  }
  case GFWL_TILING_FOCUS_UP: {
    int shortest_distance = INT_MAX;

    for (auto toplevel : toplevel_container_list) {
      int distance_above_origin = origin.y - toplevel->box.y;
      if (toplevel->box.y < origin.y && toplevel->box.x <= origin.x &&
          toplevel->box.x + toplevel->box.width >= origin.x &&
          distance_above_origin < shortest_distance &&
          toplevel != toplevel->server->active_toplevel_container) {
        // If distance is negative something went wrong.
        assert(distance_above_origin > 0);
        closest_valid_toplevel = toplevel;
        shortest_distance = distance_above_origin;
      }
    }
    break;
  }
  case GFWL_TILING_FOCUS_DOWN: {
    int shortest_distance = INT_MAX;

    for (auto toplevel : toplevel_container_list) {
      int distance_below_origin = toplevel->box.y - origin.y;
      if (toplevel->box.y > origin.y && toplevel->box.x <= origin.x &&
          toplevel->box.x + toplevel->box.width >= origin.x &&
          distance_below_origin < shortest_distance &&
          toplevel != toplevel->tiling_state->active_toplevel_container) {
        // If distance is negative something went wrong.
        assert(distance_below_origin > 0);
        closest_valid_toplevel = toplevel;
        shortest_distance = distance_below_origin;
      }
    }
    break;
  }
  }

  return closest_valid_toplevel;
}

bool tiling_focus_move_in_dir(enum gfwl_tiling_focus_direction dir,
                              std::shared_ptr<GfTilingState> state) {
  // Get Container In The Specified Direction.
  std::shared_ptr<GfContainer> container_to_focus =
      get_container_in_dir(dir, state);
  if (container_to_focus == NULL) {
    return false;
  }

  // Focus container.
  // TODO: I think I can just return focus_and_warp_to_container
  if (focus_and_warp_to_container(container_to_focus, state) != true) {
    return false;
  };
  return true;
}

static std::shared_ptr<GfContainer>
get_container_in_dir(enum gfwl_tiling_focus_direction dir,
                     std::shared_ptr<GfTilingState> state) {
  assert(state);

  std::shared_ptr<GfContainer> curr_focused = state->active_toplevel_container;
  assert(curr_focused);

  // Get Currently Focused Container Origin, X and Y position.
  struct gfwl_point curr_focused_origin = get_container_origin(curr_focused);

  // Get List of all Toplevel Containers.

  auto toplevel_container_list = state->root->get_top_level_container_list();

  std::shared_ptr<GfContainer> to_focus = find_closest_to_origin_in_dir(
      curr_focused_origin, toplevel_container_list, dir);

  if (to_focus) {
    assert(to_focus->e_type == GFWL_CONTAINER_TOPLEVEL);
    return to_focus;
  }

  return NULL;
}

static bool focus_and_warp_to_container(std::shared_ptr<GfContainer> container,
                                        std::shared_ptr<GfTilingState> state) {
  assert(container && container->e_type == GFWL_CONTAINER_TOPLEVEL);

  struct gfwl_toplevel *toplevel = container->toplevel;
  assert(toplevel);

  struct wlr_surface *surface = toplevel->xdg_toplevel->base->surface;
  assert(surface);

  if (toplevel == NULL || surface == NULL) {
    wlr_log(WLR_ERROR, "Couldn't focus because of null toplevel or surface.");
    return false;
  }

  focus_toplevel(container->toplevel,
                 container->toplevel->xdg_toplevel->base->surface);

  focus_output_from_container(container);

  return true;
}

gfwl_point get_container_origin(std::shared_ptr<GfContainer> container) {
  struct wlr_box box = container->box;
  struct gfwl_point center = {.x = (box.width / 2) + box.x,
                              .y = (box.height / 2) + box.y};
  return center;
}
