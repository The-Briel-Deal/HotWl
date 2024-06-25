#include "wlr/util/log.h"
#include <assert.h>
#include <layer_shell.h>
#include <output.h>
#include <scene.h>
#include <server.h>
#include <sys/types.h>
#include <wayland-util.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/box.h>
#include <xdg_shell.h>

static u_int16_t count_toplevels(struct wl_list *toplevels) {
  assert(toplevels);
  if (!toplevels)
    return 0;
  struct gfwl_toplevel *curr_toplevel;
  u_int16_t count = 0;
  wl_list_for_each(curr_toplevel, toplevels, link) { count += 1; }
  return count;
}

void hori_split_toplevels(struct wl_list *toplevels,
                          struct gfwl_server *server) {
  // Get count.
  u_int16_t count = count_toplevels(toplevels);
  if (count == 0) {
    wlr_log(WLR_DEBUG, "You probably don't want to divide by 0");
    return;
  }
  /**
   * -- I may want to use these methods later.
   * wlr_output_layout_get(server->output_layout, server->output_layout);
   * wlr_output_layout_get_box(struct wlr_output_layout *layout, struct
   *     wlr_output *reference, struct wlr_box *dest_box)
   **/
  struct gfwl_output *output =
      wl_container_of(server->outputs.next, output, link);

  u_int32_t width = output->wlr_output->width;
  u_int32_t height = output->wlr_output->height;

  u_int32_t per_win_width = width / count;

  // Set all sizes. (recycling count for the index)
  count = 0;
  struct gfwl_toplevel *curr_toplevel;
  wl_list_for_each(curr_toplevel, toplevels, link) {
    const struct wlr_box box = {.x = count * per_win_width,
                                .y = 0,
                                .width = per_win_width,
                                .height = height};
    set_xdg_surface_box(curr_toplevel->xdg_toplevel, box);
    count += 1;
  }
}

void set_xdg_surface_box(struct wlr_xdg_toplevel *toplevel,
                         struct wlr_box box) {
  // Set the size.
  wlr_xdg_toplevel_set_size(toplevel, box.width, box.height);
  // Set the position.
  struct wlr_scene_tree *scene_tree = toplevel->base->data;
  scene_tree->node.x = box.x;
  scene_tree->node.y = box.y;
};
