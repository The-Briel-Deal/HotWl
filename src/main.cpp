#include <cassert>
#include <conf/config.hpp>
#include <getopt.h>
#include <includes.hpp>
#include <input.hpp>
#include <keyboard.hpp>
#include <layer_shell.hpp>
#include <output.hpp>
#include <pointer.hpp>
#include <scene.hpp>
#include <server.hpp>

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>
#include <xdg_shell.hpp>
#include <xkbcommon/xkbcommon.h>

int main(int argc, char* argv[]) {
  wlr_log_init(WLR_DEBUG, nullptr);
  char* startup_cmd = nullptr;

  // Iterate through args until it finds a -s then save to startup_cmd.
  int c;
  while ((c = getopt(argc, argv, "s:h")) != -1) {
    switch (c) {
      case 's': startup_cmd = optarg; break;
      default: printf("Usage: %s [-s startup command]\n", argv[0]); return 0;
    }
  }
  if (optind < argc) {
    printf("Usage: %s [-s startup command]\n", argv[0]);
    return 0;
  }

  /* Create Server - This is where all important state is stored. */

  // TODO(gabe): Move these checks to another function.
  if (g_Server.backend == nullptr) {
    wlr_log(WLR_ERROR, "failed to create wlr_backend");
    return 1;
  }

  if (g_Server.renderer == nullptr) {
    wlr_log(WLR_ERROR, "failed to create wlr_renderer");
    return 1;
  }

  if (g_Server.allocator == nullptr) {
    wlr_log(WLR_ERROR, "failed to create wlr_allocator");
    return 1;
  }
  /*
   * wlr_cursor *only* displays an image on screen. It does not move around
   * when the pointer moves. However, we can attach input devices to it, and
   * it will generate aggregate events for all of them. In these events, we
   * can choose how we want to process them, forwarding them to clients and
   * moving the cursor around. More detail on this process is described in
   * https://drewdevault.com/2018/07/17/Input-handling-in-wlroots.html.
   *
   * And more comments are sprinkled throughout the notify functions above.
   */
  /* Add a Unix socket to the Wayland display. */
  const char* socket = wl_display_add_socket_auto(g_Server.wl_display);
  if (!socket) {
    wlr_backend_destroy(g_Server.backend);
    return 1;
  }

  /* Start the backend. This will enumerate outputs and inputs, become the DRM
   * master, etc */
  if (!wlr_backend_start(g_Server.backend)) {
    wlr_backend_destroy(g_Server.backend);
    wl_display_destroy(g_Server.wl_display);
    return 1;
  }

  /* Set the WAYLAND_DISPLAY environment variable to our socket and run the
   * startup command if requested. */
  setenv("WAYLAND_DISPLAY", socket, 1);
  if (startup_cmd) {
    if (fork() == 0) {
      execl("/bin/sh", "/bin/sh", "-c", startup_cmd, nullptr);
    }
  }
  /* Run the Wayland event loop. This does not return until you exit the
   * compositor. Starting the backend rigged up all of the necessary event
   * loop configuration to listen to libinput events, DRM events, generate
   * frame events at the refresh rate, and so on. */
  wlr_log(WLR_INFO, "Running Wayland compositor on WAYLAND_DISPLAY=%s", socket);
  wl_display_run(g_Server.wl_display);

  /* Once wl_display_run returns, we destroy all clients then shut down the
   * server. */
  wl_display_destroy_clients(g_Server.wl_display);
  wlr_scene_node_destroy(&g_Server.scene.root->tree.node);
  wlr_xcursor_manager_destroy(g_Server.cursor_mgr);
  wlr_cursor_destroy(g_Server.cursor);
  wlr_allocator_destroy(g_Server.allocator);
  wlr_renderer_destroy(g_Server.renderer);
  wlr_backend_destroy(g_Server.backend);
  wl_display_destroy(g_Server.wl_display);
  return 0;
}
