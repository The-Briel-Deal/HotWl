#pragma once
#include <conf/config.hpp>
#include <deque>
#include <memory>
#include <output.hpp>
#include <scene.hpp>
#include <tiling/container.hpp>
#include <vector>
#include <wayland-server-core.h>

// TODO: fix prefix.
enum gfwl_cursor_mode {
  TINYWL_CURSOR_PASSTHROUGH,
  TINYWL_CURSOR_MOVE,
  TINYWL_CURSOR_RESIZE,
};

// This server struct is for holding our compositors state.
class GfServer {
public:
  GfServer();
  // This is the core object in wayland. Its a special singleton.
  struct wl_display* wl_display;
  // Provides a set of input and output devices. Has signals for when inputs
  // and outputs are added.
  struct wlr_backend* backend;
  // A struct to access the renderer. Can give you a file descriptor pointing
  // to the DRM device.
  struct wlr_renderer* renderer;
  // The allocator allocates memory for pixel buffers.
  struct wlr_allocator*                     allocator;
  struct GfScene                         scene;
  struct wlr_scene_output_layout*           scene_layout;

  struct wlr_xdg_shell*                     xdg_shell;
  struct wl_listener                        new_xdg_toplevel;
  struct wl_listener                        new_xdg_popup;
  struct wl_list                            toplevels;
  std::deque<std::weak_ptr<GfContainer>>    active_toplevel_container;

  struct wlr_layer_shell_v1*                layer_shell;
  struct wl_listener                        new_layer_shell_surface;

  struct wlr_cursor*                        cursor;
  struct wlr_xcursor_manager*               cursor_mgr;
  struct wl_listener                        cursor_motion;
  struct wl_listener                        cursor_motion_absolute;
  struct wl_listener                        cursor_button;
  struct wl_listener                        cursor_axis;
  struct wl_listener                        cursor_frame;

  struct wlr_seat*                          seat;
  struct wl_listener                        new_input;
  struct wl_listener                        request_cursor;
  struct wl_listener                        request_set_selection;
  struct wl_list                            keyboards;
  struct GfToplevel*                     grabbed_toplevel;
  double                                    grab_x, grab_y;
  struct wlr_box                            grab_geobox;
  uint32_t                                  resize_edges;
  enum gfwl_cursor_mode                     cursor_mode;

  struct wlr_output_layout*                 output_layout;
  wlr_xdg_output_manager_v1*                xdg_output_manager_v1;
  std::shared_ptr<GfOutput>              focused_output;
  std::vector<std::shared_ptr<GfOutput>> outputs;
  struct wl_listener                        new_output;

  GfConfig                                  config;
};

inline GfServer g_Server;
