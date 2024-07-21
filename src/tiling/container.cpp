#include "container.hpp"
#include "state.hpp"
#include "wlr/util/box.h"
#include "wlr/util/log.h"
#include <algorithm>
#include <cassert>
#include <deque>
#include <includes.hpp>
#include <memory>
#include <output.hpp>
#include <server.hpp>
#include <vector>
#include <xdg_shell.hpp>

GfContainerToplevel::~GfContainerToplevel() {
  wlr_xdg_toplevel_send_close(this->toplevel->xdg_toplevel);
  this->tiling_state.lock()->root->parse_containers();
}

std::weak_ptr<GfContainer> GfContainer::insert(gfwl_toplevel* to_insert) {
  return this->insert_based_on_longer_dir(to_insert);
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

// The default behavior for all containers is to just set the containers box.
void GfContainer::set_container_box(struct wlr_box box_in) {
  this->box = box_in;
}

// But for toplevels, we also need to set the toplevel's pos and size.
void GfContainerToplevel::set_container_box(struct wlr_box box_in) {
  this->box = box_in;

  auto xdg_toplevel = this->toplevel->xdg_toplevel;
  wlr_xdg_toplevel_set_size(xdg_toplevel, box_in.width, box_in.height);

  auto scene_tree = static_cast<wlr_scene_tree*>(xdg_toplevel->base->data);
  wlr_scene_node_set_position(&scene_tree->node, box_in.x, box_in.y);
}

// Get A List of Toplevels below this Container node.
std::vector<std::weak_ptr<GfContainer>>
GfContainer::get_top_level_container_list() {
  std::vector<std::weak_ptr<GfContainer>> list;
  std::deque<std::weak_ptr<GfContainer>>  stack;

  for (auto output : server.outputs) {
    stack.push_back(output->tiling_state->root);
  }

  while (!stack.empty()) {
    auto curr_node = stack.back();
    stack.pop_back();
    for (auto child : curr_node.lock()->child_containers) {
      switch (child->e_type) {
        case GFWL_CONTAINER_TOPLEVEL: list.push_back(child); break;
        case GFWL_CONTAINER_HSPLIT: stack.push_back(child); break;
        case GFWL_CONTAINER_VSPLIT: stack.push_back(child); break;
        case GFWL_CONTAINER_ROOT: break;
        case GFWL_CONTAINER_UNKNOWN: break;
        default: break;
      }
    }
  }
  return list;
}
