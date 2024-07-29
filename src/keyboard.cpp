#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <iterator>
#include <keyboard.hpp>
#include <memory>
#include <server.hpp>
#include <string>
#include <sys/types.h>
#include <tiling/focus.hpp>
#include <unistd.h>
#include <vector>
#include <wayland-server-core.h>
#include <wayland-server-protocol.h>
#include <wayland-util.h>
#include <wlr/types/wlr_seat.h>
#include <xkbcommon/xkbcommon.h>

#include "conf/config.hpp"
#include "output.hpp"
#include "tiling/container/base.hpp"
#include "tiling/container/toplevel.hpp"
#include "tiling/marks.hpp"
#include "tiling/state.hpp"
#include "wlr/types/wlr_input_device.h"
#include "wlr/types/wlr_keyboard.h"
#include "xdg_shell.hpp"

static void keyboard_handle_destroy(struct wl_listener*    listener,
                                    [[maybe_unused]] void* data) {
  /* This event is raised by the keyboard base wlr_input_device to signal
   * the destruction of the wlr_keyboard. It will no longer receive events
   * and should be destroyed.
   */
  struct GfKeyboard* keyboard = wl_container_of(listener, keyboard, destroy);
  wl_list_remove(&keyboard->modifiers.link);
  wl_list_remove(&keyboard->key.link);
  wl_list_remove(&keyboard->destroy.link);
  wl_list_remove(&keyboard->link);
  free(keyboard);
}

static void keyboard_handle_modifiers(struct wl_listener*    listener,
                                      [[maybe_unused]] void* data) {
  /* This event is raised when a modifier key, such as shift or alt, is
   * pressed. We simply communicate this to the client. */
  struct GfKeyboard* keyboard = wl_container_of(listener, keyboard, modifiers);
  /*
   * A seat can only have one keyboard, but this is a limitation of the
   * Wayland protocol - not wlroots. We assign all connected keyboards to the
   * same seat. You can swap out the underlying wlr_keyboard like this and
   * wlr_seat handles this transparently.
   */
  wlr_seat_set_keyboard(keyboard->server->seat, keyboard->wlr_keyboard);
  /* Send modifiers to the client. */
  wlr_seat_keyboard_notify_modifiers(keyboard->server->seat,
                                     &keyboard->wlr_keyboard->modifiers);
}

static void launch_app(const std::string& path) {
  pid_t pid = fork();
  if (pid == 0) {
    execlp(path.c_str(), path.c_str());
  }
}

static bool handle_keybinding(class GfServer* server, xkb_keysym_t sym) {
  /*
   * Here we handle compositor keybindings. This is when the compositor is
   * processing keys, rather than passing them on to the client for its own
   * processing.
   *
   * This function assumes your mod is pressed.
   */

  if (sym == server->config.keybinds.new_term) {
    launch_app("kitty");
  } else if (sym == server->config.keybinds.launcher) {
    launch_app("fuzzel");
  } else if (sym == server->config.keybinds.exit) {
    wl_display_terminate(server->wl_display);
  } else if (sym == server->config.keybinds.tiling_focus_left) {
    tiling_focus_move_in_dir(GFWL_TILING_FOCUS_LEFT,
                             server->focused_output->tiling_state);
  } else if (sym == server->config.keybinds.tiling_focus_down) {
    tiling_focus_move_in_dir(GFWL_TILING_FOCUS_DOWN,
                             server->focused_output->tiling_state);
  } else if (sym == server->config.keybinds.tiling_focus_up) {
    tiling_focus_move_in_dir(GFWL_TILING_FOCUS_UP,
                             server->focused_output->tiling_state);
  } else if (sym == server->config.keybinds.tiling_focus_right) {
    tiling_focus_move_in_dir(GFWL_TILING_FOCUS_RIGHT,
                             server->focused_output->tiling_state);
  } else if (sym == server->config.keybinds.flip_split_direction) {
    server->focused_output->tiling_state->flip_split_direction();
  } else if (sym == server->config.keybinds.next_monitor) {
    // TODO(gabe): Make these a helper function.
    auto next_output = std::next(std::find(server->outputs.begin(),
                                           server->outputs.end(),
                                           server->focused_output));
    if (next_output != server->outputs.end() && *next_output != nullptr) {
      server->focused_output = *next_output;
    }
  } else if (sym == server->config.keybinds.prev_monitor) {
    auto curr_output = std::find(
        server->outputs.begin(), server->outputs.end(), server->focused_output);
    auto prev_output = std::prev(curr_output);
    if (curr_output != server->outputs.begin() && *prev_output != nullptr) {
      server->focused_output = *prev_output;
    }
  } else if (sym == server->config.keybinds.close_surface &&
             !server->active_toplevel_container.empty() &&
             !server->active_toplevel_container.front().expired()) {
    server->active_toplevel_container.front().lock()->close();
  }

  return true;
}

