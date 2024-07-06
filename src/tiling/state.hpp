#pragma once

enum gfwl_split_direction {
  GFWL_SPLIT_DIR_UNKNOWN = 0,
  GFWL_SPLIT_DIR_HORI = 1,
  GFWL_SPLIT_DIR_VERT = 2
};

#include "container.hpp"
#include "xdg_shell.hpp"

struct gfwl_tiling_state {
public:
  void insert(gfwl_container *container);
  void insert(gfwl_toplevel *toplevel);

  void insert_child_container(struct gfwl_container *parent,
                              struct gfwl_container *child);
  void new_vert_split_container(struct gfwl_container *new_container,
                                struct gfwl_container *focused_container);
  void new_hori_split_container(struct gfwl_container *new_container,
                                struct gfwl_container *focused_container);
  void flip_split_direction();
  struct gfwl_container *root;
  struct gfwl_container *active_toplevel_container;
  enum gfwl_split_direction split_dir;

private:
  struct gfwl_server *server;
};
