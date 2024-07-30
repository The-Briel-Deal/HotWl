#pragma once
#include <includes.hpp>
#include <memory>
#include <server.hpp>
#include <wayland-server-core.h>
#include <wayland-util.h>

class GfServer;
struct GfOutput;

struct GfLayerSurface {
  struct wl_list                     link;
  std::shared_ptr<GfOutput>          output;
  struct wlr_scene_layer_surface_v1* scene;
  struct wlr_layer_surface_v1*       wlr_layer_surface;
  struct wlr_surface*                prev_focused;
  GfServer*                          server;
  struct wl_listener                 map;
  struct wl_listener                 unmap;
  struct wl_listener                 commit;
};

void handle_new_layer_shell_surface(struct wl_listener* listener, void* data);
void handle_layer_surface_commit(struct wl_listener* listener, void* data);
void handle_layer_surface_map(struct wl_listener* listener, void* data);
