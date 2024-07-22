#include "wlr/util/box.h"
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

  wlr_seat_keyboard_notify_enter(seat,
                                 gfwl_layer_surface->wlr_layer_surface->surface,
                                 keyboard->keycodes,
                                 keyboard->num_keycodes,
                                 &keyboard->modifiers);
}

void unfocus_layer_surface(struct gfwl_layer_surface* gfwl_layer_surface) {
  struct wlr_seat*     seat     = gfwl_layer_surface->server->seat;
  struct wlr_keyboard* keyboard = wlr_seat_get_keyboard(seat);

  wlr_seat_keyboard_notify_enter(seat,
                                 gfwl_layer_surface->prev_focused,
                                 keyboard->keycodes,
                                 keyboard->num_keycodes,
                                 &keyboard->modifiers);
}

// Returns false if failed.
wlr_box center_scene_layer_surface(gfwl_layer_surface* layer_surface) {
  auto    scene_layer_surface = layer_surface->scene;
  auto    wlr_output          = layer_surface->output->wlr_output;
  wlr_box box                 = {.x     = 0,
                                 .y     = 0,
                                 .width = static_cast<int>(
                     layer_surface->wlr_layer_surface->pending.desired_width),
                                 .height = static_cast<int>(
                     layer_surface->wlr_layer_surface->pending.desired_height)};

  int32_t op_x = wlr_output->width;
  int32_t op_y = wlr_output->height;

  box.x =
      (op_x - scene_layer_surface->layer_surface->pending.desired_width) / 2;
  box.y =
      (op_y - scene_layer_surface->layer_surface->pending.desired_height) / 2;

  return box;
}

void get_box_from_anchors(gfwl_layer_surface* layer_surface) {
  auto    state  = layer_surface->scene->layer_surface->pending;
  auto    anchor = state.anchor;
  auto    output = layer_surface->scene->layer_surface->output;
  wlr_box box;

  if (!anchor) {
    box = center_scene_layer_surface(layer_surface);
  } else {
    box = {
        .x      = 0,
        .y      = 0,
        .width  = static_cast<int>(state.desired_width),
        .height = static_cast<int>(state.desired_height),
    };
    if (anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT) {
      box.x = 0;
    }
    if (anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM) {
      box.y = output->height - box.height;
    }
    if (anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT) {
      box.width = output->width;
    }
    if (anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP) {
      // TODO: Handle Top Positioning Here.
    }
  }

  wlr_box usable_box = {
      .x      = layer_surface->output->scene_output->x,
      .y      = layer_surface->output->scene_output->y,
      .width  = layer_surface->output->wlr_output->width,
      .height = layer_surface->output->wlr_output->height,
  };
  wlr_box usable_area = {0, 0, 0, 0};
  wlr_output_effective_resolution(
      output, &usable_area.width, &usable_area.height);
  const struct wlr_box full_area = usable_area;

  wlr_log(WLR_INFO,
          "Break Height: %i, Width: %i, x: %i, y: %i",
          usable_area.height,
          usable_area.width,
          usable_area.x,
          usable_area.y);
  wlr_scene_layer_surface_v1_configure(
      layer_surface->scene, &full_area, &usable_area);
  wlr_log(WLR_INFO,
          "Break Height: %i, Width: %i, x: %i, y: %i",
          usable_area.height,
          usable_area.width,
          usable_area.x,
          usable_area.y);
}

void handle_layer_surface_map(struct wl_listener*    listener,
                              [[maybe_unused]] void* data) {
  struct gfwl_layer_surface* gfwl_layer_surface =
      wl_container_of(listener, gfwl_layer_surface, map);

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
  wlr_log(WLR_INFO, "Commited Layer Surface Change");
  struct gfwl_layer_surface* gfwl_layer_surface =
      wl_container_of(listener, gfwl_layer_surface, commit);

  if (gfwl_layer_surface->wlr_layer_surface->initial_commit) {
    get_box_from_anchors(gfwl_layer_surface);
  }
}

void handle_new_layer_shell_surface(struct wl_listener* listener, void* data) {
  class GfServer* server =
      wl_container_of(listener, server, new_layer_shell_surface);
  if (!server) {
    wlr_log(WLR_ERROR, "No server from listener.");
    return;
  }

  struct wlr_layer_surface_v1* wlr_layer_surface = (wlr_layer_surface_v1*)data;
  if (!wlr_layer_surface) {
    wlr_log(WLR_ERROR, "No layer surface.");
    return;
  }

  struct gfwl_layer_surface* gfwl_layer_surface =
      (struct gfwl_layer_surface*)calloc(1, sizeof(*gfwl_layer_surface));
  if (!gfwl_layer_surface) {
    wlr_log(WLR_ERROR, "No gfwl layer surface.");
    return;
  }
  gfwl_layer_surface->wlr_layer_surface = wlr_layer_surface;
  gfwl_layer_surface->server            = server;

  if (wlr_layer_surface->output) {
    for (auto output : server->outputs) {
      if (output->wlr_output == wlr_layer_surface->output) {
        gfwl_layer_surface->output = output;
        break;
      }
    }
  } else {
    auto gfwl_output           = server->focused_output;
    gfwl_layer_surface->output = gfwl_output;
    wlr_layer_surface->output  = gfwl_output->wlr_output;
  }

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
