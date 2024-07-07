#include "tiling/container.hpp"
#include <includes.hpp>
#include <memory>
#include <output.hpp>
#include <scene.hpp>
#include <server.hpp>
#include <xdg_shell.hpp>

// TODO: I want to move all tiling and container logic to its own class.
enum gfwl_split_direction get_split_dir(std::shared_ptr<GfContainer> container);
void split_containers(std::shared_ptr<GfContainer> container);

// Shouldn't be needed anymore.
// static u_int16_t count_toplevel_containers(struct wl_list *toplevels) {
//   assert(toplevels);
//   if (!toplevels)
//     return 0;
//   struct GfContainer *curr_toplevel;
//   u_int16_t count = 0;
//   wl_list_for_each(curr_toplevel, toplevels, link) { count += 1; }
//   return count;
// }

// It would be nice to log containers during this for debugging.
void parse_containers(std::shared_ptr<GfContainer> container) {
  if (container->is_root) {
    // Get output.
    struct gfwl_output *output =
        wl_container_of(container->server->outputs.next, output, link);
    container->box.width = output->wlr_output->width;
    container->box.height = output->wlr_output->height;
  }
  split_containers(container);
  struct GfContainer *cursor;
  // TODO: Rewrite this with shared_ptr and vector.
  //  struct wl_list *head = &container->child_containers;
  //  wl_list_for_each(cursor, head, link) {
  //    if (cursor->e_type == GFWL_CONTAINER_HSPLIT ||
  //        cursor->e_type == GFWL_CONTAINER_VSPLIT) {
  //      parse_containers(cursor);
  //    }
  //  }
}

// TODO: Replace duplicate parts with generalized helpers.
// Change this to get output size from the parent.
void vert_split_containers(struct std::shared_ptr<GfContainer> container) {
  // Get count.
  int count = container->child_containers.size();
  if (count == 0) {
    wlr_log(WLR_DEBUG, "You probably don't want to divide by 0");
    return;
  }

  // Get Width and Height.
  int width = container->box.width;
  int height = container->box.height;

  // Get per_win_width.
  int per_win_height = height / count;

  // Set all sizes. (recycling count for the index)
  count = 0;
  struct GfContainer *curr_toplevel_container;
  // TODO: Rewrite this loop with Vector.
  //   wl_list_for_each(curr_toplevel_container, toplevel_containers, link) {
  //     const struct wlr_box box = {.x = container->box.x,
  //                                 .y = per_win_height * count,
  //                                 .width = width,
  //                                 .height = per_win_height};
  //     set_container_box(curr_toplevel_container, box);
  //     count += 1;
  //   }
}

void hori_split_containers(struct std::shared_ptr<GfContainer> container) {
  // Get count.
  int count = container->child_containers.size();
  if (count == 0) {
    wlr_log(WLR_DEBUG, "You probably don't want to divide by 0");
    return;
  }

  // Get Width and Height.
  int width = container->box.width;
  int height = container->box.height;

  // Get per_win_width.
  int per_win_width = width / count;

  // Set all sizes. (recycling count for the index)
  count = 0;
  struct GfContainer *curr_toplevel_container;
  //   wl_list_for_each(curr_toplevel_container, toplevel_containers, link) {
  //     const struct wlr_box box = {.x = count * per_win_width,
  //                                 .y = 0,
  //                                 .width = per_win_width,
  //                                 .height = height};
  //     set_container_box(curr_toplevel_container, box);
  //     count += 1;
  //   }
}

void split_containers(std::shared_ptr<GfContainer>container) {
  switch (container->get_split_dir()) {
  case GFWL_SPLIT_DIR_HORI:
    hori_split_containers(container);
    break;
  case GFWL_SPLIT_DIR_VERT:
    vert_split_containers(container);
    break;
  default:
    break;
  }
}

// Only supports toplevels for now.
void set_container_box(struct GfContainer *container, struct wlr_box box) {
  container->box = box;
  if (container->e_type == GFWL_CONTAINER_TOPLEVEL) {
    struct wlr_xdg_toplevel *toplevel = container->toplevel->xdg_toplevel;
    // Set the size.
    wlr_xdg_toplevel_set_size(toplevel, box.width, box.height);
    // Set the position.
    struct wlr_scene_tree *scene_tree = (wlr_scene_tree *)toplevel->base->data;
    wlr_scene_node_set_position(&scene_tree->node, box.x, box.y);
  }
};

std::shared_ptr<GfContainer>
create_parent_container(std::shared_ptr<GfContainer> child_container,
                        enum gfwl_container_type type) {
  // Just making sure I'm never passing unknown.
  assert(type != GFWL_CONTAINER_UNKNOWN);

  // TODO: Make sure this is good.
  struct std::shared_ptr<GfContainer> parent_container =
      std::make_shared<GfContainer>(false, nullptr, type, nullptr,
                                    child_container->server, nullptr);
  parent_container->e_type = type;
  parent_container->server = child_container->server;
  parent_container->tiling_state = child_container->tiling_state;
  child_container->parent_container = parent_container;
  // Change these list operations to use the vector.
  // wl_list_init(&parent_container->child_containers);
  // wl_list_insert(&parent_container->child_containers,
  // &child_container->link);
  return parent_container;
}

std::shared_ptr<GfContainer>
create_container_from_toplevel(struct gfwl_toplevel *toplevel) {
  // Replace calloc with new
  std::shared_ptr<GfContainer> container =
      std::make_shared<GfContainer>(false, nullptr, GFWL_CONTAINER_TOPLEVEL,
                                    nullptr, toplevel->server, toplevel);

  toplevel->parent_container = container;

  return container;
}

// Only Tested with toplevel_containers.

void set_focused_toplevel_container(std::shared_ptr<GfContainer> container) {
  assert(container);
  struct gfwl_tiling_state *tiling_state = container->tiling_state;
  assert(tiling_state);

  tiling_state->active_toplevel_container = container;
}
