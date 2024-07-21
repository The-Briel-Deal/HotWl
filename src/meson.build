executable(
	'gfwl',
	[
		# Core
		'main.cpp',
		'server.cpp',
		'pointer.cpp',
		'xdg_shell.cpp',
		'layer_shell.cpp',
		'keyboard.cpp',
		'output.cpp',
		'input.cpp',

		# Tiling
		'tiling/focus.cpp',
		'tiling/state.cpp',

		# Containers
		'tiling/container/base.cpp',
		'tiling/container/root.cpp',
		'tiling/container/toplevel.cpp',

		# Config
		'conf/config.cpp',
	]
	+ wl_protos_src,
	dependencies: [wlroots, wayland_server, wayland_client, libinput, libevdev, xkbcommon],
)
