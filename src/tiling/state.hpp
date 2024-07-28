#pragma once

#include <memory>

enum gfwl_split_direction {
  GFWL_SPLIT_DIR_UNKNOWN = 0,
  GFWL_SPLIT_DIR_HORI    = 1,
  GFWL_SPLIT_DIR_VERT    = 2
};

struct gfwl_output;
class GfContainer;
class GfContainerRoot;
struct GfToplevel;

struct GfTilingState : public std::enable_shared_from_this<GfTilingState> {
public:
  /* Inserts a toplevel into the tiling state. */
  std::weak_ptr<GfContainer> insert(GfToplevel* toplevel);

  /* TODO: Remove */
  void                             flip_split_direction();

  std::shared_ptr<GfContainerRoot> root;
  gfwl_split_direction             split_dir;
  std::shared_ptr<gfwl_output>     output = nullptr;
};
