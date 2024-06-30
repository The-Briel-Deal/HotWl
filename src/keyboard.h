#include "wayland-server-core.h"
#include <wayland-util.h>
#include <wlr/types/wlr_seat.h>

struct gfwl_keyboard {
  struct wl_list link;
  struct gfwl_server *server;
  struct wlr_keyboard *wlr_keyboard;

  struct wl_listener modifiers;
  struct wl_listener key;
  struct wl_listener destroy;
};

static void keyboard_handle_modifiers(struct wl_listener *listener, void *data);
static void keyboard_handle_key(struct wl_listener *listener, void *data);
static void keyboard_handle_destroy(struct wl_listener *listener, void *data);

void server_new_keyboard(struct gfwl_server *server,
                                struct wlr_input_device *device);
