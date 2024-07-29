#pragma once
#include <tiling/state.hpp>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <memory>


#include "wlr/util/box.h"
#include "wlr/types/wlr_output_layout.h"
#include "wlr/types/wlr_scene.h"

class GfContainer;

struct GfOutput {
  // For wlr_layer_surface
  struct wl_list                 link;
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

  wlr_box                          usable_space = {0, 0, 0, 0};
};

void server_new_output(struct wl_listener* listener, void* data);

void focus_output_from_container(const std::shared_ptr<GfContainer>& container);

std::shared_ptr<GfOutput>
get_output_from_container(const std::shared_ptr<GfContainer>& container);
