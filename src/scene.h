#include "server.h"
#include <wlr/types/wlr_scene.h>
#include <layer_shell.h>

struct gfwl_scene {
  // A scene wrapper so I can grab certain parts of the tree easily.
  struct {
    struct wlr_scene_tree *base;
    struct wlr_scene_tree *top;
  } layer;
  struct wlr_scene *root;
};

struct gfwl_scene_tree {
  struct gfwl_scene *p_gfwl_scene;
  struct wlr_scene_tree p_wlr_tree;
};

enum gfwl_scene_node_type {
  GFWL_SCENE_NODE_LAYER_SURFACE = 1,
  GFWL_SCENE_NODE_TOPLEVEL_SURFACE,
  GFWL_SCENE_NODE_TREE,
};

// A scene_node so I can grab things like gfwl_layer_surface, gfwl_toplevel,
// and gfwl_scene_tree without the pain of wl_container_of.
struct gfwl_scene_node {
  enum gfwl_scene_node_type e_type;
  struct wlr_scene_node s_wlr_node;

  struct gfwl_layer_surface *p_gfwl_layer_surface;
  struct gfwl_toplevel *p_gfwl_toplevel;
  struct gfwl_scene_tree *p_gfwl_tree;
};

enum gfwl_container_type {
  GFWL_CONTAINER_NODE = 1,
  GFWL_CONTAINER_TREE,
  GFWL_CONTAINER_VSPLIT,
  GFWL_CONTAINER_HSPLIT,
};

struct gfwl_container {
  enum gfwl_container_type e_type;

  struct gfwl_scene_node s_gfwl_node;
  struct gfwl_scene_tree s_gfwl_tree;
};

struct wlr_scene_tree *get_top_buffer_scene(struct wlr_scene_tree *node_in);
