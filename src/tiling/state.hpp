#pragma once

#include <memory>

enum gfwl_split_direction {
  GFWL_SPLIT_DIR_UNKNOWN = 0,
  GFWL_SPLIT_DIR_HORI = 1,
  GFWL_SPLIT_DIR_VERT = 2
};

struct gfwl_output;
struct GfContainer;
struct gfwl_toplevel;

struct GfTilingState : public std::enable_shared_from_this<GfTilingState> {
public:
  void insert(std::weak_ptr<GfContainer> container);
  void insert(gfwl_toplevel *toplevel);

  void insert_child_container(std::shared_ptr<GfContainer> parent,
                              std::shared_ptr<GfContainer> child);
  void new_vert_split_container(std::shared_ptr<GfContainer> new_container,
                                std::shared_ptr<GfContainer> focused_container);
  void new_hori_split_container(std::shared_ptr<GfContainer> new_container,
                                std::shared_ptr<GfContainer> focused_container);
  void flip_split_direction();
  void reparent_container(std::shared_ptr<GfContainer> parent,
                          std::shared_ptr<GfContainer> child);
  std::shared_ptr<GfContainer> root;
  std::weak_ptr<GfContainer> active_toplevel_container;
  gfwl_split_direction split_dir;
  std::shared_ptr<gfwl_output> output = NULL;
  struct gfwl_server *server;
};
