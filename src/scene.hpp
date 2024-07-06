#pragma once
#include <includes.hpp>
#include <wlr/util/box.h>
struct gfwl_server;

struct gfwl_scene {
  // A scene wrapper so I can grab certain parts of the tree easily.
  struct {
    struct wlr_scene_tree *base;
    struct wlr_scene_tree *top;
  } layer;
  struct wlr_scene *root;
};
