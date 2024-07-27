#pragma once
#include "wlr/util/box.h"
#include <memory>
#include <tiling/state.hpp>
#include <wayland-server-core.h>
#include <wayland-util.h>

struct gfwl_output {
  // For wlr_layer_surface
  struct wl_list                 link;
  GfServer*                      server;
  struct wlr_output*             wlr_output;
  struct wlr_scene_output*       scene_output;
  std::shared_ptr<GfTilingState> tiling_state =
      std::make_shared<GfTilingState>();
  struct wlr_output_layout_output* output_layout_output;
  struct wl_listener               frame;
  struct wl_listener               request_state;
  struct wl_listener               destroy;
  void                             set_usable_space(wlr_box box);
  wlr_box                          get_usable_space();

private:
  wlr_box usable_space = {0, 0, 0, 0};
};

void server_new_output(struct wl_listener* listener, void* data);

void focus_output_from_container(const std::shared_ptr<GfContainer>& container);

std::shared_ptr<gfwl_output>
get_output_from_container(const std::shared_ptr<GfContainer>& container);
