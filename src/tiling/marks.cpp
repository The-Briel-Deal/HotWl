#include "marks.hpp"
#include "tiling/container/toplevel.hpp"

#include <memory>
#include <optional>
#include <utility>

void GfMarks::new_mark(xkb_keysym_t                       keysym,
                       std::weak_ptr<GfContainerToplevel> container) {
  marks[keysym] = std::move(container);
}

std::optional<std::weak_ptr<GfContainerToplevel>>
GfMarks::get_container_from_keysym(xkb_keysym_t keysym) {
  if (marks.contains(keysym)) {
    return marks[keysym];
  }
  return std::nullopt;
}
