#pragma once
#include <utility>

#include "base.hpp"

class GfContainerRoot : public GfContainer {
public:
  explicit GfContainerRoot(GfServer&                    server,
                           const gfwl_container_type    e_type,
                           std::weak_ptr<GfTilingState> tiling_state) :
      GfContainer(server, e_type, std::move(tiling_state)){};

  std::weak_ptr<GfContainer> insert(GfToplevel* to_insert) final;
  void                       parse_containers() final;
  void                       close() final;

private:
  void set_to_output_size();
};
