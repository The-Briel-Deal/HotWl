#include <includes.hpp>
#include <pointer.hpp>
#include <wayland-server-core.h>
#include <wlr/types/wlr_xdg_shell.h>

void server_new_input(struct wl_listener *listener, void *data);
void seat_request_cursor(struct wl_listener *listener, void *data);
void seat_request_set_selection(struct wl_listener *listener, void *data);
struct gfwl_toplevel *desktop_toplevel_at(struct GfServer *server, double lx,
                                          double ly,
                                          struct wlr_surface **surface,
                                          double *sx, double *sy);
