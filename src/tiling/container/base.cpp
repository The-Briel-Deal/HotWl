#include "base.hpp"
#include "root.hpp"
#include "toplevel.hpp"
#include <algorithm>
#include <server.hpp>
#include <tiling/focus.hpp>
#include <xdg_shell.hpp>

// TODO: Add this to container or move to helpers.
void focus_next_in_stack(std::weak_ptr<GfContainer>             curr,
                         std::deque<std::weak_ptr<GfContainer>> stack) {
  while (!stack.empty()) {
    if (!stack.front().expired() && stack.front().lock() != curr.lock()) {
      auto toplevel_container =
          dynamic_cast<GfContainerToplevel*>(stack.front().lock().get());
      if (toplevel_container != NULL) {
        focus_toplevel(
            toplevel_container->toplevel,
            toplevel_container->toplevel->xdg_toplevel->base->surface);
        return;
      }
    }
    stack.pop_front();
  }
}

// Return const reference to containers box.
const wlr_box& GfContainer::get_box() {
  return this->box;
}

// This is intended for toplevel containers.
std::weak_ptr<GfContainer>
GfContainer::insert_based_on_longer_dir(gfwl_toplevel* to_insert) {
  // TODO: I will likely have to make sure the container is inserted at the
  // right position.
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
  return this->parent_container;
}

void GfContainer::move_container_to(std::weak_ptr<GfContainer> new_parent) {
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
std::weak_ptr<GfContainer> GfContainer::insert_child(gfwl_toplevel* to_insert) {
  auto toplevel_container =
      this->child_containers
          .emplace_back(
              std::make_shared<GfContainerToplevel>(to_insert,
                                                    *to_insert->server,
                                                    this->weak_from_this(),
                                                    this->tiling_state))
          ->weak_from_this();
  to_insert->parent_container = toplevel_container;
  // As an optimization down the road, I can try just parsing the changes
  // containers.
  this->tiling_state.lock()->root->parse_containers();
  return toplevel_container;
}

// Inserts a toplevel nested in a new split_container.
std::weak_ptr<GfContainer> GfContainer::insert_child_in_split(
    gfwl_toplevel* to_insert, enum gfwl_container_type split_container_type) {
  assert(split_container_type != GFWL_CONTAINER_TOPLEVEL);

  return this->child_containers
      .emplace_back(std::make_shared<GfContainerSplit>(*to_insert->server,
                                                       this->weak_from_this(),
                                                       split_container_type,
                                                       this->tiling_state))
      ->insert_child(to_insert);
}

// Inserts a toplevel nested in a new split_container.
std::weak_ptr<GfContainer> GfContainer::insert_child_in_split(
    gfwl_toplevel*             to_insert,
    std::weak_ptr<GfContainer> insert_after,
    enum gfwl_container_type   split_container_type) {
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

void GfContainer::set_focused_toplevel_container() {

  auto sp_tiling_state = this->tiling_state.lock();
  if (sp_tiling_state) {
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
void GfContainerRoot::close() {
  return;
}

enum gfwl_split_direction GfContainer::get_split_dir_from_container_type() {
  switch (this->e_type) {
    case GFWL_CONTAINER_VSPLIT: return GFWL_SPLIT_DIR_VERT;
    case GFWL_CONTAINER_HSPLIT: return GFWL_SPLIT_DIR_HORI;
    case GFWL_CONTAINER_ROOT: return GFWL_SPLIT_DIR_HORI;
    case GFWL_CONTAINER_TOPLEVEL: return GFWL_SPLIT_DIR_UNKNOWN;
    case GFWL_CONTAINER_UNKNOWN: return GFWL_SPLIT_DIR_UNKNOWN;
    default: return GFWL_SPLIT_DIR_UNKNOWN;
  }
}

// TODO: Replace duplicate parts with generalized helpers.
// Change this to get output size from the parent.
void GfContainer::vert_split_containers() {
  // Get count.
  auto count = int(this->child_containers.size());
  if (count == 0) {
    wlr_log(WLR_DEBUG, "You probably don't want to divide by 0");
    return;
  }

  auto width  = this->box.width;
  auto height = this->box.height;

  auto per_win_height = height / count;

  // Set all sizes. (recycling count for the index)
  count = 0;
  for (auto curr : this->child_containers) {
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
  auto count = int(this->child_containers.size());
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
  for (auto curr : this->child_containers) {
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
    case GFWL_SPLIT_DIR_UNKNOWN: break;
    default: break;
  }
}

void GfContainer::parse_children() {
  for (auto child : this->child_containers) {
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