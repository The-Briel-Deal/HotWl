#pragma once
#include "state.hpp"
#include <includes.hpp>

enum gfwl_container_type {
  GFWL_CONTAINER_UNKNOWN = 0,
  GFWL_CONTAINER_HSPLIT = 1,
  GFWL_CONTAINER_VSPLIT = 2,
  GFWL_CONTAINER_TOPLEVEL = 3,
  GFWL_CONTAINER_ROOT = 4,
};

struct gfwl_container *
create_parent_container(struct gfwl_container *child_container,
                        enum gfwl_container_type type);

struct gfwl_container {
public:
  enum gfwl_split_direction get_split_dir();
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

void hori_split_containers(struct gfwl_container *container);

void vert_split_containers(struct gfwl_container *container);

void set_container_box(struct gfwl_container *toplevel, struct wlr_box box);

void parse_containers(struct gfwl_container *container);

void set_focused_toplevel_container(struct gfwl_container *container);

struct gfwl_container *
create_container_from_toplevel(struct gfwl_toplevel *toplevel);
