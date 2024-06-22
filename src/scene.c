#include <layer_shell.h>
#include <scene.h>

void gfwl_output_scene_layers_init(struct gfwl_scene_layers *layers,
                                   struct gfwl_scene_layers *parent_layers) {
  layers->base = wlr_scene_tree_create(parent_layers->base);
  layers->top = wlr_scene_tree_create(parent_layers->top);
};
