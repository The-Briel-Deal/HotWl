#pragma once
#include <includes.hpp>
#include <wlr/util/box.h>
class GfServer;

struct GfScene {
  GfScene() {
    root = wlr_scene_create();

    // Create tiling first so its the lowest.
    layer.base = wlr_scene_tree_create(&root->tree);

    // Create shell_top after so that it displays over layer_roots.
    layer.top = wlr_scene_tree_create(&root->tree);
  }
  // A scene wrapper so I can grab certain parts of the tree easily.
  struct {
    struct wlr_scene_tree* base;
    struct wlr_scene_tree* top;
  } layer;
  struct wlr_scene* root;
};
