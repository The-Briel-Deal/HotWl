#include "scene.hpp"
#include <server.hpp>

gfwl_server::gfwl_server() {

  wl_display = wl_display_create();
  backend = wlr_backend_autocreate(wl_display_get_event_loop(wl_display), NULL);
  renderer = wlr_renderer_autocreate(backend);
  allocator = wlr_allocator_autocreate(backend, renderer);
  output_layout = wlr_output_layout_create(wl_display);
  scene_layout = wlr_scene_attach_output_layout(scene.root, output_layout);

  wlr_renderer_init_wl_display(renderer, wl_display);

  /* This creates some hands-off wlroots interfaces. */
  wlr_compositor_create(wl_display, 5, renderer);
  wlr_subcompositor_create(wl_display);
  wlr_data_device_manager_create(wl_display);

  // TODO: Ideally these signals would be in their respective classes.
  new_output.notify = server_new_output;
  wl_signal_add(&backend->events.new_output, &new_output);
}
