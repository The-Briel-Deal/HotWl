#include "toplevel.hpp"

#include <xdg_shell.hpp>

#include "includes.hpp"
#include "wlr/util/box.h"
#include "tiling/container/root.hpp"
#include "tiling/state.hpp"
#include "wlr/types/wlr_xdg_shell.h"

GfContainerToplevel::~GfContainerToplevel() {
  wlr_xdg_toplevel_send_close(this->toplevel->xdg_toplevel);
  this->tiling_state.lock()->root->parse_containers();
}

// But for toplevels, we also need to set the toplevel's pos and size.
void GfContainerToplevel::set_container_box(struct wlr_box box_in) {
  this->box = box_in;

  auto* xdg_toplevel = this->toplevel->xdg_toplevel;
  wlr_xdg_toplevel_set_size(xdg_toplevel, box_in.width, box_in.height);

  auto* scene_tree = static_cast<wlr_scene_tree*>(xdg_toplevel->base->data);
  wlr_scene_node_set_position(&scene_tree->node, box_in.x, box_in.y);
}
