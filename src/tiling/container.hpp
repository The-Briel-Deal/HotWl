#pragma once
#include "state.hpp"
#include "xdg_shell.hpp"
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
  explicit GfContainer(const gfwl_toplevel *toplevel, const gfwl_server &server,
                       std::weak_ptr<GfContainer> parent_container,
                       const gfwl_container_type e_type,
                       std::weak_ptr<GfTilingState> tiling_state,
                       const bool is_root)
      : toplevel(toplevel), server(server), parent_container(parent_container),
        e_type(e_type), is_root(is_root){};
  // Member Functions
  std::weak_ptr<GfContainer> insert_child(gfwl_toplevel *toplevel);
  std::vector<std::weak_ptr<GfContainer>> get_top_level_container_list();
  gfwl_split_direction get_split_dir();
  void vert_split_containers();
  void hori_split_containers();
  void split_containers();
  void parse_containers();
  void close();

  // Descriptors of this container
  const gfwl_container_type e_type = GFWL_CONTAINER_UNKNOWN;
  const bool is_root = false;

  // The Container Above and Below this one in the tiling tree
  std::weak_ptr<GfContainer> parent_container;
  std::vector<std::shared_ptr<GfContainer>> child_containers;

  // The Dimensions of a container
  wlr_box box = {.x = 0, .y = 0, .width = 0, .height = 0};

  // Tiling State of The Current Output
  std::weak_ptr<GfTilingState> tiling_state;

  // References to the associated toplevel and server
  const gfwl_toplevel *toplevel;
  const gfwl_server &server;
};

void set_container_box(std::weak_ptr<GfContainer> toplevel, struct wlr_box box);

void set_focused_toplevel_container(std::weak_ptr<GfContainer> container);
