#pragma once

#include "tiling/container/base.hpp"
#include "tiling/container/toplevel.hpp"
#include <map>
#include <memory>
#include <optional>
#include <xkbcommon/xkbcommon.h>
class GfMarks {
public:
  std::map<xkb_keysym_t, std::weak_ptr<GfContainerToplevel>> marks;
  void new_mark(xkb_keysym_t, std::weak_ptr<GfContainerToplevel>);
  std::optional<std::weak_ptr<GfContainerToplevel>>
      get_container_from_keysym(xkb_keysym_t);
};

inline GfMarks g_Marks;
