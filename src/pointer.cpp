#include <includes.hpp>
#include <input.hpp>
#include <scene.hpp>
#include <server.hpp>
#include <wayland-util.h>
#include <wlr/util/edges.h>
#include <wlr/util/log.h>
#include <xdg_shell.hpp>

struct wlr_scene_layer_surface_v1*
     desktop_layersurface_at(GfServer*            server,
                             double               lx,
                             double               ly,
                             struct wlr_surface** surface,
                             double*              sx,
                             double*              sy);
void server_new_pointer(GfServer* server, struct wlr_input_device* device) {
  /* We don't do anything special with pointers. All of our pointer handling
   * is proxied through wlr_cursor. On another compositor, you might take this
   * opportunity to do libinput configuration on the device to set
   * acceleration, etc. */
  wlr_cursor_attach_input_device(server->cursor, device);
}

void reset_cursor_mode(GfServer* server) {
  /* Reset the cursor mode to passthrough. */
  server->cursor_mode      = TINYWL_CURSOR_PASSTHROUGH;
  server->grabbed_toplevel = NULL;
}

static void process_cursor_move(GfServer* server, uint32_t _) {
  /* Move the grabbed toplevel to the new position. */
  struct gfwl_toplevel* toplevel = server->grabbed_toplevel;
  wlr_scene_node_set_position(&toplevel->scene_tree->node,
                              server->cursor->x - server->grab_x,
                              server->cursor->y - server->grab_y);
}

static void process_cursor_resize(GfServer* server, uint32_t _) {
  /*
   * Resizing the grabbed toplevel can be a little bit complicated, because we
   * could be resizing from any corner or edge. This not only resizes the
   * toplevel on one or two axes, but can also move the toplevel if you resize
   * from the top or left edges (or top-left corner).
   *
   * Note that some shortcuts are taken here. In a more fleshed-out
   * compositor, you'd wait for the client to prepare a buffer at the new
   * size, then commit any movement that was prepared.
   */
  struct gfwl_toplevel* toplevel = server->grabbed_toplevel;
  double                border_x = server->cursor->x - server->grab_x;
  double                border_y = server->cursor->y - server->grab_y;
  int                   new_left = server->grab_geobox.x;
  int new_right  = server->grab_geobox.x + server->grab_geobox.width;
  int new_top    = server->grab_geobox.y;
  int new_bottom = server->grab_geobox.y + server->grab_geobox.height;

  if (server->resize_edges & WLR_EDGE_TOP) {
    new_top = border_y;
    if (new_top >= new_bottom) {
      new_top = new_bottom - 1;
    }
  } else if (server->resize_edges & WLR_EDGE_BOTTOM) {
    new_bottom = border_y;
    if (new_bottom <= new_top) {
      new_bottom = new_top + 1;
    }
  }
  if (server->resize_edges & WLR_EDGE_LEFT) {
    new_left = border_x;
    if (new_left >= new_right) {
      new_left = new_right - 1;
    }
  } else if (server->resize_edges & WLR_EDGE_RIGHT) {
    new_right = border_x;
    if (new_right <= new_left) {
      new_right = new_left + 1;
    }
  }

  struct wlr_box geo_box;
  wlr_xdg_surface_get_geometry(toplevel->xdg_toplevel->base, &geo_box);
  wlr_scene_node_set_position(
      &toplevel->scene_tree->node, new_left - geo_box.x, new_top - geo_box.y);

  int new_width  = new_right - new_left;
  int new_height = new_bottom - new_top;
  wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel, new_width, new_height);
}

