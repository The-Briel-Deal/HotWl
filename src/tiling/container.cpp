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

void focus_next_in_stack(std::weak_ptr<GfContainer> curr,
                         std::deque<std::weak_ptr<GfContainer>> stack) {
  while (!stack.empty()) {
    if (!stack.front().expired() && stack.front().lock() != curr.lock() &&
        stack.front().lock()->e_type == GFWL_CONTAINER_TOPLEVEL) {
      focus_toplevel(
          stack.front().lock()->toplevel,
          stack.front().lock()->toplevel->xdg_toplevel->base->surface);
      return;
    }
    stack.pop_front();
  }
}
GfContainer::~GfContainer() {
  auto tl = this->toplevel;
  if (tl && this->e_type == GFWL_CONTAINER_TOPLEVEL) {
    wlr_xdg_toplevel_send_close(tl->xdg_toplevel);
    this->tiling_state.lock()->root->parse_containers();
  };
}

std::weak_ptr<GfContainer> GfContainer::insert(gfwl_toplevel *to_insert) {
  if (this->is_root)
    return this->insert_child_in_split(to_insert, GFWL_CONTAINER_HSPLIT);

  return this->insert_based_on_longer_dir(to_insert);
}

// Return const reference to containers box.
const wlr_box &GfContainer::get_box() { return this->box; }

// This is intended for toplevel containers.
std::weak_ptr<GfContainer>
GfContainer::insert_based_on_longer_dir(gfwl_toplevel *to_insert) {
  // TODO: I will likely have to make sure the container is inserted at the
  // right position.
  assert(this->e_type == GFWL_CONTAINER_TOPLEVEL);
  auto parent = this->parent_container.lock();
  auto split_dir_longer = this->get_split_dir_longer();
  std::weak_ptr<GfContainer> new_toplevel_container;

  switch (split_dir_longer) {
  case GFWL_SPLIT_DIR_HORI:
    new_toplevel_container = parent->insert_child_in_split(
        to_insert, this->weak_from_this(), GFWL_CONTAINER_HSPLIT);
    break;
  case GFWL_SPLIT_DIR_VERT:
    new_toplevel_container = parent->insert_child_in_split(
        to_insert, this->weak_from_this(), GFWL_CONTAINER_VSPLIT);
    break;
  case GFWL_SPLIT_DIR_UNKNOWN:
    assert(false); // Split dir should never be unknown.
    wlr_log(WLR_ERROR, "Split dir is unknown, this should not be the case.");
    break;
  default:
    assert(false); // Split dir should never be unknown.
    wlr_log(WLR_ERROR,
            "Split dir is an invalid enum value, this should not be the case.");
    break;
  }
  this->move_container_to(new_toplevel_container.lock()->parent_container);
  this->tiling_state.lock()->root->parse_containers();
  return this->parent_container;
}

void GfContainer::move_container_to(std::weak_ptr<GfContainer> new_parent) {
  auto this_locked = this->shared_from_this();
  auto &prev_parent_child_containers =
      this->parent_container.lock()->child_containers;

  prev_parent_child_containers.erase(
      std::find(prev_parent_child_containers.begin(),
                prev_parent_child_containers.end(), this_locked));

  new_parent.lock()->child_containers.insert(
      new_parent.lock()->child_containers.begin(), this_locked);
  this->parent_container = new_parent;
}

/* Insert directly after this container, returns a weak pointer to the new
 * container. */
std::weak_ptr<GfContainer>
GfContainer::insert_sibling(gfwl_toplevel *to_insert) {
  auto parent = parent_container.lock();
  assert(parent);
  /* Get position of this container in its parent. */
  auto pos =
      std::find(parent->child_containers.begin(),
                parent->child_containers.end(), this->shared_from_this());
  /* Emplace a new container before the afformentioned pos in the parent. */
  auto toplevel_container =
      parent->child_containers
          .emplace(pos,
                   std::make_shared<GfContainer>(
                       to_insert, *to_insert->server, parent->weak_from_this(),
                       GFWL_CONTAINER_TOPLEVEL, this->tiling_state, false))
          ->get()
          ->weak_from_this();

  /* Set the new container as the parent in the toplevel. */
  to_insert->parent_container = toplevel_container;
  this->tiling_state.lock()->root->parse_containers();

  return toplevel_container;
}

// Inserting the toplevel directly, returns a weak pointer to the new
// container.
std::weak_ptr<GfContainer> GfContainer::insert_child(gfwl_toplevel *to_insert) {
  auto toplevel_container =
      this->child_containers
          .emplace_back(std::make_shared<GfContainer>(
              to_insert, *to_insert->server, this->weak_from_this(),
              GFWL_CONTAINER_TOPLEVEL, this->tiling_state, false))
          ->weak_from_this();
  to_insert->parent_container = toplevel_container;
  // As an optimization down the road, I can try just parsing the changes
  // containers.
  this->tiling_state.lock()->root->parse_containers();
  return toplevel_container;
}

