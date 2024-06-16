#include "wlr/util/log.h"
#include <wayland-server-core.h>
#include "server.h"
#include "layer_shell.h"

void handle_layer_surface_map(struct wl_listener *listener, void *data) {
  wlr_log(WLR_INFO, "GFLOG: handle_layer_surface_map started.");

  struct gfwl_layer_surface *gfwl_layer_surface =
      wl_container_of(listener, gfwl_layer_surface, map);
  struct gfwl_server *server = gfwl_layer_surface->server;

  // Remove launchers in favor of somewhere else to save layer surface nodes.
  wl_list_insert(&server->launchers, &gfwl_layer_surface->link);

  wlr_log(WLR_INFO, "GFLOG: handle_layer_surface_map finished.");
}
