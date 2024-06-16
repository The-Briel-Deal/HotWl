#include <wayland-server-core.h>
#include <wayland-util.h>

struct gfwl_output {
  // For wlr_layer_surface
  struct {
    struct wlr_scene_tree *shell_background;
    struct wlr_scene_tree *shell_bottom;
    struct wlr_scene_tree *tiling;
    struct wlr_scene_tree *fullscreen;
    struct wlr_scene_tree *shell_top;
    struct wlr_scene_tree *shell_overlay;
    struct wlr_scene_tree *session_lock;
  } layers;
  struct wl_list link;
  struct gfwl_server *server;
  struct wlr_output *wlr_output;
  struct wl_listener frame;
  struct wl_listener request_state;
  struct wl_listener destroy;
};

