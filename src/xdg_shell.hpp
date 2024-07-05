#pragma once
#include "scene.hpp"
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xdg_shell.h>

struct gfwl_toplevel {
  struct wl_list link;
  struct gfwl_container *parent_container;
  struct gfwl_server *server;
  struct wlr_xdg_toplevel *xdg_toplevel;
  struct wlr_scene_tree *scene_tree;
  struct wlr_surface *prev_focused;
  struct wl_listener map;
  struct wl_listener unmap;
  struct wl_listener commit;
  struct wl_listener destroy;
  struct wl_listener request_move;
  struct wl_listener request_resize;
  struct wl_listener request_maximize;
  struct wl_listener request_fullscreen;
};

struct gfwl_popup {
  struct wlr_xdg_popup *xdg_popup;
  struct wl_listener commit;
  struct wl_listener destroy;
};

void server_new_xdg_popup(struct wl_listener *listener, void *data);

void focus_toplevel(struct gfwl_toplevel *toplevel,
                    struct wlr_surface *surface);
void server_new_xdg_toplevel(struct wl_listener *listener, void *data);
