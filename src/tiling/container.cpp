#include "state.hpp"
#include "wlr/util/box.h"
#include <cassert>
#include <includes.hpp>
#include <memory>
#include <output.hpp>
#include <server.hpp>
#include <xdg_shell.hpp>

GfContainerToplevel::~GfContainerToplevel() {
  wlr_xdg_toplevel_send_close(this->toplevel->xdg_toplevel);
  this->tiling_state.lock()->root->parse_containers();
}

std::weak_ptr<GfContainer> GfContainerRoot::insert(gfwl_toplevel* to_insert) {
  return this->insert_child_in_split(to_insert, GFWL_CONTAINER_HSPLIT);
}

void GfContainerRoot::set_to_output_size() {
  std::shared_ptr<gfwl_output> output = this->tiling_state.lock()->output;
  this->box.x                         = output->scene_output->x;
  this->box.y                         = output->scene_output->y;
  this->box.width                     = output->wlr_output->width;
  this->box.height                    = output->wlr_output->height;
}

/* If we are in the root we need to set the root container to the size of the
 * output. */
void GfContainerRoot::parse_containers() {
  this->set_to_output_size();
  this->split_containers();
  this->parse_children();
}

// But for toplevels, we also need to set the toplevel's pos and size.
void GfContainerToplevel::set_container_box(struct wlr_box box_in) {
  this->box = box_in;

  auto xdg_toplevel = this->toplevel->xdg_toplevel;
  wlr_xdg_toplevel_set_size(xdg_toplevel, box_in.width, box_in.height);

  auto scene_tree = static_cast<wlr_scene_tree*>(xdg_toplevel->base->data);
  wlr_scene_node_set_position(&scene_tree->node, box_in.x, box_in.y);
}
