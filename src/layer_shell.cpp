#include <assert.h>
#include <includes.hpp>
#include <layer_shell.hpp>
#include <memory>
#include <output.hpp>
#include <scene.hpp>
#include <server.hpp>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/util/log.h>

void focus_layer_surface(struct gfwl_layer_surface* gfwl_layer_surface) {
  struct wlr_seat*     seat        = gfwl_layer_surface->server->seat;
  struct wlr_keyboard* keyboard    = wlr_seat_get_keyboard(seat);
  gfwl_layer_surface->prev_focused = seat->keyboard_state.focused_surface;

  wlr_seat_keyboard_notify_enter(
      seat, gfwl_layer_surface->wlr_layer_surface->surface, keyboard->keycodes,
      keyboard->num_keycodes, &keyboard->modifiers);
}

void unfocus_layer_surface(struct gfwl_layer_surface* gfwl_layer_surface) {
  struct wlr_seat*     seat     = gfwl_layer_surface->server->seat;
  struct wlr_keyboard* keyboard = wlr_seat_get_keyboard(seat);

  wlr_seat_keyboard_notify_enter(seat, gfwl_layer_surface->prev_focused,
                                 keyboard->keycodes, keyboard->num_keycodes,
                                 &keyboard->modifiers);
}

// Returns false if failed.
bool center_scene_layer_surface(
    struct wlr_scene_layer_surface_v1* scene_layer_surface,
    struct wlr_output*                 wlr_output) {
  assert(wlr_output);
  if (!wlr_output || !scene_layer_surface)
    return false;

  int32_t op_x = wlr_output->width;
  int32_t op_y = wlr_output->height;

  assert(scene_layer_surface);
  if (scene_layer_surface) {
    scene_layer_surface->tree->node.x =
        (op_x - scene_layer_surface->layer_surface->pending.desired_width) / 2;
    scene_layer_surface->tree->node.y =
        (op_y - scene_layer_surface->layer_surface->pending.desired_height) / 2;
  }

  return true;
}

void handle_layer_surface_map(struct wl_listener*    listener,
                              [[maybe_unused]] void* data) {
  struct gfwl_layer_surface* gfwl_layer_surface =
      wl_container_of(listener, gfwl_layer_surface, map);

  center_scene_layer_surface(gfwl_layer_surface->scene,
                             gfwl_layer_surface->wlr_layer_surface->output);
  focus_layer_surface(gfwl_layer_surface);
}

void handle_layer_surface_unmap(struct wl_listener*    listener,
                                [[maybe_unused]] void* data) {
  struct gfwl_layer_surface* gfwl_layer_surface =
      wl_container_of(listener, gfwl_layer_surface, unmap);
  unfocus_layer_surface(gfwl_layer_surface);
}

void handle_layer_surface_commit(struct wl_listener*    listener,
                                 [[maybe_unused]] void* data) {
  struct gfwl_layer_surface* gfwl_layer_surface =
      wl_container_of(listener, gfwl_layer_surface, commit);

  if (gfwl_layer_surface->wlr_layer_surface->initial_commit) {
    wlr_layer_surface_v1_configure(gfwl_layer_surface->wlr_layer_surface, 0, 0);
  }
}

void handle_new_layer_shell_surface(struct wl_listener* listener, void* data) {
  // Grab our server (parent of the listener).
  class GfServer* server =
      wl_container_of(listener, server, new_layer_shell_surface);
  if (!server) {
    wlr_log(WLR_ERROR, "No server from listener.");
    return;
  }

  // Grab layer surface.
  struct wlr_layer_surface_v1* wlr_layer_surface = (wlr_layer_surface_v1*)data;
  if (!wlr_layer_surface) {
    wlr_log(WLR_ERROR, "No layer surface.");
    return;
  }

  // Dynamically allocate a new layer surface wrapper and save callback args.
  struct gfwl_layer_surface* gfwl_layer_surface =
      (struct gfwl_layer_surface*)calloc(1, sizeof(*gfwl_layer_surface));
  if (!gfwl_layer_surface) {
    wlr_log(WLR_ERROR, "No gfwl layer surface.");
    return;
  }
  gfwl_layer_surface->wlr_layer_surface = wlr_layer_surface;
  gfwl_layer_surface->server            = server;

  // Check for layer surface output.
  if (!wlr_layer_surface->output) {
    wlr_log(WLR_INFO, "No output on layer surface.");
    struct wlr_seat* seat = server->seat;
    if (!seat) {
      wlr_log(WLR_ERROR, "No seat.");
      return;
    }

    // TODO: Make this not always just put the layer shell on the first output
    // lol.
    // Get first output.
    struct std::shared_ptr<gfwl_output> gfwl_output = server->outputs[0];
    if (!gfwl_output) {
      wlr_log(WLR_ERROR, "No output.");
      return;
    }

    // Set layer_surface output.
    if (gfwl_output->wlr_output) {
      wlr_layer_surface->output = gfwl_output->wlr_output;
    } else {
      wlr_log(WLR_ERROR, "gfwl_output has no wlr_output.");
      return;
    }
  }
  // Get gfwl_output from wlr_output
  struct gfwl_output* gfwl_output =
      wl_container_of(wlr_layer_surface->output, gfwl_output, wlr_output);
  if (!gfwl_output) {
    wlr_log(WLR_ERROR, "No gfwl_output is parent of wlr_output.");
    return;
  }

  gfwl_layer_surface->output = gfwl_output;

  // Create the scene.
  struct wlr_scene_layer_surface_v1* scene_surface =
      wlr_scene_layer_surface_v1_create(server->scene.layer.top,
                                        wlr_layer_surface);
  // Add to layer_surface object.
  gfwl_layer_surface->scene = scene_surface;
  // Register commit handler.
  gfwl_layer_surface->commit.notify = handle_layer_surface_commit;
  wl_signal_add(&wlr_layer_surface->surface->events.commit,
                &gfwl_layer_surface->commit);

  // Register map handler.
  gfwl_layer_surface->map.notify = handle_layer_surface_map;
  wl_signal_add(&wlr_layer_surface->surface->events.map,
                &gfwl_layer_surface->map);

  // Register unmap handler.
  gfwl_layer_surface->unmap.notify = handle_layer_surface_unmap;
  wl_signal_add(&wlr_layer_surface->surface->events.unmap,
                &gfwl_layer_surface->unmap);
}