static void process_cursor_motion(GfServer* server, uint32_t time) {
  /* If the mode is non-passthrough, delegate to those functions. */
  if (server->cursor_mode == TINYWL_CURSOR_MOVE) {
    process_cursor_move(server, time);
    return;
  } else if (server->cursor_mode == TINYWL_CURSOR_RESIZE) {
    process_cursor_resize(server, time);
    return;
  }

  /* Otherwise, find the toplevel under the pointer and send the event along. */
  double              sx, sy;
  struct wlr_seat*    seat        = server->seat;
  struct wlr_surface* wlr_surface = NULL;
  // TODO: This has yet to be tested. First I need to add a check to make sure
  // desktop_toplevel_at isn't overriding this. Also, fuzzel doesn't do any
  // mouse things. And Wofi is crashing.

  struct gfwl_toplevel* toplevel = desktop_toplevel_at(
      server, server->cursor->x, server->cursor->y, &wlr_surface, &sx, &sy);

  if (!toplevel) {
    /* If there's no toplevel under the cursor, set the cursor image to a
     * default. This is what makes the cursor image appear when you move it
     * around the screen, not over any toplevels. */
    wlr_cursor_set_xcursor(server->cursor, server->cursor_mgr, "default");
  }
  if (wlr_surface) {
    /*
     * Send pointer enter and motion events.
     *
     * The enter event gives the surface "pointer focus", which is distinct
     * from keyboard focus. You get pointer focus by moving the pointer over
     * a window.
     *
     * Note that wlroots will avoid sending duplicate enter/motion events if
     * the surface has already has pointer focus or if the client is already
     * aware of the coordinates passed.
     */
    wlr_seat_pointer_notify_enter(seat, wlr_surface, sx, sy);
    wlr_seat_pointer_notify_motion(seat, time, sx, sy);
  } else {
    /* Clear pointer focus so future button events and such are not sent to
     * the last client to have the cursor over it. */
    wlr_seat_pointer_clear_focus(seat);
  }
}

void server_cursor_motion(struct wl_listener* listener, void* data) {
  /* This event is forwarded by the cursor when a pointer emits a _relative_
   * pointer motion event (i.e. a delta) */
  GfServer* server = wl_container_of(listener, server, cursor_motion);
  struct wlr_pointer_motion_event* event = (wlr_pointer_motion_event*)data;
  /* The cursor doesn't move unless we tell it to. The cursor automatically
   * handles constraining the motion to the output layout, as well as any
   * special configuration applied for the specific input device which
   * generated the event. You can pass NULL for the device if you want to move
   * the cursor around without any input. */
  // wlr_log(WLR_INFO, "Cursor moved: x=%f, y=%f", event->delta_x,
  // event->delta_y);
  wlr_cursor_move(
      server->cursor, &event->pointer->base, event->delta_x, event->delta_y);
  process_cursor_motion(server, event->time_msec);
}

void server_cursor_motion_absolute(struct wl_listener* listener, void* data) {
  /* This event is forwarded by the cursor when a pointer emits an _absolute_
   * motion event, from 0..1 on each axis. This happens, for example, when
   * wlroots is running under a Wayland window rather than KMS+DRM, and you
   * move the mouse over the window. You could enter the window from any edge,
   * so we have to warp the mouse there. There is also some hardware which
   * emits these events. */
  GfServer* server = wl_container_of(listener, server, cursor_motion_absolute);
  struct wlr_pointer_motion_absolute_event* event =
      (wlr_pointer_motion_absolute_event*)data;
  wlr_cursor_warp_absolute(
      server->cursor, &event->pointer->base, event->x, event->y);
  process_cursor_motion(server, event->time_msec);
}

void server_cursor_button(struct wl_listener* listener, void* data) {
  /* This event is forwarded by the cursor when a pointer emits a button
   * event. */
  GfServer* server = wl_container_of(listener, server, cursor_button);
  struct wlr_pointer_button_event* event = (wlr_pointer_button_event*)data;
  /* Notify the client with pointer focus that a button press has occurred */
  wlr_seat_pointer_notify_button(
      server->seat, event->time_msec, event->button, event->state);
  double                sx, sy;
  struct wlr_surface*   surface  = NULL;
  struct gfwl_toplevel* toplevel = desktop_toplevel_at(
      server, server->cursor->x, server->cursor->y, &surface, &sx, &sy);
  if (event->state == WL_POINTER_BUTTON_STATE_RELEASED) {
    /* If you released any buttons, we exit interactive move/resize mode. */
    reset_cursor_mode(server);
  } else {
    /* Focus that client if the button was _pressed_ */
    focus_toplevel(toplevel, surface);
  }
}

