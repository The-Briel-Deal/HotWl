executable(
  'gfwl',
  [
    'main.c',
    'pointer.c',
    'xdg_shell.c',
    'layer_shell.c',
    'scene.c',
    'server.c',
    'keyboard.c',
    'output.c',
    'input.c',
    'tiling_focus.c',
  ]
  + wl_protos_src,
  dependencies: [wlroots, wayland_server, wayland_client, libinput, libevdev, xkbcommon],
)
