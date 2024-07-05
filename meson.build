project(
	'gfwl',
	['c', 'cpp'],
	version: '0.1',
	license: 'MIT',
	meson_version: '>=0.60.0',
	default_options: [
		'warning_level=0',
		'werror=false',
	],
)

add_project_arguments(
	[
		'-DWLR_USE_UNSTABLE',
	],
	language: 'c',
)

cc = meson.get_compiler('cpp')

is_freebsd = host_machine.system().startswith('freebsd')

if is_freebsd
	add_project_arguments('-D_C11_SOURCE', language: 'c')
endif

# Execute the wlroots subproject, if any
wlroots_version = ['>=0.17.0', '<0.19.0']
wlroots = dependency('wlroots-0.18', version: wlroots_version, fallback: 'wlroots')
wlroots_features = {
	'xwayland': false,
	'libinput_backend': false,
	'session': false,
}
foreach name, _ : wlroots_features
	var_name = 'have_' + name.underscorify()
	have = wlroots.get_variable(pkgconfig: var_name, internal: var_name) == 'true'
	wlroots_features += {name: have}
endforeach

null_dep = dependency('', required: false)

wayland_server = dependency('wayland-server', version: '>=1.21.0')
wayland_client = dependency('wayland-client')
wayland_protos = dependency('wayland-protocols', version: '>=1.24', default_options: ['tests=false'])
wlr_protos = dependency('wlr-protocols')
xkbcommon = dependency('xkbcommon', version: '>=1.5.0')
libevdev = dependency('libevdev')
libinput = wlroots_features['libinput_backend'] ? dependency('libinput', version: '>=1.21.0') : null_dep
xcb = wlroots_features['xwayland'] ? dependency('xcb') : null_dep
libudev = wlroots_features['libinput_backend'] ? dependency('libudev') : null_dep
math = cc.find_library('m')
rt = cc.find_library('rt')
xcb_icccm = wlroots_features['xwayland'] ? dependency('xcb-icccm') : null_dep

subdir('protocols')
subdir('src')
