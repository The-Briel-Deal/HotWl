/*   Hotswap
 * Hotswapping as a feature is the idea of mapping a key to a every window in
 * the tiling trees. Then when you press the hotswap button it will display
 * said keys on the windows and allow you to press that key to quickly go to
 * that window. */

#include "tiling/container.hpp"
#include <map>
#include <memory>
#include <xkbcommon/xkbcommon.h>

class GfHotswapState {
public:
  /* Returns container mapped to that key */
  std::weak_ptr<GfContainer> get(xkb_keysym_t);
  /* Returns Newly Mapped Keysym */
  xkb_keysym_t insert(std::weak_ptr<GfContainer>);
  /* Returns whether or not it was found */
  bool remove(std::weak_ptr<GfContainer>);

private:
  std::map<xkb_keysym_t, std::weak_ptr<GfContainer>> container_map;
};
