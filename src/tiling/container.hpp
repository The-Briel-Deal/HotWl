#pragma once
#include "state.hpp"
#include <includes.hpp>
#include <memory>
#include <vector>

enum gfwl_container_type {
  GFWL_CONTAINER_UNKNOWN = 0,
  GFWL_CONTAINER_HSPLIT = 1,
  GFWL_CONTAINER_VSPLIT = 2,
  GFWL_CONTAINER_TOPLEVEL = 3,
  GFWL_CONTAINER_ROOT = 4,
};

struct GfContainer {
public:
  gfwl_split_direction get_split_dir();
  struct gfwl_tiling_state *tiling_state;
  gfwl_container_type e_type;
  struct std::shared_ptr<GfContainer> parent_container;
  bool is_root;
  struct wlr_box box;

  struct gfwl_toplevel *toplevel;
  struct gfwl_server *server;

  std::vector<std::shared_ptr<GfContainer>> child_containers;

  GfContainer(bool root, gfwl_tiling_state *state, gfwl_container_type type,
              std::shared_ptr<GfContainer> parent, gfwl_server *server,
              gfwl_toplevel *toplevel);
};

void hori_split_containers(std::shared_ptr<GfContainer> container);

void vert_split_containers(std::shared_ptr<GfContainer> container);

void set_container_box(std::shared_ptr<GfContainer> toplevel,
                       struct wlr_box box);

void parse_containers(std::shared_ptr<GfContainer> container);

void set_focused_toplevel_container(std::shared_ptr<GfContainer> container);

std::shared_ptr<GfContainer>
create_container_from_toplevel(struct gfwl_toplevel *toplevel);

std::shared_ptr<GfContainer>
create_parent_container(std::shared_ptr<GfContainer> child_container,
                        enum gfwl_container_type type);
