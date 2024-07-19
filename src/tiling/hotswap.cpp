#include "hotswap.hpp"
#include "container.hpp"
#include <memory>

std::weak_ptr<GfContainer> GfHotswapState::get(xkb_keysym_t key) {
  auto container = container_map[key];
  return container;
};

xkb_keysym_t GfHotswapState::insert(std::weak_ptr<GfContainer> container) {};

bool GfHotswapState::remove(std::weak_ptr<GfContainer> container) {};
