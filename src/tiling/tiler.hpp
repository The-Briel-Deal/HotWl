/* GfTiler takes in toplevels and adjusts their position and size to be in a
 * tiling layout. */
#include <memory>

enum gfwl_tiling_focus_direction : char {
  GFWL_TILING_FOCUS_LEFT = 1,
  GFWL_TILING_FOCUS_DOWN,
  GFWL_TILING_FOCUS_UP,
  GFWL_TILING_FOCUS_RIGHT,
};
struct GfToplevel;

class GfTiler {
public:
  /* Give ownership of the toplevel to the tiler. */
  GfToplevel* insert_toplevel(std::unique_ptr<GfToplevel>);
  /* Retrieve ownership of the toplevel from the tiler. */
  std::unique_ptr<GfToplevel> remove_toplevel(GfToplevel*);

  /* Ask the tiler to move focus in a certain direction. */
  void move_focus_in_dir(gfwl_tiling_focus_direction);
  /* Ask the tiler to move focus in a certain direction. */
  void focus_toplevel(GfToplevel*);
};
