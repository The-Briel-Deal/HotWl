#pragma once
#include "state.hpp"
#include "wlr/util/box.h"
#include "xdg_shell.hpp"
#include <includes.hpp>
#include <memory>
#include <tiling/container/base.hpp>

class GfContainerRoot : public GfContainer {
public:
  explicit GfContainerRoot(
      GfServer& server,
      const gfwl_container_type
          e_type, /* TODO: Replace with always being GFWL_CONTAINER_ROOT */
      std::weak_ptr<GfTilingState> tiling_state) :
      GfContainer(server, e_type, tiling_state){};

  std::weak_ptr<GfContainer> insert(gfwl_toplevel* toplevel);
  void                       parse_containers();
  void                       close();

private:
  void set_to_output_size();
};

class GfContainerToplevel : public GfContainer {
public:
  explicit GfContainerToplevel(gfwl_toplevel* const         toplevel,
                               GfServer&                    server,
                               std::weak_ptr<GfContainer>   parent,
                               std::weak_ptr<GfTilingState> tiling_state) :
      GfContainer(server, parent, GFWL_CONTAINER_TOPLEVEL, tiling_state),
      toplevel(toplevel){};
  ~GfContainerToplevel();

  gfwl_toplevel* const toplevel;

  void                 set_container_box(struct wlr_box box_in);
};

class GfContainerSplit : public GfContainer {
public:
  explicit GfContainerSplit(GfServer&                    server,
                            std::weak_ptr<GfContainer>   parent,
                            const gfwl_container_type    e_type,
                            std::weak_ptr<GfTilingState> tiling_state) :
      GfContainer(server, parent, e_type, tiling_state){};
};

void set_focused_toplevel_container(std::weak_ptr<GfContainer> container);
