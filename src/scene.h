struct gfwl_scene_layers {
  struct wlr_scene_tree *base;
  struct wlr_scene_tree *top;
};

void gfwl_scene_layers_init(struct gfwl_scene_layers *layers,
                            struct gfwl_scene_layers *parent_layers);

struct gfwl_scene {
  // A scene wrapper so I can grab certain parts of the tree easily.
  struct gfwl_scene_layers layer;
  struct wlr_scene *root;
};
