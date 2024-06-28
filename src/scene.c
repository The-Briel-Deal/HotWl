#include "wlr/util/log.h"
#include "xdg-shell-protocol.h"
#include <assert.h>
#include <layer_shell.h>
#include <output.h>
#include <scene.h>
#include <server.h>
#include <stdlib.h>
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
  wlr_log(WLR_INFO, "Vert Split=%i", server->split_dir);
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
  wlr_log(WLR_INFO, "In parse container enum %i", container->e_type);
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
      wlr_log(WLR_INFO, "In parse container enum %i", cursor->e_type);
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
  wlr_log(WLR_INFO, "Am I being Hit?");

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
    wlr_log(WLR_INFO, "vert-split box\n \
        x: %i\n \
        y: %i\n \
        width: %i\n \
        height: %i\n",
            box.x, box.y, box.width, box.height);
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

struct gfwl_container *
create_parent_container(struct gfwl_container *child_container) {

  struct gfwl_container *parent_container =
      calloc(1, sizeof(*parent_container));
  parent_container->e_type = GFWL_CONTAINER_VSPLIT;
  parent_container->server = child_container->server;
  child_container->parent_container = parent_container;
  wl_list_init(&parent_container->child_containers);
  wl_list_insert(&parent_container->child_containers, &child_container->link);
  return parent_container;
}

struct gfwl_container *
create_container_from_toplevel(struct gfwl_toplevel *toplevel) {
  struct gfwl_container *container = calloc(1, sizeof(*container));

  container->e_type = GFWL_CONTAINER_TOPLEVEL;
  container->toplevel = toplevel;
  container->server = toplevel->server;
  toplevel->parent_container = container;

  return container;
}

// Only Tested with toplevel_containers.
void new_vert_split_container(struct gfwl_container *new_container,
                              struct gfwl_container *focused_container) {
  assert(focused_container);

  struct gfwl_container *vert_split_container =
      create_parent_container(new_container);
  wl_list_remove(&focused_container->link);
  wl_list_insert(&vert_split_container->child_containers,
                 &focused_container->link);
  assert(vert_split_container &&
         vert_split_container->e_type == GFWL_CONTAINER_VSPLIT);

  wl_list_insert(
      &focused_container->server->toplevel_root_container.child_containers,
      &vert_split_container->link);
}

void insert_child_container(struct gfwl_container *parent,
                            struct gfwl_container *child) {
  child->parent_container = parent;

  if (child->link.next)
    wl_list_remove(&child->link);
  wl_list_insert(&parent->child_containers, &child->link);
}
void add_to_tiling_layout(struct gfwl_toplevel *toplevel) {
  assert(toplevel);
  struct gfwl_server *server = toplevel->server;
  assert(server);
  struct gfwl_container *toplevel_container =
      create_container_from_toplevel(toplevel);
  assert(toplevel_container);

  // lf means last focused btw.
  struct gfwl_toplevel *lf_toplevel = NULL;
  struct gfwl_container *lft_container = NULL, *lftc_container = NULL;
  enum gfwl_split_direction focused_split_type = GFWL_SPLIT_DIR_UNKNOWN;

  if (server)
    lf_toplevel = server->last_focused_toplevel;
  if (lf_toplevel)
    lft_container = lf_toplevel->parent_container;
  if (lft_container)
    lftc_container = lft_container->parent_container;
  if (lftc_container) {
    if (lftc_container->e_type == GFWL_CONTAINER_HSPLIT)
      focused_split_type = GFWL_SPLIT_DIR_HORI;
    else if (lftc_container->e_type == GFWL_CONTAINER_VSPLIT)
      focused_split_type = GFWL_SPLIT_DIR_VERT;
  }
  // TODO: MAKE SURE TO SET PARENT CONTAINER ON ALL CONTAINERS.
  // DOING: CLEAN THIS GARBAGE UP LOL
  // TODO: FIX BUG WHERE YOU MAKE A HORI CONTAINER IN A SPLIT CONTAINER. I
  //       JUST COVERED IT UP WITH THE THIRD PART OF THE IF STATEMENT.
  // TODO: WM CRASHES WHEN THE FIRST CONTAINER IS SPLIT VERT.
  // TODO: Move the tiling code to its own func in scene.
  // TODO: Change lf_toplevel to lf container.
  // TODO: Create a tiling_state struct.

  // Add vert container to already vert split container.
  if (focused_split_type == GFWL_SPLIT_DIR_VERT &&
      server->split_dir == GFWL_SPLIT_DIR_VERT) {
    insert_child_container(lftc_container, toplevel_container);
  } else if (server->split_dir == GFWL_SPLIT_DIR_VERT) {
    new_vert_split_container(toplevel_container, lft_container);
  }
  // Normal Horizontal Mode, This Is Only Like This Due To Lazy Gabriels Not
  // Actually Adding Horizontal Split Containers.
  else {
    wl_list_insert(&server->toplevel_root_container.child_containers,
                   &toplevel_container->link);
  }

  parse_containers(&server->toplevel_root_container);
}
