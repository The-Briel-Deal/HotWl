#pragma once
#include <utility>

#include "base.hpp"

class GfContainerSplit : public GfContainer {
public:
  explicit GfContainerSplit(GfServer&                    server,
                            std::weak_ptr<GfContainer>   parent,
                            const gfwl_container_type    e_type,
                            std::weak_ptr<GfTilingState> tiling_state) :
      GfContainer(server, std::move(parent), e_type, std::move(tiling_state)){};
};
