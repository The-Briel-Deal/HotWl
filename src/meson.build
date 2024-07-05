executable(
  'gfwl',
  [
    'main.cpp',
    'pointer.cpp',
    'xdg_shell.cpp',
    'layer_shell.cpp',
    'scene.cpp',
    'server.cpp',
    'keyboard.cpp',
    'output.cpp',
    'input.cpp',
  ]
  + wl_protos_src,
  dependencies: [wlroots, wayland_server, wayland_client, libinput, libevdev, xkbcommon],
)