static void keyboard_handle_key(struct wl_listener* listener, void* data) {
  /* This event is raised when a key is pressed or released. */
  struct GfKeyboard* keyboard = wl_container_of(listener, keyboard, key);
  class GfServer*    server   = keyboard->server;
  struct wlr_keyboard_key_event* event =
      static_cast<wlr_keyboard_key_event*>(data);
  struct wlr_seat* seat = server->seat;

  /* Translate libinput keycode -> xkbcommon */
  uint32_t keycode = event->keycode + 8;
  /* Get a list of keysyms based on the keymap for this keyboard */
  const xkb_keysym_t* syms;
  int                 nsyms =
      xkb_state_key_get_syms(keyboard->wlr_keyboard->xkb_state, keycode, &syms);

  bool     handled   = false;
  uint32_t modifiers = wlr_keyboard_get_modifiers(keyboard->wlr_keyboard);

  if ((modifiers & server->config.keybinds.modmask) &&
      event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
    for (int i = 0; i < nsyms; i++) {
      handled = handle_keybinding(server, syms[i]);
    }
  }
  if ((modifiers == server->config.keybinds.new_mark_modmask) &&
      event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
    for (int i = 0; i < nsyms; i++) {
      // TODO: Make active toplevel container only have toplevels
      g_Marks.new_mark(syms[i],
                       std::dynamic_pointer_cast<GfContainerToplevel>(
                           server->active_toplevel_container.front().lock()));
    }
  }
  if ((modifiers == server->config.keybinds.goto_mark_modmask) &&
      event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
    for (int i = 0; i < nsyms; i++) {
      // TODO: Make active toplevel container only have toplevels
      auto container = g_Marks.get_container_from_keysym(syms[i]);
      if (container.has_value() && !container->expired()) {
        auto locked = container->lock();
        locked->set_focused_toplevel_container();
      }
    }
  }

  if (!handled) {
    /* Otherwise, we pass it along to the client. */
    wlr_seat_set_keyboard(seat, keyboard->wlr_keyboard);
    wlr_seat_keyboard_notify_key(
        seat, event->time_msec, event->keycode, event->state);
  }
}

void server_new_keyboard(class GfServer*          server,
                         struct wlr_input_device* device) {
  struct wlr_keyboard* wlr_keyboard = wlr_keyboard_from_input_device(device);

  struct GfKeyboard*   keyboard =
      static_cast<GfKeyboard*>(calloc(1, sizeof(*keyboard)));
  keyboard->server       = server;
  keyboard->wlr_keyboard = wlr_keyboard;

  /* We need to prepare an XKB keymap and assign it to the keyboard. This
   * assumes the defaults (e.g. layout = "us"). */
  struct xkb_context* context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  struct xkb_keymap*  keymap =
      xkb_keymap_new_from_names(context, nullptr, XKB_KEYMAP_COMPILE_NO_FLAGS);

  wlr_keyboard_set_keymap(wlr_keyboard, keymap);
  xkb_keymap_unref(keymap);
  xkb_context_unref(context);
  wlr_keyboard_set_repeat_info(wlr_keyboard, 25, 600);

  /* Here we set up listeners for keyboard events. */
  keyboard->modifiers.notify = keyboard_handle_modifiers;
  wl_signal_add(&wlr_keyboard->events.modifiers, &keyboard->modifiers);
  keyboard->key.notify = keyboard_handle_key;
  wl_signal_add(&wlr_keyboard->events.key, &keyboard->key);
  keyboard->destroy.notify = keyboard_handle_destroy;
  wl_signal_add(&device->events.destroy, &keyboard->destroy);

  wlr_seat_set_keyboard(server->seat, keyboard->wlr_keyboard);

  /* And add the keyboard to our list of keyboards */
  wl_list_insert(&server->keyboards, &keyboard->link);
}
