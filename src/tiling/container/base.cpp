#include "base.hpp"

#include <algorithm>
#include <cassert>
#include <deque>
#include <memory>
#include <server.hpp>
#include <vector>
#include <xdg_shell.hpp>

#include "output.hpp"
#include "root.hpp"
#include "tiling/container/split.hpp"
#include "tiling/state.hpp"
#include "toplevel.hpp"
#include "wlr/types/wlr_xdg_shell.h"
#include "wlr/util/box.h"
#include "wlr/util/log.h"

const wlr_box& GfContainer::get_box() {
  return this->box;
}

void GfContainer::set_container_box(struct wlr_box box_in) {
  this->box = box_in;
}

// Get A List of Toplevels below this Container node.
std::vector<std::weak_ptr<GfContainerToplevel>>
GfContainer::get_top_level_container_list() {
  std::vector<std::weak_ptr<GfContainerToplevel>> list;
  std::deque<std::weak_ptr<GfContainer>>          stack;

  for (const auto& output : server.outputs) {
    stack.push_back(output->tiling_state->root);
  }

  while (!stack.empty()) {
    auto curr_node = stack.back();
    stack.pop_back();
    for (const auto& child : curr_node.lock()->child_containers) {
      switch (child->e_type) {
        case GFWL_CONTAINER_TOPLEVEL:
          list.push_back(std::static_pointer_cast<GfContainerToplevel>(child));
          break;
        case GFWL_CONTAINER_HSPLIT:
        case GFWL_CONTAINER_VSPLIT: stack.push_back(child); break;
        case GFWL_CONTAINER_ROOT:
        case GFWL_CONTAINER_UNKNOWN:
        default: break;
      }
    }
  }
  return list;
}

std::weak_ptr<GfContainerToplevel> GfContainer::insert(GfToplevel* to_insert) {
  return this->insert_based_on_longer_dir(to_insert);
}

std::weak_ptr<GfContainerToplevel>
GfContainer::insert_based_on_longer_dir(GfToplevel* to_insert) {
  assert(this->e_type == GFWL_CONTAINER_TOPLEVEL);
  auto                       parent           = this->parent_container.lock();
  auto                       split_dir_longer = this->get_split_dir_longer();
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
      wlr_log(
          WLR_ERROR,
          "Split dir is an invalid enum value, this should not be the case.");
      break;
  }
  this->move_container_to(new_toplevel_container.lock()->parent_container);
  this->tiling_state.lock()->root->parse_containers();
  // TODO: Fix the rest of the cascading type issues and remove static cast
  return std::static_pointer_cast<GfContainerToplevel>(
      new_toplevel_container.lock());
}

void GfContainer::move_container_to(
    const std::weak_ptr<GfContainer>& new_parent) {
  auto  this_locked = this->shared_from_this();
  auto& prev_parent_child_containers =
      this->parent_container.lock()->child_containers;

  prev_parent_child_containers.erase(
      std::find(prev_parent_child_containers.begin(),
                prev_parent_child_containers.end(),
                this_locked));

  new_parent.lock()->child_containers.insert(
      new_parent.lock()->child_containers.begin(), this_locked);
  this->parent_container = new_parent;
}

// Inserting the toplevel directly, returns a weak pointer to the new
// container.
std::weak_ptr<GfContainerToplevel>
GfContainer::insert_child(GfToplevel* to_insert) {
  std::shared_ptr<GfContainerToplevel> toplevel_container =
      std::make_shared<GfContainerToplevel>(to_insert,
                                            *to_insert->server,
                                            this->weak_from_this(),
                                            this->tiling_state);
  this->child_containers.push_back(
      std::static_pointer_cast<GfContainer>(toplevel_container));
  to_insert->parent_container = toplevel_container;
  // As an optimization down the road, I can try just parsing the changes
  // containers.
  this->tiling_state.lock()->root->parse_containers();
  return toplevel_container;
}

// Inserts a toplevel nested in a new split_container.
std::weak_ptr<GfContainerToplevel> GfContainer::insert_child_in_split(
    GfToplevel* to_insert, enum gfwl_container_type split_container_type) {
  assert(split_container_type != GFWL_CONTAINER_TOPLEVEL);

  return this->child_containers
      .emplace_back(std::make_shared<GfContainerSplit>(*to_insert->server,
                                                       this->weak_from_this(),
                                                       split_container_type,
                                                       this->tiling_state))
      ->insert_child(to_insert);
}

// Inserts a toplevel nested in a new split_container.
std::weak_ptr<GfContainerToplevel> GfContainer::insert_child_in_split(
    GfToplevel*                       to_insert,
    const std::weak_ptr<GfContainer>& insert_after,
    enum gfwl_container_type          split_container_type) {
  assert(split_container_type != GFWL_CONTAINER_TOPLEVEL);

  return this->child_containers
      .emplace(std::find(this->child_containers.begin(),
                         this->child_containers.end(),
                         insert_after.lock()),
               std::make_shared<GfContainerSplit>(*to_insert->server,
                                                  this->weak_from_this(),
                                                  split_container_type,
                                                  this->tiling_state))
      ->get()
      ->insert_child(to_insert);
}

