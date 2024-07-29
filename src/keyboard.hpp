#include <wayland-server-core.h>
#include <wayland-util.h>
extern "C" {
#include <wlr/types/wlr_seat.h>
}

struct GfKeyboard {
  struct wl_list       link;
  class GfServer*      server;
  struct wlr_keyboard* wlr_keyboard;

  struct wl_listener   modifiers;
  struct wl_listener   key;
  struct wl_listener   destroy;
};

void server_new_keyboard(class GfServer*          server,
                         struct wlr_input_device* device);
