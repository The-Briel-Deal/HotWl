executable(
  'gfwl',
  [
    'main.cpp',
    'pointer.cpp',
    'xdg_shell.cpp',
    'layer_shell.cpp',
    'server.cpp',
    'keyboard.cpp',
    'output.cpp',
    'input.cpp',
    'tiling_focus.cpp',
    'scene.cpp',
  ]
  + wl_protos_src,
  dependencies: [wlroots, wayland_server, wayland_client, libinput, libevdev, xkbcommon],
)