void GfContainerToplevel::set_focused_toplevel_container() {
  auto sp_tiling_state = this->tiling_state.lock();
  if (sp_tiling_state) {

    // TODO: Make this method only on toplevels
    g_Server.active_toplevel_container.push_front(
        std::static_pointer_cast<GfContainerToplevel>(
            this->shared_from_this()));
  }
  // Make this function be on the toplevel class for real.
  auto* casted_this_toplevel = reinterpret_cast<GfContainerToplevel*>(this);
  focus_toplevel(casted_this_toplevel->toplevel,
                 casted_this_toplevel->toplevel->xdg_toplevel->base->surface);
}

gfwl_split_direction GfContainer::get_split_dir_longer() const {
  // TODO(gabe): This will fail once too many windows are open. Maybe do
  // something about this?
  assert(this->box.height && this->box.height);
  if (this->box.width >= this->box.height) {
    return GFWL_SPLIT_DIR_HORI;
  }
  return GFWL_SPLIT_DIR_VERT;
}

void focus_next_in_stack(const std::weak_ptr<GfContainer>&              curr,
                         std::deque<std::weak_ptr<GfContainerToplevel>> stack) {
  while (!stack.empty()) {
    if (!stack.front().expired() && stack.front().lock() != curr.lock()) {
      auto* toplevel_container =
          dynamic_cast<GfContainerToplevel*>(stack.front().lock().get());
      if (toplevel_container != nullptr) {
        toplevel_container->set_focused_toplevel_container();
        return;
      }
    }
    stack.pop_front();
  }
}
void GfContainer::close() {
  auto parent             = this->parent_container.lock();
  auto position_in_parent = std::find(parent->child_containers.begin(),
                                      parent->child_containers.end(),
                                      this->shared_from_this());

  parent->child_containers.erase(position_in_parent);
  if (parent->child_containers.empty()) {
    parent->close();
  }
  if (parent->server.active_toplevel_container.front().lock().get() == this) {
    parent->server.active_toplevel_container.pop_front();
  }
  focus_next_in_stack(this->weak_from_this(),
                      parent->server.active_toplevel_container);
}

/* Close Should not do anything on the Root container */
void GfContainerRoot::close() {}

enum gfwl_split_direction
GfContainer::get_split_dir_from_container_type() const {
  switch (this->e_type) {
    case GFWL_CONTAINER_VSPLIT: return GFWL_SPLIT_DIR_VERT;
    case GFWL_CONTAINER_HSPLIT:
    case GFWL_CONTAINER_ROOT: return GFWL_SPLIT_DIR_HORI;
    case GFWL_CONTAINER_TOPLEVEL:
    case GFWL_CONTAINER_UNKNOWN:
    default: return GFWL_SPLIT_DIR_UNKNOWN;
  }
}

// TODO(gabe): Replace duplicate parts with generalized helpers.
// Change this to get output size from the parent.
void GfContainer::vert_split_containers() {
  // Get count.
  auto count = static_cast<int>(this->child_containers.size());
  if (count == 0) {
    wlr_log(WLR_DEBUG, "You probably don't want to divide by 0");
    return;
  }

  auto width  = this->box.width;
  auto height = this->box.height;

  auto per_win_height = height / count;

  // Set all sizes. (recycling count for the index)
  count = 0;
  for (const auto& curr : this->child_containers) {
    wlr_box curr_box = {.x      = this->box.x,
                        .y      = this->box.y + (per_win_height * count),
                        .width  = width,
                        .height = per_win_height};
    curr->set_container_box(curr_box);
    count += 1;
  }
}

void GfContainer::hori_split_containers() {
  // Get count.
  auto count = static_cast<int>(this->child_containers.size());
  if (count == 0) {
    wlr_log(WLR_DEBUG, "You probably don't want to divide by 0");
    return;
  }
  // Get Width and Height.
  int width  = this->box.width;
  int height = this->box.height;

  // Get per_win_width.
  int per_win_width = width / count;

  // Set all sizes. (recycling count for the index)
  count = 0;
  for (const auto& curr : this->child_containers) {
    wlr_box curr_box = {.x      = this->box.x + (count * per_win_width),
                        .y      = this->box.y,
                        .width  = per_win_width,
                        .height = height};
    curr->set_container_box(curr_box);
    count += 1;
  }
}

void GfContainer::split_containers() {
  switch (this->get_split_dir_from_container_type()) {
    case GFWL_SPLIT_DIR_HORI: this->hori_split_containers(); break;
    case GFWL_SPLIT_DIR_VERT: this->vert_split_containers(); break;
    case GFWL_SPLIT_DIR_UNKNOWN:
    default: break;
  }
}

void GfContainer::parse_children() {
  for (const auto& child : this->child_containers) {
    if (child->e_type == GFWL_CONTAINER_HSPLIT ||
        child->e_type == GFWL_CONTAINER_VSPLIT) {
      child->parse_containers();
    }
  }
}
/* The default behavior is to divide size of child containers and then recurse
 * for all split child containers. */
void GfContainer::parse_containers() {

  this->split_containers();
  this->parse_children();
}
