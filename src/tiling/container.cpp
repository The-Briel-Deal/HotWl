#include "container.hpp"
#include "state.hpp"

GfContainer::GfContainer(bool root, gfwl_tiling_state *state,
                         gfwl_container_type type,
                         std::shared_ptr<GfContainer> parent,
                         gfwl_server *server, gfwl_toplevel *toplevel) {
  this->is_root = root;
  this->tiling_state = state;
  this->e_type = type;
  this->parent_container = parent;
  this->server = server;
  this->toplevel = toplevel;
};

enum gfwl_split_direction GfContainer::get_split_dir() {
  switch (this->e_type) {
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
