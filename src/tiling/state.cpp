#include <cassert>
#include "state.hpp"
#include <tiling/container.hpp>

void gfwl_tiling_state::insert_child_container(
    std::shared_ptr<GfContainer> parent, std::shared_ptr<GfContainer> child) {
  child->parent_container = parent;

  // TODO: Use vectors.
  //  if (child->link.next)
  //    wl_list_remove(&child->link);
  //  wl_list_insert(&parent->child_containers, &child->link);
}

void gfwl_tiling_state::new_vert_split_container(
    std::shared_ptr<GfContainer> new_container,
    std::shared_ptr<GfContainer> focused_container) {
  assert(new_container);
  std::shared_ptr<GfContainer> fc_parent;

  std::shared_ptr<GfContainer> split_container =
      create_parent_container(new_container, GFWL_CONTAINER_VSPLIT);
  if (focused_container) {
    fc_parent = focused_container->parent_container;
    assert(fc_parent);
    // TODO: Replace with Vector Ops
    // if (focused_container->link.next)
    //   wl_list_remove(&focused_container->link);
    // wl_list_insert(&split_container->child_containers,
    //               &focused_container->link);
  }
  assert(split_container && split_container->e_type == GFWL_CONTAINER_VSPLIT);

  // if (fc_parent)
  //   wl_list_insert(&fc_parent->child_containers, &split_container->link);
  // else
  //   wl_list_insert(&new_container->tiling_state->root->child_containers,
  //                  &split_container->link);
}

// I think these need to be changed for nesting.
void gfwl_tiling_state::new_hori_split_container(
    std::shared_ptr<GfContainer> new_container,
    std::shared_ptr<GfContainer> focused_container) {
  assert(new_container);
  std::shared_ptr<GfContainer> fc_parent = NULL;

  std::shared_ptr<GfContainer> split_container =
      create_parent_container(new_container, GFWL_CONTAINER_HSPLIT);
  if (focused_container) {
    fc_parent = focused_container->parent_container;
    assert(fc_parent);
    // TODO: Vector instead
    //     if (focused_container->link.next)
    //       wl_list_remove(&focused_container->link);
    //     wl_list_insert(&split_container->child_containers,
    //                   &focused_container->link);
  }
  assert(split_container && split_container->e_type == GFWL_CONTAINER_HSPLIT);

  // TODO: More vector things.
  //  if (fc_parent)
  //    wl_list_insert(&fc_parent->child_containers, &split_container->link);
  //  else
  //    wl_list_insert(&new_container->tiling_state->root->child_containers,
  //                   &split_container->link);
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
  parse_containers(this->root);
}

// Insert is overloaded so that you can directly insert toplevels as well.
void gfwl_tiling_state::insert(gfwl_toplevel *toplevel) {
  assert(toplevel);
  this->insert(create_container_from_toplevel(toplevel));
}
