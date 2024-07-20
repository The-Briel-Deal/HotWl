#include <input.hpp>
#include <layer_shell.hpp>
#include <output.hpp>
#include <pointer.hpp>
#include <scene.hpp>
#include <server.hpp>

GfServer::GfServer() {

  wl_display = wl_display_create();
  backend = wlr_backend_autocreate(wl_display_get_event_loop(wl_display), NULL);
  renderer      = wlr_renderer_autocreate(backend);
  allocator     = wlr_allocator_autocreate(backend, renderer);
  output_layout = wlr_output_layout_create(wl_display);
  scene_layout  = wlr_scene_attach_output_layout(scene.root, output_layout);
  layer_shell   = wlr_layer_shell_v1_create(wl_display, 1);
  xdg_shell     = wlr_xdg_shell_create(wl_display, 3);
  cursor        = wlr_cursor_create();
  cursor_mgr    = wlr_xcursor_manager_create(NULL, 24);

  wlr_renderer_init_wl_display(renderer, wl_display);

  /* This creates some hands-off wlroots interfaces. */
  wlr_compositor_create(wl_display, 5, renderer);
  wlr_subcompositor_create(wl_display);
  wlr_data_device_manager_create(wl_display);
  wlr_cursor_attach_output_layout(cursor, output_layout);

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

  cursor_mode          = TINYWL_CURSOR_PASSTHROUGH;
  cursor_motion.notify = server_cursor_motion;
  wl_signal_add(&cursor->events.motion, &cursor_motion);
  cursor_motion_absolute.notify = server_cursor_motion_absolute;
  wl_signal_add(&cursor->events.motion_absolute, &cursor_motion_absolute);
  cursor_button.notify = server_cursor_button;
  wl_signal_add(&cursor->events.button, &cursor_button);
  cursor_axis.notify = server_cursor_axis;
  wl_signal_add(&cursor->events.axis, &cursor_axis);
  cursor_frame.notify = server_cursor_frame;
  wl_signal_add(&cursor->events.frame, &cursor_frame);

  /*
   * Configures a seat, which is a single "seat" at which a user sits and
   * operates the computer. This conceptually includes up to one keyboard,
   * pointer, touch, and drawing tablet device. We also rig up a listener to
   * let us know when new input devices are available on the backend.
   */
  wl_list_init(&keyboards);
  new_input.notify = server_new_input;
  wl_signal_add(&backend->events.new_input, &new_input);
  seat                  = wlr_seat_create(wl_display, "seat0");
  request_cursor.notify = seat_request_cursor;
  wl_signal_add(&seat->events.request_set_cursor, &request_cursor);
  request_set_selection.notify = seat_request_set_selection;
  wl_signal_add(&seat->events.request_set_selection, &request_set_selection);
}
