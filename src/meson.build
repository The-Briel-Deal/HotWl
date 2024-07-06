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
		'scene.cpp',

		'tiling/focus.cpp',
		'tiling/state.cpp',
		'tiling/container.cpp',
	]
	+ wl_protos_src,
	dependencies: [wlroots, wayland_server, wayland_client, libinput, libevdev, xkbcommon],
)
