#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/types/wlr_layer_shell_v1.h>

struct gfwl_layer_surface {
  struct wl_list link;
  struct gfwl_output *output;
  struct wlr_layer_surface_v1 *wlr_layer_surface;
  struct gfwl_server *server;
  struct wl_listener map;
  struct wl_listener commit;
};

void handle_new_layer_shell_surface(struct wl_listener *listener, void *data);
void handle_layer_surface_commit(struct wl_listener *listener, void *data);
void handle_layer_surface_map(struct wl_listener *listener, void *data);
