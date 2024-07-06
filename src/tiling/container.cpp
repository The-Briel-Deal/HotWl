#include "container.hpp"
#include "state.hpp"

enum gfwl_split_direction
gfwl_container::get_split_dir() {
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
