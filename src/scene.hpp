#pragma once

#ifdef __cplusplus
#include <includes.hpp>
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

struct gfwl_server;

struct gfwl_scene {
  // A scene wrapper so I can grab certain parts of the tree easily.
  struct {
    struct wlr_scene_tree *base;
    struct wlr_scene_tree *top;
  } layer;
  struct wlr_scene *root;
};

enum gfwl_split_direction {
  GFWL_SPLIT_DIR_UNKNOWN = 0,
  GFWL_SPLIT_DIR_HORI = 1,
  GFWL_SPLIT_DIR_VERT = 2
};

enum gfwl_container_type {
  GFWL_CONTAINER_UNKNOWN = 0,
  GFWL_CONTAINER_HSPLIT = 1,
  GFWL_CONTAINER_VSPLIT = 2,
  GFWL_CONTAINER_TOPLEVEL = 3,
  GFWL_CONTAINER_ROOT = 4,
};

struct gfwl_container {
  struct gfwl_tiling_state *tiling_state;
  enum gfwl_container_type e_type;
  struct gfwl_container *parent_container;
  bool is_root;
  struct wlr_box box;

  struct gfwl_toplevel *toplevel;
  struct gfwl_server *server;

  struct wl_list child_containers;
  struct wl_list link;
};

struct gfwl_tiling_state {
  struct gfwl_server *server;
  struct gfwl_container *root;
  struct gfwl_container *active_toplevel_container;
  enum gfwl_split_direction split_dir;
};

EXTERNC void flip_split_direction(struct gfwl_tiling_state *tiling_state);

EXTERNC void hori_split_containers(struct gfwl_container *container);

EXTERNC void vert_split_containers(struct gfwl_container *container);

EXTERNC void set_container_box(struct gfwl_container *toplevel,
                               struct wlr_box box);

EXTERNC void parse_containers(struct gfwl_container *container);

EXTERNC struct gfwl_container *
create_parent_container(struct gfwl_container *child_container,
                        enum gfwl_container_type type);

EXTERNC void add_to_tiling_layout(struct gfwl_toplevel *toplevel_to_add,
                                  struct gfwl_tiling_state *tiling_state);
EXTERNC void set_focused_toplevel_container(struct gfwl_container *container);