void server_cursor_axis(struct wl_listener* listener, void* data) {
  /* This event is forwarded by the cursor when a pointer emits an axis event,
   * for example when you move the scroll wheel. */
  GfServer* server = wl_container_of(listener, server, cursor_axis);
  struct wlr_pointer_axis_event* event = (wlr_pointer_axis_event*)data;
  /* Notify the client with pointer focus of the axis event. */
  wlr_seat_pointer_notify_axis(server->seat,
                               event->time_msec,
                               event->orientation,
                               event->delta,
                               event->delta_discrete,
                               event->source,
                               event->relative_direction);
}

void server_cursor_frame(struct wl_listener*    listener,
                         [[maybe_unused]] void* data) {
  /* This event is forwarded by the cursor when a pointer emits an frame
   * event. Frame events are sent after regular pointer events to group
   * multiple events together. For instance, two axis events may happen at the
   * same time, in which case a frame event won't be sent in between. */
  GfServer* server = wl_container_of(listener, server, cursor_frame);
  /* Notify the client with pointer focus of the frame event. */
  wlr_seat_pointer_notify_frame(server->seat);
}

struct gfwl_toplevel* desktop_toplevel_at(GfServer*            server,
                                          double               lx,
                                          double               ly,
                                          struct wlr_surface** surface,
                                          double*              sx,
                                          double*              sy) {
  /* This returns the topmost node in the scene at the given layout coords.
   * We only care about surface nodes as we are specifically looking for a
   * surface in the surface tree of a gfwl_toplevel. */
  struct wlr_scene_node* node =
      wlr_scene_node_at(&server->scene.root->tree.node, lx, ly, sx, sy);
  if (node == NULL || node->type != WLR_SCENE_NODE_BUFFER) {
    return NULL;
  }
  struct wlr_scene_buffer*  scene_buffer = wlr_scene_buffer_from_node(node);
  struct wlr_scene_surface* scene_surface =
      wlr_scene_surface_try_from_buffer(scene_buffer);
  if (!scene_surface) {
    return NULL;
  }

  *surface = scene_surface->surface;
  /* Find the node corresponding to the gfwl_toplevel at the root of this
   * surface tree, it is the only one for which we set the data field. */
  struct wlr_scene_tree* tree = node->parent;
  while (tree != NULL && tree->node.data == NULL) {
    tree = tree->node.parent;
  }
  // Only return the tree's node IF it has a node.
  if (tree)
    return (struct gfwl_toplevel*)tree->node.data;
  return NULL;
}

struct wlr_scene_layer_surface_v1*
desktop_layersurface_at(GfServer*            server,
                        double               lx,
                        double               ly,
                        struct wlr_surface** surface,
                        double*              sx,
                        double*              sy) {
  /* This returns the topmost node in the scene at the given layout coords.
   * We only care about surface nodes as we are specifically looking for a
   * surface in the surface tree of a gfwl_toplevel. */
  struct wlr_scene_node* node =
      wlr_scene_node_at(&server->scene.layer.top->node, lx, ly, sx, sy);
  if (node == NULL || node->type != WLR_SCENE_NODE_BUFFER) {
    return NULL;
  }
  struct wlr_scene_buffer* scene_buffer = wlr_scene_buffer_from_node(node);
  // TODO: I have a new theory that I might be able to just look at the
  // scene_trees parent to see if its owned by a wlr_scene_layer_surface_v1
  struct wlr_scene_surface* scene_surface =
      wlr_scene_surface_try_from_buffer(scene_buffer);
  if (!scene_surface) {
    return NULL;
  }

  *surface = scene_surface->surface;
  /* Find the node corresponding to the gfwl_toplevel at the root of this
   * surface tree, it is the only one for which we set the data field. */
  struct wlr_scene_tree*             tree = node->parent;

  struct wlr_scene_layer_surface_v1* maybe_scene_layer_surface = NULL;
  while (tree != NULL && maybe_scene_layer_surface == NULL) {
    maybe_scene_layer_surface =
        wl_container_of(tree, maybe_scene_layer_surface, tree);
    tree = tree->node.parent;
  }

  return maybe_scene_layer_surface;
}
