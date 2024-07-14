#include "state.hpp"
#include <cassert>
#include <memory>
#include <server.hpp>

/* TODO: Remove.
 * DEPRECATED: Moving to this being the containers responsibility.
 * Flips tiling states split_dir */
void GfTilingState::flip_split_direction() {
  if (this->split_dir == GFWL_SPLIT_DIR_HORI)
    this->split_dir = GFWL_SPLIT_DIR_VERT;
  else
    this->split_dir = GFWL_SPLIT_DIR_HORI;
}

/* Once we make the call to the appropriate container, its up to the container
 * to put it in the right place. */
std::weak_ptr<GfContainer> GfTilingState::insert(gfwl_toplevel *toplevel) {
  /* Ideally insert into the active container in this tiling state. */
  auto focused_container = this->active_toplevel_container.lock();
  if (focused_container) {
    return focused_container->parent_container.lock()->insert_child(toplevel);
  }

  /* In some cases (like when the last focused has been freed, we will insert
   * in the root node. */
  return this->root->insert_child(toplevel);
}