std::weak_ptr<GfContainer>
GfContainer::insert_child(gfwl_toplevel *to_insert,
                          std::weak_ptr<GfContainer> insert_before) {
  auto toplevel_container =
      this->child_containers
          .emplace(std::find(this->child_containers.begin(),
                             this->child_containers.end(),
                             insert_before.lock()),
                   std::make_shared<GfContainer>(
                       to_insert, *to_insert->server, this->weak_from_this(),
                       GFWL_CONTAINER_TOPLEVEL, this->tiling_state, false))
          ->get()
          ->weak_from_this();
  to_insert->parent_container = toplevel_container;
  // As an optimization down the road, I can try just parsing the changes
  // containers.
  this->tiling_state.lock()->root->parse_containers();
  return toplevel_container;
}

// Inserts a toplevel nested in a new split_container.
std::weak_ptr<GfContainer> GfContainer::insert_child_in_split(
    gfwl_toplevel *to_insert, enum gfwl_container_type split_container_type) {
  assert(split_container_type != GFWL_CONTAINER_TOPLEVEL);

  return this->child_containers
      .emplace_back(std::make_shared<GfContainer>(
          to_insert, *to_insert->server, this->weak_from_this(),
          split_container_type, this->tiling_state, false))
      ->insert_child(to_insert);
}

// Inserts a toplevel nested in a new split_container.
std::weak_ptr<GfContainer> GfContainer::insert_child_in_split(
    gfwl_toplevel *to_insert, std::weak_ptr<GfContainer> insert_after,
    enum gfwl_container_type split_container_type) {
  assert(split_container_type != GFWL_CONTAINER_TOPLEVEL);

  return this->child_containers
      .emplace(std::find(this->child_containers.begin(),
                         this->child_containers.end(), insert_after.lock()),
               std::make_shared<GfContainer>(
                   to_insert, *to_insert->server, this->weak_from_this(),
                   split_container_type, this->tiling_state, false))
      ->get()
      ->insert_child(to_insert);
}

void GfContainer::set_focused_toplevel_container() {

  auto sp_tiling_state = this->tiling_state.lock();
  if (sp_tiling_state) {
    sp_tiling_state->active_toplevel_container = this->weak_from_this();
    sp_tiling_state->server->active_toplevel_container.push_front(
        this->weak_from_this());
  }
}

gfwl_split_direction GfContainer::get_split_dir_longer() {
  // TODO: This will fail once too many windows are open. Maybe do something
  // about this?
  assert(this->box.height && this->box.height);
  if (this->box.width >= this->box.height)
    return GFWL_SPLIT_DIR_HORI;
  return GFWL_SPLIT_DIR_VERT;
}

void GfContainer::close() {
  auto parent = this->parent_container.lock();
  auto position_in_parent =
      std::find(parent->child_containers.begin(),
                parent->child_containers.end(), this->shared_from_this());

  parent->child_containers.erase(position_in_parent);
  if (parent->child_containers.empty() && parent->is_root == false) {
    parent->close();
  }
  if (parent->server.active_toplevel_container.front().lock().get() == this) {
    parent->server.active_toplevel_container.pop_front();
  }
  focus_next_in_stack(this->weak_from_this(),
                      parent->server.active_toplevel_container);
}

enum gfwl_split_direction GfContainer::get_split_dir_from_container_type() {
  switch (this->e_type) {
  case GFWL_CONTAINER_VSPLIT:
    return GFWL_SPLIT_DIR_VERT;
  case GFWL_CONTAINER_HSPLIT:
    return GFWL_SPLIT_DIR_HORI;
  case GFWL_CONTAINER_ROOT:
    return GFWL_SPLIT_DIR_HORI;
  case GFWL_CONTAINER_TOPLEVEL:
    return GFWL_SPLIT_DIR_UNKNOWN;
  case GFWL_CONTAINER_UNKNOWN:
    return GFWL_SPLIT_DIR_UNKNOWN;
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
    curr->set_container_box(box);
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
    curr->set_container_box(box);
    count += 1;
  }
}

void GfContainer::split_containers() {
  switch (this->get_split_dir_from_container_type()) {
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

// Sets the size and position of a container based on a wlr_box.
void GfContainer::set_container_box(struct wlr_box box) {
  this->box = box;
  if (e_type == GFWL_CONTAINER_TOPLEVEL) {
    struct wlr_xdg_toplevel *toplevel = this->toplevel->xdg_toplevel;
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
  // I need to figure out what the heck the difference between emplace and
  // push is. I also need to figure out how to get my lost shared_ptr back ):
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
