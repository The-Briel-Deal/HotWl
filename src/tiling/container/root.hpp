#pragma once
#include "base.hpp"

class GfContainerRoot : public GfContainer {
public:
  explicit GfContainerRoot(GfServer&                    server,
                           const gfwl_container_type    e_type,
                           std::weak_ptr<GfTilingState> tiling_state) :
      GfContainer(server, e_type, tiling_state){};

  std::weak_ptr<GfContainer> insert(gfwl_toplevel* to_insert);
  void                       parse_containers();
  void                       close();

private:
  void set_to_output_size();
};
