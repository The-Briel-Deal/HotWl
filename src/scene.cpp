#include <includes.hpp>
#include <output.hpp>
#include <scene.hpp>
#include <server.hpp>
#include <xdg_shell.hpp>

// TODO: I want to move all tiling and container logic to its own class.
enum gfwl_split_direction get_split_dir(struct gfwl_container *container);
void split_containers(struct gfwl_container *container);

static u_int16_t count_toplevel_containers(struct wl_list *toplevels) {
  assert(toplevels);
  if (!toplevels)
    return 0;
  struct gfwl_container *curr_toplevel;
  u_int16_t count = 0;
  wl_list_for_each(curr_toplevel, toplevels, link) { count += 1; }
  return count;
}

// It would be nice to log containers during this for debugging.
void parse_containers(struct gfwl_container *container) {
  if (container->is_root) {
    // Get output.
    struct gfwl_output *output =
        wl_container_of(container->server->outputs.next, output, link);
    container->box.width = output->wlr_output->width;
    container->box.height = output->wlr_output->height;
  }
  split_containers(container);
  struct gfwl_container *cursor;
  struct wl_list *head = &container->child_containers;
  wl_list_for_each(cursor, head, link) {
    if (cursor->e_type == GFWL_CONTAINER_HSPLIT ||
        cursor->e_type == GFWL_CONTAINER_VSPLIT) {
      parse_containers(cursor);
    }
  }
}

// TODO: Replace duplicate parts with generalized helpers.
// Change this to get output size from the parent.
void vert_split_containers(struct gfwl_container *container) {
  struct wl_list *toplevel_containers = &container->child_containers;
  // Get count.
  int count = count_toplevel_containers(toplevel_containers);
  if (count == 0) {
    wlr_log(WLR_DEBUG, "You probably don't want to divide by 0");
    return;
  }

  // Get Width and Height.
  int width = container->box.width;
  int height = container->box.height;

  // Get per_win_width.
  int per_win_height = height / count;

  // Set all sizes. (recycling count for the index)
  count = 0;
  struct gfwl_container *curr_toplevel_container;
  wl_list_for_each(curr_toplevel_container, toplevel_containers, link) {
    const struct wlr_box box = {.x = container->box.x,
                                .y = per_win_height * count,
                                .width = width,
                                .height = per_win_height};
    set_container_box(curr_toplevel_container, box);
    count += 1;
  }
}

void hori_split_containers(struct gfwl_container *container) {
  struct wl_list *toplevel_containers = &container->child_containers;
  // Get count.
  u_int16_t count = count_toplevel_containers(toplevel_containers);
  if (count == 0) {
    wlr_log(WLR_DEBUG, "You probably don't want to divide by 0");
    return;
  }

  // Get Width and Height.
  int width = container->box.width;
  int height = container->box.height;

  // Get per_win_width.
  int per_win_width = width / count;

  // Set all sizes. (recycling count for the index)
  count = 0;
  struct gfwl_container *curr_toplevel_container;
  wl_list_for_each(curr_toplevel_container, toplevel_containers, link) {
    const struct wlr_box box = {.x = count * per_win_width,
                                .y = 0,
                                .width = per_win_width,
                                .height = height};
    set_container_box(curr_toplevel_container, box);
    count += 1;
  }
}

void split_containers(struct gfwl_container *container) {
  switch (get_split_dir(container)) {
  case GFWL_SPLIT_DIR_HORI:
    hori_split_containers(container);
    break;
  case GFWL_SPLIT_DIR_VERT:
    vert_split_containers(container);
    break;
  default:
    break;
  }
}

