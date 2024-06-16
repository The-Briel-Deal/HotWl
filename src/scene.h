
struct gfwl_scene {
  // A scene wrapper so I can grab certain parts of the tree easily.
  struct {
    struct wlr_scene_tree *shell_background;
    struct wlr_scene_tree *shell_bottom;
    struct wlr_scene_tree *tiling;
    struct wlr_scene_tree *fullscreen;
    struct wlr_scene_tree *shell_top;
    struct wlr_scene_tree *shell_overlay;
    struct wlr_scene_tree *session_lock;
  } layer_roots;
  struct wlr_scene *root;
};
