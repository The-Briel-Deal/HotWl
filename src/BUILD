load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_import(
    name = "wlroots",
    system_provided = 1,
)

# cc_import(
#     name = "wayland-server-core",
#     hdrs = ["wayland-server-core.h"],
#     system_provided = 1,
# )

# cc_library(
#     name = "server",
#     srcs = ["server.c"],
#     hdrs = ["server.h"],
#     deps = [":wlroots", ":xdg_shell"],
# )

# cc_library(
#     name = "xdg_shell",
#     srcs = ["xdg_shell.c"],
#     hdrs = ["xdg_shell.h"],
#     deps = [":wlroots", ":server"],
# )

# cc_library(
#     name = "input",
#     srcs = ["input.c"],
#     hdrs = ["input.h"],
#     deps = [":wlroots"],
# )


cc_binary(
    name = "gfwl",
    copts = ["-Isrc","-lwlroots-0.18", "-lwayland-server", "-lxkbcommon", "-lwlr-protocols"],
    includes = ["/usr/include"],
    srcs = [
        "main.c",
	"xdg_shell.c",
	"xdg_shell.h",
	"server.c",
	"server.h",
	"layer_shell.c",
	"layer_shell.h",
	"keyboard.c",
	"keyboard.h",
	"pointer.c",
	"pointer.h",
	"output.c",
	"output.h",
	"scene.c",
	"scene.h",
	"input.c",
	"input.h",
    ],
    defines = ["WLR_USE_UNSTABLE"],
    deps = [
        ":wlroots",
    ]
)
