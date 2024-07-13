#include "tiling/container.hpp"
#include <includes.hpp>
#include <memory>
#include <output.hpp>
#include <scene.hpp>
#include <server.hpp>
#include <xdg_shell.hpp>

std::shared_ptr<GfContainer>
create_parent_container(std::shared_ptr<GfContainer> child_container,
                        enum gfwl_container_type type) {
  // Just making sure I'm never passing unknown.
  assert(type != GFWL_CONTAINER_UNKNOWN);

  struct std::shared_ptr<GfContainer> parent_container =
      std::make_shared<GfContainer>(false, nullptr, type, nullptr,
                                    child_container->server, nullptr);
  parent_container->e_type = type;
  parent_container->server = child_container->server;
  parent_container->tiling_state = child_container->tiling_state;
  child_container->parent_container = parent_container;
  parent_container->child_containers.push_back(child_container);
  return parent_container;
}

std::shared_ptr<GfContainer>
create_container_from_toplevel(struct gfwl_toplevel *toplevel) {
  std::shared_ptr<GfContainer> container =
      std::make_shared<GfContainer>(false, nullptr, GFWL_CONTAINER_TOPLEVEL,
                                    nullptr, toplevel->server, toplevel);

  toplevel->parent_container = container;

  return container;
}

// Only Tested with toplevel_containers.

void set_focused_toplevel_container(std::shared_ptr<GfContainer> container) {
  assert(container);
  struct std::shared_ptr<GfTilingState> tiling_state = container->tiling_state;
  assert(tiling_state);

  tiling_state->active_toplevel_container = container;
  tiling_state->server->active_toplevel_container = container;
}
