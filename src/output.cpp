#include <includes.hpp>
#include <memory>
#include <output.hpp>
#include <scene.hpp>
#include <server.hpp>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/util/box.h>

// Gets the output that a container is in.
std::shared_ptr<gfwl_output>
get_output_from_container(std::shared_ptr<GfContainer> container) {
  auto container_box = container->box;
  auto server = container->server;
  auto outputs = server->outputs;
  for (auto output : outputs) {
    wlr_box output_box = {
        .x = output->scene_output->x,
        .y = output->scene_output->y,
        .width = output->wlr_output->width,
        .height = output->wlr_output->height,
    };
    if (output_box.x <= container_box.x && output_box.y <= container_box.y &&
        output_box.x + output_box.width >= container_box.x &&
        output_box.y + output_box.height >= container_box.y) {
      return output;
    }
  }
  return NULL;
}

// Focuses the output that the container is in.
void focus_output_from_container(std::shared_ptr<GfContainer> container) {
  auto output = get_output_from_container(container);
  auto server = container->server;
  if (output && server) {
    server->focused_output = output;
  } else {
    wlr_log(WLR_ERROR, "Your container doesn't have an output ):<");
  }
}

static void output_frame(struct wl_listener *listener, void *data) {
  /* This function is called every time an output is ready to display a frame,
   * generally at the output's refresh rate (e.g. 60Hz). */
  struct gfwl_output *output = wl_container_of(listener, output, frame);
  struct wlr_scene *scene = output->server->scene->root;

  struct wlr_scene_output *scene_output =
      wlr_scene_get_scene_output(scene, output->wlr_output);

  /* Render the scene if needed and commit the output */
  wlr_scene_output_commit(scene_output, NULL);

  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  wlr_scene_output_send_frame_done(scene_output, &now);
}

static void output_request_state(struct wl_listener *listener, void *data) {
  /* This function is called when the backend requests a new state for
   * the output. For example, Wayland and X11 backends request a new mode
   * when the output window is resized. */
  struct gfwl_output *output = wl_container_of(listener, output, request_state);
  const struct wlr_output_event_request_state *event =
      (wlr_output_event_request_state *)data;
  wlr_output_commit_state(output->wlr_output, event->state);
}

static void output_destroy(struct wl_listener *listener, void *data) {
  struct gfwl_output *output = wl_container_of(listener, output, destroy);

  wl_list_remove(&output->frame.link);
  wl_list_remove(&output->request_state.link);
  wl_list_remove(&output->destroy.link);
  wl_list_remove(&output->link);
  free(output);
}

void server_new_output(struct wl_listener *listener, void *data) {
  /* This event is raised by the backend when a new output (aka a display or
   * monitor) becomes available. */
  struct gfwl_server *server = wl_container_of(listener, server, new_output);
  struct wlr_output *wlr_output = (struct wlr_output *)data;

  /* Configures the output created by the backend to use our allocator
   * and our renderer. Must be done once, before commiting the output */
  wlr_output_init_render(wlr_output, server->allocator, server->renderer);

  /* The output may be disabled, switch it on. */
  struct wlr_output_state state;
  wlr_output_state_init(&state);
  wlr_output_state_set_enabled(&state, true);

  /* Some backends don't have modes. DRM+KMS does, and we need to set a mode
   * before we can use the output. The mode is a tuple of (width, height,
   * refresh rate), and each monitor supports only a specific set of modes. We
   * just pick the monitor's preferred mode, a more sophisticated compositor
   * would let the user configure it. */
  struct wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
  if (mode != NULL) {
    wlr_output_state_set_mode(&state, mode);
  }

  /* Atomically applies the new output state. */
  wlr_output_commit_state(wlr_output, &state);
  wlr_output_state_finish(&state);

  /* Allocates and configures our state for this output */
  struct std::shared_ptr<gfwl_output> output = std::make_shared<gfwl_output>();
  output->wlr_output = wlr_output;
  output->server = server;

  /* Sets up a listener for the frame event. */
  output->frame.notify = output_frame;
  wl_signal_add(&wlr_output->events.frame, &output->frame);

  /* Sets up a listener for the state request event. */
  output->request_state.notify = output_request_state;
  wl_signal_add(&wlr_output->events.request_state, &output->request_state);

  /* Sets up a listener for the destroy event. */
  output->destroy.notify = output_destroy;
  wl_signal_add(&wlr_output->events.destroy, &output->destroy);

  // Setting tiling state_defaults. TODO: Move to its own function.
  server->focused_output = output;
  server->outputs.push_back(output);
  output->tiling_state->root = std::make_shared<GfContainer>(
      true, nullptr, GFWL_CONTAINER_ROOT, nullptr, nullptr, nullptr);
  output->tiling_state->root->tiling_state = output->tiling_state;

  output->tiling_state->root->e_type = GFWL_CONTAINER_ROOT;
  output->tiling_state->split_dir = GFWL_SPLIT_DIR_HORI;
  output->tiling_state->root->server = server;
  output->tiling_state->root->is_root = true;
  output->tiling_state->output = output;

  /* Adds this to the output layout. The add_auto function arranges outputs
   * from left-to-right in the order they appear. A more sophisticated
   * compositor would let the user configure the arrangement of outputs in the
   * layout.
   *
   * The output layout utility automatically adds a wl_output global to the
   * display, which Wayland clients can see to find out information about the
   * output (such as DPI, scale factor, manufacturer, etc).
   */
  output->output_layout_output =
      wlr_output_layout_add_auto(server->output_layout, wlr_output);
  output->scene_output =
      wlr_scene_output_create(server->scene->root, wlr_output);
  wlr_scene_output_layout_add_output(
      server->scene_layout, output->output_layout_output, output->scene_output);
}
