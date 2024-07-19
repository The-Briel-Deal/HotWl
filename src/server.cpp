#include "layer_shell.hpp"
#include "scene.hpp"
#include <server.hpp>

gfwl_server::gfwl_server() {

  wl_display = wl_display_create();
  backend = wlr_backend_autocreate(wl_display_get_event_loop(wl_display), NULL);
  renderer = wlr_renderer_autocreate(backend);
  allocator = wlr_allocator_autocreate(backend, renderer);
  output_layout = wlr_output_layout_create(wl_display);
  scene_layout = wlr_scene_attach_output_layout(scene.root, output_layout);
  layer_shell = wlr_layer_shell_v1_create(wl_display, 1);
  xdg_shell = wlr_xdg_shell_create(wl_display, 3);

  wlr_renderer_init_wl_display(renderer, wl_display);

  /* This creates some hands-off wlroots interfaces. */
  wlr_compositor_create(wl_display, 5, renderer);
  wlr_subcompositor_create(wl_display);
  wlr_data_device_manager_create(wl_display);

  // TODO: I don't think I need this anymore, I should be using vectors.
  wl_list_init(&toplevels);

  /* Signals */
  /* output */
  new_output.notify = server_new_output;
  wl_signal_add(&backend->events.new_output, &new_output);

  /* xdg_shell */
  new_xdg_toplevel.notify = server_new_xdg_toplevel;
  wl_signal_add(&xdg_shell->events.new_toplevel, &new_xdg_toplevel);

  new_xdg_popup.notify = server_new_xdg_popup;
  wl_signal_add(&xdg_shell->events.new_popup, &new_xdg_popup);

  /* layer_shell */
  new_layer_shell_surface.notify = handle_new_layer_shell_surface;
  wl_signal_add(&layer_shell->events.new_surface, &new_layer_shell_surface);
}
