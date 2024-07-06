#include "scene.hpp"
#include "wlr/util/log.h"
#include "xdg_shell.hpp"
#include <assert.h>
#include <stdlib.h>
#include <tiling_focus.hpp>
#include <wayland-util.h>
// TODO: Make tiling_state object oriented in cpp.
static struct gfwl_container *
get_container_in_dir(enum gfwl_tiling_focus_direction dir,
                     struct gfwl_tiling_state *state);

static bool focus_and_warp_to_container(struct gfwl_container *contaiener,
                                        struct gfwl_tiling_state *state);

static struct gfwl_point get_container_origin(struct gfwl_container *container);

static struct wl_list *get_toplevel_container_list(struct gfwl_container *head,
                                                   struct wl_list *list);

static struct gfwl_container *
find_closest_to_origin_in_dir(struct gfwl_point origin,
                              struct wl_list *toplevel_container_list,
                              enum gfwl_tiling_focus_direction dir);

bool tiling_focus_move_in_dir(enum gfwl_tiling_focus_direction dir,
                              struct gfwl_tiling_state *state) {
  // Get Container In The Specified Direction.
  struct gfwl_container *container_to_focus = get_container_in_dir(dir, state);
  if (container_to_focus == NULL) {
    return false;
  }

  // Focus container.
  if (focus_and_warp_to_container(container_to_focus, state) != true) {
    return false;
  };
  return true;
}

static struct gfwl_container *
get_container_in_dir(enum gfwl_tiling_focus_direction dir,
                     struct gfwl_tiling_state *state) {
  assert(state);

  struct gfwl_container *curr_focused = state->active_toplevel_container;
  assert(curr_focused);

  // Get Currently Focused Container Origin, X and Y position.
  struct gfwl_point curr_focused_origin = get_container_origin(curr_focused);

  // Get List of all Toplevel Containers.
  struct wl_list toplevel_container_list;
  wl_list_init(&toplevel_container_list);
  get_toplevel_container_list(state->root, &toplevel_container_list);

  assert(false);
  // Iterate through all Toplevel Containers, if we are going left, we
  // should look for a container where the curr focused container's y value
  // is within (new_focused_y < curr_focused_center_y &&
  //         new_focused_y + new_focusedheight > curr_focused_center_y &&
  //         new_focused_x < curr_focused_center_x)
  // With this container save it along with the distance
  //        (new_focused_x - curr_focused_center_x)
  // And return the one with the closest distance.
  struct gfwl_container *to_focus = find_closest_to_origin_in_dir(
      curr_focused_origin, &toplevel_container_list, dir);

  // Return found container.
  if (to_focus) {
    assert(to_focus->e_type == GFWL_CONTAINER_TOPLEVEL);
    return to_focus;
  }

  return NULL;
}

static bool focus_and_warp_to_container(struct gfwl_container *container,
                                        struct gfwl_tiling_state *state) {
  assert(container);
  assert(container->e_type == GFWL_CONTAINER_TOPLEVEL);

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
  return true;
}

static struct gfwl_point
get_container_origin(struct gfwl_container *container) {
  struct wlr_box box = container->box;
  struct gfwl_point center = {.x = (box.width / 2) + box.x,
                              .y = (box.height / 2) + box.y};
  return center;
}

static struct wl_list *get_toplevel_container_list(struct gfwl_container *head,
                                                   struct wl_list *list) {
  if (head->e_type == GFWL_CONTAINER_TOPLEVEL) {
    wl_list_insert(list, &head->link);
    return list;
  }
  struct gfwl_container *curs = NULL;
  struct wl_list *child_containers = head->child_containers.next;
  wl_list_for_each(curs, child_containers, link) {
    get_toplevel_container_list(curs, list);
  }
  return list;
}

static struct gfwl_container *
find_closest_to_origin_in_dir(struct gfwl_point origin,
                              struct wl_list *toplevel_container_list,
                              enum gfwl_tiling_focus_direction dir) {

  return NULL;
}

