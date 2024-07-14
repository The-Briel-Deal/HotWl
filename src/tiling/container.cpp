#include "container.hpp"
#include "state.hpp"
#include <cassert>
#include <deque>
#include <includes.hpp>
#include <memory>
#include <output.hpp>
#include <server.hpp>
#include <vector>
#include <xdg_shell.hpp>

// Inserting the toplevel directly, returns a weak pointer to the new container.
std::weak_ptr<GfContainer> GfContainer::insert_child(gfwl_toplevel *toplevel) {
  auto toplevel_container =
      this->child_containers
          .emplace_back(std::make_shared<GfContainer>(
              toplevel, *toplevel->server, this->weak_from_this(),
              GFWL_CONTAINER_TOPLEVEL, this->tiling_state, false))
          ->weak_from_this();
  toplevel->parent_container = toplevel_container;
  return toplevel_container;
}

// Inserts a toplevel nested in a new split_container.
std::weak_ptr<GfContainer> GfContainer::insert_child_in_split(
    gfwl_toplevel *toplevel, enum gfwl_container_type split_container_type) {
  assert(split_container_type != GFWL_CONTAINER_TOPLEVEL);

  return this->child_containers
      .emplace_back(std::make_shared<GfContainer>(
          toplevel, *toplevel->server, this->weak_from_this(),
          split_container_type, this->tiling_state, false))
      ->insert_child(toplevel);
}

void GfContainer::set_focused_toplevel_container() {

  auto tiling_state = this->tiling_state.lock();
  if (tiling_state) {
    tiling_state->active_toplevel_container = this->weak_from_this();
    tiling_state->server->active_toplevel_container = this->weak_from_this();
  }
}

void GfContainer::close() {
  // I need to rethink ownership of containers.
}

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

// TODO: Replace duplicate parts with generalized helpers.
// Change this to get output size from the parent.
void GfContainer::vert_split_containers() {
  // Get count.
  int count = this->child_containers.size();
  if (count == 0) {
    wlr_log(WLR_DEBUG, "You probably don't want to divide by 0");
    return;
  }

  int width = this->box.width;
  int height = this->box.height;

  int per_win_height = height / count;

  // Set all sizes. (recycling count for the index)
  count = 0;
  for (auto curr : this->child_containers) {
    wlr_box box = {.x = this->box.x,
                   .y = this->box.y + (per_win_height * count),
                   .width = width,
                   .height = per_win_height};
    set_container_box(curr, box);
    count += 1;
  }
}

void GfContainer::hori_split_containers() {
  // Get count.
  int count = this->child_containers.size();
  if (count == 0) {
    wlr_log(WLR_DEBUG, "You probably don't want to divide by 0");
    return;
  }
  // Get Width and Height.
  int width = this->box.width;
  int height = this->box.height;

  // Get per_win_width.
  int per_win_width = width / count;

  // Set all sizes. (recycling count for the index)
  count = 0;
  for (auto curr : this->child_containers) {
    wlr_box box = {.x = this->box.x + (count * per_win_width),
                   .y = this->box.y,
                   .width = per_win_width,
                   .height = height};
    set_container_box(curr, box);
    count += 1;
  }
}

void GfContainer::split_containers() {
  switch (this->get_split_dir()) {
  case GFWL_SPLIT_DIR_HORI:
    this->hori_split_containers();
    break;
  case GFWL_SPLIT_DIR_VERT:
    this->vert_split_containers();
    break;
  default:
    break;
  }
}

void GfContainer::parse_containers() {
  // Get output if we're at the root.
  if (this->is_root) {
    // TODO: This also probably shouldn't just grab the first output.
    std::shared_ptr<gfwl_output> output = this->tiling_state.lock()->output;
    assert(output);
    this->box.x = output->scene_output->x;
    this->box.y = output->scene_output->y;
    this->box.width = output->wlr_output->width;
    this->box.height = output->wlr_output->height;
  }
  this->split_containers();
  for (auto child : this->child_containers) {
    if (child->e_type == GFWL_CONTAINER_HSPLIT ||
        child->e_type == GFWL_CONTAINER_VSPLIT) {
      child->parse_containers();
    }
  }
}

// Only supports toplevels for now.
void set_container_box(std::weak_ptr<GfContainer> container,
                       struct wlr_box box) {
  container.lock()->box = box;
  if (container.lock()->e_type == GFWL_CONTAINER_TOPLEVEL) {
    struct wlr_xdg_toplevel *toplevel =
        container.lock()->toplevel->xdg_toplevel;
    // Set the size.
    wlr_xdg_toplevel_set_size(toplevel, box.width, box.height);
    // Set the position.
    struct wlr_scene_tree *scene_tree = (wlr_scene_tree *)toplevel->base->data;
    wlr_scene_node_set_position(&scene_tree->node, box.x, box.y);
  }
};

// Get A List of Toplevels below this Container node.
std::vector<std::weak_ptr<GfContainer>>
GfContainer::get_top_level_container_list() {
  std::vector<std::weak_ptr<GfContainer>> list;
  std::deque<std::weak_ptr<GfContainer>> stack;
  // I need to figure out what the heck the difference between emplace and push
  // is. I also need to figure out how to get my lost shared_ptr back ):
  auto server = this->server;

  for (auto output : server.outputs) {
    stack.push_back(output->tiling_state->root);
  }

  while (!stack.empty()) {
    auto curr_node = stack.back();
    stack.pop_back();
    for (auto child : curr_node.lock()->child_containers) {
      switch (child->e_type) {
      case GFWL_CONTAINER_TOPLEVEL:
        list.push_back(child);
        break;
      case GFWL_CONTAINER_HSPLIT:
        stack.push_back(child);
        break;
      case GFWL_CONTAINER_VSPLIT:
        stack.push_back(child);
        break;
      default:
        break;
      }
    }
  }
  return list;
}
