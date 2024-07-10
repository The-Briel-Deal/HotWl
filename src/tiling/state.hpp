#pragma once
#include <memory>
enum gfwl_split_direction {
  GFWL_SPLIT_DIR_UNKNOWN = 0,
  GFWL_SPLIT_DIR_HORI = 1,
  GFWL_SPLIT_DIR_VERT = 2
};

struct GfContainer;
struct gfwl_toplevel;

struct GfTilingState {
public:
  void insert(std::shared_ptr<GfContainer> container);
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
  std::shared_ptr<GfContainer> active_toplevel_container;
  gfwl_split_direction split_dir;

private:
  struct gfwl_server *server;
};