// Only supports toplevels for now.
void set_container_box(struct gfwl_container *container, struct wlr_box box) {
  container->box = box;
  if (container->e_type == GFWL_CONTAINER_TOPLEVEL) {
    struct wlr_xdg_toplevel *toplevel = container->toplevel->xdg_toplevel;
    // Set the size.
    wlr_xdg_toplevel_set_size(toplevel, box.width, box.height);
    // Set the position.
    struct wlr_scene_tree *scene_tree = (wlr_scene_tree *)toplevel->base->data;
    wlr_scene_node_set_position(&scene_tree->node, box.x, box.y);
  }
};

struct gfwl_container *
create_parent_container(struct gfwl_container *child_container,
                        enum gfwl_container_type type) {
  // Just making sure I'm never passing unknown.
  assert(type != GFWL_CONTAINER_UNKNOWN);

  struct gfwl_container *parent_container =
      (gfwl_container *)calloc(1, sizeof(*parent_container));
  parent_container->e_type = type;
  parent_container->server = child_container->server;
  parent_container->tiling_state = child_container->tiling_state;
  child_container->parent_container = parent_container;
  wl_list_init(&parent_container->child_containers);
  wl_list_insert(&parent_container->child_containers, &child_container->link);
  return parent_container;
}


struct gfwl_container *
create_container_from_toplevel(struct gfwl_toplevel *toplevel) {
	// Replace calloc with new
  struct gfwl_container *container =
      (gfwl_container *)calloc(1, sizeof(*container));

  container->e_type = GFWL_CONTAINER_TOPLEVEL;
  container->toplevel = toplevel;
  container->server = toplevel->server;
  toplevel->parent_container = container;

  return container;
}

// Only Tested with toplevel_containers.



void set_focused_toplevel_container(struct gfwl_container *container) {
  assert(container);
  struct gfwl_tiling_state *tiling_state = container->tiling_state;
  assert(tiling_state);

  tiling_state->active_toplevel_container = container;
}

// bool gfwl_tiling_state::insert(gfwl_container container) { return true; }

// TODO: FIX BUG WHERE YOU MAKE A HORI CONTAINER IN A SPLIT CONTAINER. I
//       JUST COVERED IT UP WITH THE THIRD PART OF THE IF STATEMENT.
// TODO: WM CRASHES WHEN THE FIRST CONTAINER IS SPLIT VERT.
// TODO: Change lf_toplevel to currently_focused container..
// TODO: Create a tiling_state struct.
// void add_to_tiling_layout(struct gfwl_toplevel *toplevel,
//                           struct gfwl_tiling_state *tiling_state) {
//   assert(toplevel);
//   struct gfwl_container *toplevel_container =
//       create_container_from_toplevel(toplevel);
//   assert(toplevel_container);
//   toplevel_container->tiling_state = tiling_state;
// 
//   // lf means last focused btw.
//   struct gfwl_container *lft_container = NULL, *lftc_container = NULL;
//   enum gfwl_split_direction split_dir = GFWL_SPLIT_DIR_UNKNOWN;
// 
//   // TODO: Come up with better names for this.
//   lft_container = tiling_state->active_toplevel_container;
//   if (lft_container)
//     lftc_container = lft_container->parent_container;
//   if (lftc_container) {
//     split_dir = get_split_dir(lftc_container);
//   }
// 
//   switch (tiling_state->split_dir) {
//   case GFWL_SPLIT_DIR_VERT:
//     if (split_dir == GFWL_SPLIT_DIR_VERT)
//       insert_child_container(lftc_container, toplevel_container);
//     else
//       new_vert_split_container(toplevel_container, lft_container);
//     break;
//   case GFWL_SPLIT_DIR_HORI:
//     if (split_dir == GFWL_SPLIT_DIR_HORI)
//       insert_child_container(lftc_container, toplevel_container);
//     else
//       new_hori_split_container(toplevel_container, lft_container);
//     break;
// 
//   case GFWL_SPLIT_DIR_UNKNOWN:
//     wlr_log(WLR_ERROR, "Split dir shouldn't ever be unknown on a toplevel.");
//     break;
//   }
//   parse_containers(tiling_state->root);
// }
