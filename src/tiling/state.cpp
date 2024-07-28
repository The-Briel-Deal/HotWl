#include "state.hpp"
#include "tiling/container.hpp"
#include <cassert>
#include <memory>
#include <server.hpp>

/* TODO: Remove.
 * DEPRECATED: Moving to this being the containers responsibility.
 * Flips tiling states split_dir */
void GfTilingState::flip_split_direction() {
  if (this->split_dir == GFWL_SPLIT_DIR_HORI) {
    this->split_dir = GFWL_SPLIT_DIR_VERT;
  } else {
    this->split_dir = GFWL_SPLIT_DIR_HORI;
  }
}

/* Once we make the call to the appropriate container, its up to the container
 * to put it in the right place. */
std::weak_ptr<GfContainer> GfTilingState::insert(GfToplevel* toplevel) {
  /* Ideally insert into the active container in this tiling state. */
  // If focus stack not empty or expired insert in that container.
  if (!g_Server.active_toplevel_container.empty() &&
      !g_Server.active_toplevel_container.front().expired() &&
      g_Server.active_toplevel_container.front()
              .lock()
              ->tiling_state.lock()
              .get() == this) {
    return g_Server.active_toplevel_container.front().lock()->insert(
        toplevel);
  }
  return this->root->insert(toplevel);
}