void gfwl_tiling_state::flip_split_direction() {
  if (this->split_dir == GFWL_SPLIT_DIR_HORI)
    this->split_dir = GFWL_SPLIT_DIR_VERT;
  else
    this->split_dir = GFWL_SPLIT_DIR_HORI;
}

enum gfwl_split_direction get_split_dir(struct gfwl_container *container) {
  if (container == NULL)
    return GFWL_SPLIT_DIR_UNKNOWN;

  switch (container->e_type) {
  case GFWL_CONTAINER_VSPLIT:
    return GFWL_SPLIT_DIR_VERT;
  case GFWL_CONTAINER_HSPLIT:
    return GFWL_SPLIT_DIR_HORI;
  case GFWL_CONTAINER_ROOT:
    return GFWL_SPLIT_DIR_HORI;
  default:
    return GFWL_SPLIT_DIR_UNKNOWN;
  }
}

void insert_child_container(struct gfwl_container *parent,
                            struct gfwl_container *child) {
  child->parent_container = parent;

  if (child->link.next)
    wl_list_remove(&child->link);
  wl_list_insert(&parent->child_containers, &child->link);
}

void new_vert_split_container(struct gfwl_container *new_container,
                              struct gfwl_container *focused_container) {
  assert(new_container);
  struct gfwl_container *fc_parent;

  struct gfwl_container *split_container =
      create_parent_container(new_container, GFWL_CONTAINER_VSPLIT);
  if (focused_container) {
    fc_parent = focused_container->parent_container;
    assert(fc_parent);
    if (focused_container->link.next)
      wl_list_remove(&focused_container->link);
    wl_list_insert(&split_container->child_containers,
                   &focused_container->link);
  }
  assert(split_container && split_container->e_type == GFWL_CONTAINER_VSPLIT);

  if (fc_parent)
    wl_list_insert(&fc_parent->child_containers, &split_container->link);
  else
    wl_list_insert(&new_container->tiling_state->root->child_containers,
                   &split_container->link);
}

// I think these need to be changed for nesting.
void new_hori_split_container(struct gfwl_container *new_container,
                              struct gfwl_container *focused_container) {
  assert(new_container);
  struct gfwl_container *fc_parent = NULL;

  struct gfwl_container *split_container =
      create_parent_container(new_container, GFWL_CONTAINER_HSPLIT);
  if (focused_container) {
    fc_parent = focused_container->parent_container;
    assert(fc_parent);
    if (focused_container->link.next)
      wl_list_remove(&focused_container->link);
    wl_list_insert(&split_container->child_containers,
                   &focused_container->link);
  }
  assert(split_container && split_container->e_type == GFWL_CONTAINER_HSPLIT);

  if (fc_parent)
    wl_list_insert(&fc_parent->child_containers, &split_container->link);
  else
    wl_list_insert(&new_container->tiling_state->root->child_containers,
                   &split_container->link);
}
void gfwl_tiling_state::insert(gfwl_container *container) {
  container->tiling_state = this;

  // lf means last focused btw.
  struct gfwl_container *lft_container = NULL, *lftc_container = NULL;
  enum gfwl_split_direction split_dir = GFWL_SPLIT_DIR_UNKNOWN;

  // TODO: Come up with better names for this.
  lft_container = this->active_toplevel_container;
  if (lft_container)
    lftc_container = lft_container->parent_container;
  if (lftc_container) {
    split_dir = get_split_dir(lftc_container);
  }

  switch (this->split_dir) {
  case GFWL_SPLIT_DIR_VERT:
    if (split_dir == GFWL_SPLIT_DIR_VERT)
      insert_child_container(lftc_container, container);
    else
      new_vert_split_container(container, lft_container);
    break;
  case GFWL_SPLIT_DIR_HORI:
    if (split_dir == GFWL_SPLIT_DIR_HORI)
      insert_child_container(lftc_container, container);
    else
      new_hori_split_container(container, lft_container);
    break;

  case GFWL_SPLIT_DIR_UNKNOWN:
    wlr_log(WLR_ERROR, "Split dir shouldn't ever be unknown on a toplevel.");
    break;
  }
  parse_containers(this->root);
}
