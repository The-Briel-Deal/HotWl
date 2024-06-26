#include "wlr/util/log.h"
#include "xdg-shell-protocol.h"
#include <assert.h>
#include <layer_shell.h>
#include <output.h>
#include <scene.h>
#include <server.h>
#include <sys/types.h>
#include <wayland-util.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/box.h>
#include <xdg_shell.h>

void flip_split_direction(struct gfwl_server *server) {
  if (server->split_dir == GFWL_SPLIT_DIR_HORI)
    server->split_dir = GFWL_SPLIT_DIR_VERT;
  else
    server->split_dir = GFWL_SPLIT_DIR_HORI;
  wlr_log(WLR_INFO, "Vert Split=%b", server->split_dir);
}

static u_int16_t count_toplevel_containers(struct wl_list *toplevels) {
  assert(toplevels);
  if (!toplevels)
    return 0;
  struct gfwl_container *curr_toplevel;
  u_int16_t count = 0;
  wl_list_for_each(curr_toplevel, toplevels, link) { count += 1; }
  return count;
}

void parse_containers(struct gfwl_container *container) {
  if (container->is_root) {
    // Get output.
    struct gfwl_output *output =
        wl_container_of(container->server->outputs.next, output, link);
    container->box.width = output->wlr_output->width;
    container->box.height = output->wlr_output->height;
  }
  wlr_log(WLR_DEBUG, "In parse container");
  if (container->e_type == GFWL_CONTAINER_HSPLIT)
    hori_split_toplevels(container, container->server);
  if (container->e_type == GFWL_CONTAINER_VSPLIT)
    vert_split_toplevels(container, container->server);

  struct gfwl_container *cursor;
  struct wl_list *head = &container->child_containers;
  wl_list_for_each(cursor, head, link) {
    if (cursor->e_type == GFWL_CONTAINER_HSPLIT ||
        cursor->e_type == GFWL_CONTAINER_VSPLIT) {
      parse_containers(cursor);
    }
  }
}

// Change this to get output size from the parent.
void vert_split_toplevels(struct gfwl_container *container_in,
                          struct gfwl_server *server) {
  struct wl_list *toplevel_containers = &container_in->child_containers;
  // Get count.
  u_int16_t count = count_toplevel_containers(toplevel_containers);
  if (count == 0) {
    wlr_log(WLR_DEBUG, "You probably don't want to divide by 0");
    return;
  }

  // Get output.
  struct gfwl_output *output =
      wl_container_of(server->outputs.next, output, link);

  // Get Width and Height.
  u_int32_t width = container_in->box.width;
  u_int32_t height = container_in->box.height;

  // Get per_win_width.
  u_int32_t per_win_height = height / count;

  // Set all sizes. (recycling count for the index)
  count = 0;
  struct gfwl_container *curr_toplevel_container;
  wl_list_for_each(curr_toplevel_container, toplevel_containers, link) {
    const struct wlr_box box = {.x = container_in->box.x,
                                .y = per_win_height * count,
                                .width = width,
                                .height = per_win_height};
    set_container_box(curr_toplevel_container, box);
    count += 1;
  }
}
void hori_split_toplevels(struct gfwl_container *container_in,
                          struct gfwl_server *server) {
  struct wl_list *toplevel_containers = &container_in->child_containers;
  // Get count.
  u_int16_t count = count_toplevel_containers(toplevel_containers);
  if (count == 0) {
    wlr_log(WLR_DEBUG, "You probably don't want to divide by 0");
    return;
  }

  // Get output.
  struct gfwl_output *output =
      wl_container_of(server->outputs.next, output, link);

  // Get Width and Height.
  u_int32_t width = container_in->box.width;
  u_int32_t height = container_in->box.height;

  // Get per_win_width.
  u_int32_t per_win_width = width / count;

  // Set all sizes. (recycling count for the index)
  count = 0;
  struct gfwl_container *curr_toplevel_container;
  wl_list_for_each(curr_toplevel_container, toplevel_containers, link) {
    const struct wlr_box box = {.x = count * per_win_width,
                                .y = 0,
                                .width = per_win_width,
                                .height = height};
    set_container_box(curr_toplevel_container, box);
    count += 1;
  }
}

// Only supports toplevels for now.
void set_container_box(struct gfwl_container *container, struct wlr_box box) {
  container->box = box;
  if (container->e_type == GFWL_CONTAINER_TOPLEVEL) {
    struct wlr_xdg_toplevel *toplevel = container->toplevel->xdg_toplevel;
    // Set the size.
    wlr_xdg_toplevel_set_size(toplevel, box.width, box.height);
    // Set the position.
    struct wlr_scene_tree *scene_tree = toplevel->base->data;
    scene_tree->node.x = box.x;
    scene_tree->node.y = box.y;
  }
};
