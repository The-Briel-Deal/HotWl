#include <wlr/types/wlr_scene.h>
struct gfwl_scene {
  // A scene wrapper so I can grab certain parts of the tree easily.
  struct {
    struct wlr_scene_tree *base;
    struct wlr_scene_tree *top;
  } layer;
  struct wlr_scene *root;
};

struct wlr_scene_tree *get_top_scene_node(struct wlr_scene_tree *node_in);
