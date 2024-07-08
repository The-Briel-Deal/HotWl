#include "state.hpp"
#include <algorithm>
#include <cassert>
#include <memory>
#include <tiling/container.hpp>

void gfwl_tiling_state::insert_child_container(
    std::shared_ptr<GfContainer> parent, std::shared_ptr<GfContainer> child) {

  auto position_to_insert = std::find(
      parent->child_containers.begin(), parent->child_containers.end(),
      parent->tiling_state->active_toplevel_container);
  if (position_to_insert != parent->child_containers.end())
    parent->child_containers.insert(position_to_insert, child);
  else
    parent->child_containers.push_back(child);
  child->parent_container = parent;
}

void gfwl_tiling_state::reparent_container(
    std::shared_ptr<GfContainer> prev_parent,
    std::shared_ptr<GfContainer> new_parent) {
  auto position_to_insert = std::find(
      prev_parent->parent_container->child_containers.begin(),
      prev_parent->parent_container->child_containers.end(), prev_parent);
  prev_parent->parent_container->child_containers.insert(position_to_insert,
                                                         new_parent);
  auto position_to_erase = std::find(
      prev_parent->parent_container->child_containers.begin(),
      prev_parent->parent_container->child_containers.end(), prev_parent);
  prev_parent->parent_container->child_containers.erase(position_to_erase);
  prev_parent->parent_container = new_parent;
  new_parent->child_containers.push_back(prev_parent);
}
void gfwl_tiling_state::new_vert_split_container(
    std::shared_ptr<GfContainer> new_container,
    std::shared_ptr<GfContainer> focused_container) {
  std::shared_ptr<GfContainer> fc_parent;

  std::shared_ptr<GfContainer> split_container =
      create_parent_container(new_container, GFWL_CONTAINER_VSPLIT);
  if (focused_container) {
    split_container->parent_container = focused_container->parent_container;
    // I should probably generalize this between new hori and vert.
    reparent_container(focused_container, split_container);
  } else {
    split_container->parent_container = new_container->tiling_state->root;
    new_container->tiling_state->root->child_containers.push_back(
        split_container);
  }
}

void gfwl_tiling_state::new_hori_split_container(
    std::shared_ptr<GfContainer> new_container,
    std::shared_ptr<GfContainer> focused_container) {
  auto split_container =
      create_parent_container(new_container, GFWL_CONTAINER_HSPLIT);
  if (focused_container) {
    split_container->parent_container = focused_container->parent_container;
    // I should probably generalize this between new hori and vert.
    // The new split container should go in the same position as the previous.

    reparent_container(focused_container, split_container);
  } else {
    split_container->parent_container = new_container->tiling_state->root;
    new_container->tiling_state->root->child_containers.push_back(
        split_container);
  }
}

void gfwl_tiling_state::flip_split_direction() {
  if (this->split_dir == GFWL_SPLIT_DIR_HORI)
    this->split_dir = GFWL_SPLIT_DIR_VERT;
  else
    this->split_dir = GFWL_SPLIT_DIR_HORI;
}

void gfwl_tiling_state::insert(std::shared_ptr<GfContainer> container) {
  container->tiling_state = this;

  // lf means last focused btw.
  std::shared_ptr<GfContainer> lft_container = NULL, lftc_container = NULL;
  enum gfwl_split_direction split_dir = GFWL_SPLIT_DIR_UNKNOWN;

  // TODO: Come up with better names for this.
  lft_container = this->active_toplevel_container;
  if (lft_container)
    lftc_container = lft_container->parent_container;
  if (lftc_container) {
    split_dir = lftc_container->get_split_dir();
  }

  switch (this->split_dir) {
  case GFWL_SPLIT_DIR_VERT:
    if (split_dir == GFWL_SPLIT_DIR_VERT)
      insert_child_container(lftc_container, container);
    else
      new_vert_split_container(container, lft_container);
    break;
  case GFWL_SPLIT_DIR_HORI:
    if (split_dir == GFWL_SPLIT_DIR_HORI)
      insert_child_container(lftc_container, container);
    else
      new_hori_split_container(container, lft_container);
    break;

  case GFWL_SPLIT_DIR_UNKNOWN:
    wlr_log(WLR_ERROR, "Split dir shouldn't ever be unknown on a toplevel.");
    break;
  }

  // After every insertion we want to resize containers to the new state.
  this->root->parse_containers();
}

// Insert is overloaded so that you can directly insert toplevels as well.
void gfwl_tiling_state::insert(gfwl_toplevel *toplevel) {
  assert(toplevel);
  this->insert(create_container_from_toplevel(toplevel));
}
