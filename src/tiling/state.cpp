#include "state.hpp"
#include "tiling/container.hpp"
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
  std::shared_ptr<GfContainer> focused_container;
  // If focus stack not empty or expired insert in that container.
  if (!this->server->active_toplevel_container.empty() &&
      !this->server->active_toplevel_container.front().expired() &&
      this->server->active_toplevel_container.front()
              .lock()
              ->tiling_state.lock()
              .get() == this) {
    focused_container = this->server->active_toplevel_container.front().lock();
  } else {
    focused_container = this->root;
  }

  return focused_container->insert(toplevel);
}
