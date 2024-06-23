#include <scene.h>
#include <wayland-util.h>

// WIP: Returns the top scene tree of the buffer node. If no buffer nodes
// return null. The iteration portion currently seems to be broken as it seems
// to be running over parts of the tree multiple times (or maybe I am creating
// un-necessary nodes in the tree when making toplevels.
struct wlr_scene_tree *get_top_buffer_scene(struct wlr_scene_tree *tree_in) {
  struct wlr_scene_node *node_iter;
  struct wl_list *children_list = &tree_in->children;

  wl_list_for_each(node_iter, children_list, link) {
    if (node_iter->type == WLR_SCENE_NODE_TREE) {
      struct wlr_scene_tree *tree_iter =
          wl_container_of(node_iter, tree_iter, node);
      get_top_buffer_scene(tree_iter);
    }
    if (node_iter->type == WLR_SCENE_NODE_BUFFER) {
      struct wlr_scene_buffer *buffer =
          wl_container_of(node_iter, buffer, node);
    }
  }
  return NULL;
}
