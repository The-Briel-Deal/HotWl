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

class GfContainer : public std::enable_shared_from_this<GfContainer> {
public:
  GfContainer(bool root, gfwl_tiling_state *state, gfwl_container_type type,
              std::shared_ptr<GfContainer> parent, gfwl_server *server,
              gfwl_toplevel *toplevel);

  // Member Functions
  gfwl_split_direction get_split_dir();
  void parse_containers();
  void split_containers();
  void vert_split_containers();
  void hori_split_containers();
  std::vector<std::shared_ptr<GfContainer>> get_top_level_container_list();

  // The Container Above and Below this one in the tiling tree
  std::shared_ptr<GfContainer> parent_container = NULL;
  std::vector<std::shared_ptr<GfContainer>> child_containers;

  // Descriptors of this container
  gfwl_container_type e_type = GFWL_CONTAINER_UNKNOWN;
  bool is_root = false;

  // The Dimensions of a container
  wlr_box box = {.x = 0, .y = 0, .width = 0, .height = 0};

  // Some relevant state
  gfwl_tiling_state *tiling_state = NULL;
  gfwl_toplevel *toplevel = NULL;
  gfwl_server *server = NULL;
};

void set_container_box(std::shared_ptr<GfContainer> toplevel,
                       struct wlr_box box);

void set_focused_toplevel_container(std::shared_ptr<GfContainer> container);

std::shared_ptr<GfContainer>
create_container_from_toplevel(struct gfwl_toplevel *toplevel);

std::shared_ptr<GfContainer>
create_parent_container(std::shared_ptr<GfContainer> child_container,
                        enum gfwl_container_type type);
