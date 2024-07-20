#include <server.hpp>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_input_device.h>

void server_new_pointer(GfServer* server, struct wlr_input_device* device);

void reset_cursor_mode(GfServer* server);

struct gfwl_toplevel* desktop_toplevel_at(GfServer*            server,
                                          double               lx,
                                          double               ly,
                                          struct wlr_surface** surface,
                                          double*              sx,
                                          double*              sy);

void server_cursor_frame(struct wl_listener* listener, void* data);

void server_cursor_axis(struct wl_listener* listener, void* data);

void server_cursor_button(struct wl_listener* listener, void* data);

void server_cursor_motion_absolute(struct wl_listener* listener, void* data);

void server_cursor_motion(struct wl_listener* listener, void* data);
