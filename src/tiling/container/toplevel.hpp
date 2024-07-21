#include "base.hpp"

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
