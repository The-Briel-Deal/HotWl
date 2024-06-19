#include <layer_shell.h>
#include <output.h>
#include <scene.h>
#include <server.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/util/log.h>
#include <stdlib.h>
#include <wayland-server-core.h>

void handle_layer_surface_map(struct wl_listener *listener, void *data) {
  wlr_log(WLR_INFO, "GFLOG: handle_layer_surface_map started.");

  struct gfwl_layer_surface *gfwl_layer_surface =
      wl_container_of(listener, gfwl_layer_surface, map);
  struct gfwl_server *server = gfwl_layer_surface->server;

  // TODO: Remove launchers in favor of somewhere else to save layer surface nodes.
  wl_list_insert(&server->launchers, &gfwl_layer_surface->link);

  struct wlr_box full_area = {0};
  struct wlr_box usable_area = {0};
  wlr_scene_layer_surface_v1_configure(gfwl_layer_surface->scene, &full_area, &usable_area);

  wlr_log(WLR_INFO, "GFLOG: handle_layer_surface_map finished.");
}

void handle_layer_surface_commit(struct wl_listener *listener, void *data) {
  wlr_log(WLR_INFO, "GFLOG: handle_layer_surface_commit started.");
  struct wlr_surface *wlr_surface = data;
  struct gfwl_layer_surface *gfwl_layer_surface =
      wl_container_of(listener, gfwl_layer_surface, commit);

  if (gfwl_layer_surface->wlr_layer_surface->initial_commit) {
    wlr_log(WLR_INFO, "GFLOG: layer_surface is initialized, configuring now.");
    wlr_layer_surface_v1_configure(gfwl_layer_surface->wlr_layer_surface, 0, 0);
//    Add Layer Scene Surface Configuring. TODO:
//    wlr_scene_layer_surface_v1_configure();
  } else {
    wlr_log(WLR_INFO,
            "GFLOG: layer_surface not yet initialized, doing nothing.");
  }

  wlr_log(WLR_INFO, "GFLOG: handle_layer_surface_commit finished.");
}

void handle_new_layer_shell_surface(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "GFLOG: handle_new_layer_shell_surface started.");
  // Grab our server (parent of the listener).
  struct gfwl_server *server =
      wl_container_of(listener, server, new_layer_shell_surface);
  if (!server) {
    wlr_log(WLR_ERROR, "No server from listener.");
    return;
  }

  // Grab layer surface.
  struct wlr_layer_surface_v1 *wlr_layer_surface = data;
  if (!wlr_layer_surface) {
    wlr_log(WLR_ERROR, "No layer surface.");
    return;
  }

  // Dynamically allocate a new layer surface wrapper and save callback args.
  struct gfwl_layer_surface *gfwl_layer_surface =
      calloc(1, sizeof(*gfwl_layer_surface));
  if (!gfwl_layer_surface) {
    wlr_log(WLR_ERROR, "No gfwl layer surface.");
    return;
  }
  gfwl_layer_surface->wlr_layer_surface = wlr_layer_surface;
  gfwl_layer_surface->server = server;

  // Check for layer surface output.
  if (!wlr_layer_surface->output) {
    wlr_log(WLR_INFO, "No output on layer surface.");
    struct gfwl_output *output = NULL;
    struct wlr_seat *seat = server->seat;
    if (!seat) {
      wlr_log(WLR_ERROR, "No seat.");
      return;
    }

    // Get first output.
    struct gfwl_output *gfwl_output =
        wl_container_of(server->outputs.next, gfwl_output, link);
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
  struct gfwl_output *gfwl_output =
      wl_container_of(wlr_layer_surface->output, gfwl_output, wlr_output);
  if (!gfwl_output) {
    wlr_log(WLR_ERROR, "No gfwl_output is parent of wlr_output.");
    return;
  }

  gfwl_layer_surface->output = gfwl_output;

  enum zwlr_layer_shell_v1_layer layer_type = wlr_layer_surface->pending.layer;

  gfwl_output->layers.shell_top =
      wlr_scene_tree_create(server->scene_roots->layer_roots.shell_top);

  // Create the scene.
  struct wlr_scene_layer_surface_v1 *scene_surface =
      wlr_scene_layer_surface_v1_create(gfwl_output->layers.shell_top,
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

  wlr_log(WLR_INFO, "GFLOG: handle_new_layer_shell_surface finished.");
}
