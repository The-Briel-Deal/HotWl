#pragma once
#include "wlr/util/box.h"
#include <scene.hpp>
#include <tiling/focus.hpp>
#include <wayland-server-core.h>

enum gfwl_cursor_mode {
  TINYWL_CURSOR_PASSTHROUGH,
  TINYWL_CURSOR_MOVE,
  TINYWL_CURSOR_RESIZE,
};

// This server struct is for holding our compositors state.
struct gfwl_server {
  // This is the core object in wayland. Its a special singleton.
  struct wl_display *wl_display;
  // Provides a set of input and output devices. Has signals for when inputs
  // and outputs are added.
  struct wlr_backend *backend;
  // A struct to access the renderer. Can give you a file descriptor pointing
  // to the DRM device.
  struct wlr_renderer *renderer;
  // The allocator allocates memory for pixel buffers.
  struct wlr_allocator *allocator;
  struct gfwl_scene *scene;
  struct wlr_scene_output_layout *scene_layout;

  struct wlr_xdg_shell *xdg_shell;
  struct wl_listener new_xdg_toplevel;
  struct wl_listener new_xdg_popup;
  struct wl_list toplevels;
  struct gfwl_tiling_state tiling_state;

  struct wlr_layer_shell_v1 *layer_shell;
  struct wl_listener new_layer_shell_surface;

  struct wlr_cursor *cursor;
  struct wlr_xcursor_manager *cursor_mgr;
  struct wl_listener cursor_motion;
  struct wl_listener cursor_motion_absolute;
  struct wl_listener cursor_button;
  struct wl_listener cursor_axis;
  struct wl_listener cursor_frame;

  struct wlr_seat *seat;
  struct wl_listener new_input;
  struct wl_listener request_cursor;
  struct wl_listener request_set_selection;
  struct wl_list keyboards;
  struct gfwl_toplevel *grabbed_toplevel;
  double grab_x, grab_y;
  struct wlr_box grab_geobox;
  uint32_t resize_edges;
  enum gfwl_cursor_mode cursor_mode;

  struct wlr_output_layout *output_layout;
  struct wl_list outputs;
  struct wl_listener new_output;
};
