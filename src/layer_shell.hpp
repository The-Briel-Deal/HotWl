#pragma once
// #include <wlr/types/wlr_layer_shell_v1.h>
// #include <wlr/types/wlr_scene.h>
extern "C" {
#include <wayland-server-core.h>
#include <wayland-util.h>
}

enum zwlr_layer_surface_v1_keyboard_interactivity {
	/**
	 * no keyboard focus is possible
	 *
	 * This value indicates that this surface is not interested in
	 * keyboard events and the compositor should never assign it the
	 * keyboard focus.
	 *
	 * This is the default value, set for newly created layer shell
	 * surfaces.
	 *
	 * This is useful for e.g. desktop widgets that display information
	 * or only have interaction with non-keyboard input devices.
	 */
	ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_NONE = 0,
	/**
	 * request exclusive keyboard focus
	 *
	 * Request exclusive keyboard focus if this surface is above the
	 * shell surface layer.
	 *
	 * For the top and overlay layers, the seat will always give
	 * exclusive keyboard focus to the top-most layer which has
	 * keyboard interactivity set to exclusive. If this layer contains
	 * multiple surfaces with keyboard interactivity set to exclusive,
	 * the compositor determines the one receiving keyboard events in
	 * an implementation- defined manner. In this case, no guarantee is
	 * made when this surface will receive keyboard focus (if ever).
	 *
	 * For the bottom and background layers, the compositor is allowed
	 * to use normal focus semantics.
	 *
	 * This setting is mainly intended for applications that need to
	 * ensure they receive all keyboard events, such as a lock screen
	 * or a password prompt.
	 */
	ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_EXCLUSIVE = 1,
	/**
	 * request regular keyboard focus semantics
	 *
	 * This requests the compositor to allow this surface to be
	 * focused and unfocused by the user in an implementation-defined
	 * manner. The user should be able to unfocus this surface even
	 * regardless of the layer it is on.
	 *
	 * Typically, the compositor will want to use its normal mechanism
	 * to manage keyboard focus between layer shell surfaces with this
	 * setting and regular toplevels on the desktop layer (e.g. click
	 * to focus). Nevertheless, it is possible for a compositor to
	 * require a special interaction to focus or unfocus layer shell
	 * surfaces (e.g. requiring a click even if focus follows the mouse
	 * normally, or providing a keybinding to switch focus between
	 * layers).
	 *
	 * This setting is mainly intended for desktop shell components
	 * (e.g. panels) that allow keyboard interaction. Using this option
	 * can allow implementing a desktop shell that can be fully usable
	 * without the mouse.
	 * @since 4
	 */
	ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_ON_DEMAND = 2,
};
enum zwlr_layer_shell_v1_layer {
	ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND = 0,
	ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM = 1,
	ZWLR_LAYER_SHELL_V1_LAYER_TOP = 2,
	ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY = 3,
};
uint32_t wlr_layer_surface_v1_configure(struct wlr_layer_surface_v1 *surface,
		uint32_t width, uint32_t height);
struct wlr_layer_shell_v1 {
	struct wl_global *global;

	struct wl_listener display_destroy;

	struct {
		// Note: the output may be NULL. In this case, it is your
		// responsibility to assign an output before returning.
		struct wl_signal new_surface; // struct wlr_layer_surface_v1
		struct wl_signal destroy;
	} events;

	void *data;
};

enum wlr_layer_surface_v1_state_field {
	WLR_LAYER_SURFACE_V1_STATE_DESIRED_SIZE = 1 << 0,
	WLR_LAYER_SURFACE_V1_STATE_ANCHOR = 1 << 1,
	WLR_LAYER_SURFACE_V1_STATE_EXCLUSIVE_ZONE = 1 << 2,
	WLR_LAYER_SURFACE_V1_STATE_MARGIN = 1 << 3,
	WLR_LAYER_SURFACE_V1_STATE_KEYBOARD_INTERACTIVITY = 1 << 4,
	WLR_LAYER_SURFACE_V1_STATE_LAYER = 1 << 5,
};

struct wlr_layer_surface_v1_state {
	uint32_t committed; // enum wlr_layer_surface_v1_state_field

	uint32_t anchor;
	int32_t exclusive_zone;
	struct {
		int32_t top, right, bottom, left;
	} margin;
	enum zwlr_layer_surface_v1_keyboard_interactivity keyboard_interactive;
	uint32_t desired_width, desired_height;
	enum zwlr_layer_shell_v1_layer layer;

	uint32_t configure_serial;
	uint32_t actual_width, actual_height;
};

struct wlr_layer_surface_v1_configure {
	struct wl_list link; // wlr_layer_surface_v1.configure_list
	uint32_t serial;

	uint32_t width, height;
};

struct wlr_layer_surface_v1 {
	struct wlr_surface *surface;
	struct wlr_output *output;
	struct wl_resource *resource;
	struct wlr_layer_shell_v1 *shell;
	struct wl_list popups; // wlr_xdg_popup.link

	char *namespace;

	bool configured;
	struct wl_list configure_list;

	struct wlr_layer_surface_v1_state current, pending;

	// Whether the surface is ready to receive configure events
	bool initialized;
	// Whether the latest commit is an initial commit
	bool initial_commit;

	struct {
		/**
		 * The destroy signal indicates that the struct wlr_layer_surface is
		 * about to be freed. It is guaranteed that the unmap signal is raised
		 * before the destroy signal if the layer surface is destroyed while
		 * mapped.
		 */
		struct wl_signal destroy;
		/**
		 * The new_popup signal is raised when a new popup is created. The data
		 * parameter passed to the listener is a pointer to the new
		 * struct wlr_xdg_popup.
		 */
		struct wl_signal new_popup;
	} events;

	void *data;

	// private state

	struct wlr_surface_synced synced;
};
struct gfwl_layer_surface {
  struct wl_list link;
  struct gfwl_output *output;
  struct wlr_scene_layer_surface_v1 *scene;
  struct wlr_layer_surface_v1 *wlr_layer_surface;
  struct wlr_surface *prev_focused;
  struct gfwl_server *server;
  struct wl_listener map;
  struct wl_listener unmap;
  struct wl_listener commit;
};

void handle_new_layer_shell_surface(struct wl_listener *listener, void *data);
void handle_layer_surface_commit(struct wl_listener *listener, void *data);
void handle_layer_surface_map(struct wl_listener *listener, void *data);
