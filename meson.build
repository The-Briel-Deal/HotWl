project(
	'gfwl',
	['c', 'cpp'],
	version: '0.1',
	license: 'MIT',
	meson_version: '>=0.60.0',
	default_options: [
		'cpp_std=c++20',
		'warning_level=0',
		'werror=true',
	],
)

add_project_arguments(
	[
		'-DWLR_USE_UNSTABLE',
	],
	language: ['c', 'cpp'],
)

cc = meson.get_compiler('cpp')

subproject('json')
wlroots = subproject('wlroots', default_options: ['examples=false', 'renderers=gles2']).get_variable('wlroots')


wayland_server = dependency('wayland-server', version: '>=1.21.0')
wayland_client = dependency('wayland-client')
wayland_protos = dependency('wayland-protocols', version: '>=1.24', default_options: ['tests=false'])
wlr_protos = dependency('wlr-protocols')
xkbcommon = dependency('xkbcommon', version: '>=1.5.0')
libevdev = dependency('libevdev')
libinput = dependency('libinput', version: '>=1.21.0')
math = cc.find_library('m')
rt = cc.find_library('rt')

subdir('protocols')
subdir('src')
