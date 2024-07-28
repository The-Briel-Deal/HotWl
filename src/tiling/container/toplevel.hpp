#pragma once
#include <utility>

#include "base.hpp"

class GfContainerToplevel : public GfContainer {
public:
  explicit GfContainerToplevel(GfToplevel* const            toplevel,
                               GfServer&                    server,
                               std::weak_ptr<GfContainer>   parent,
                               std::weak_ptr<GfTilingState> tiling_state) :
      GfContainer(server, std::move(parent), GFWL_CONTAINER_TOPLEVEL, std::move(tiling_state)),
      toplevel(toplevel){};

  ~GfContainerToplevel();

  void              set_container_box(struct wlr_box box_in) final;

  GfToplevel* const toplevel;
};
