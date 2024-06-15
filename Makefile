WAYLAND_PROTOCOLS=$(shell pkg-config --variable=pkgdatadir wayland-protocols)
WLR_PROTOCOLS=$(shell pkg-config --variable=pkgdatadir wlr-protocols)
WAYLAND_SCANNER=$(shell pkg-config --variable=wayland_scanner wayland-scanner)
LIBS=\
	 $(shell pkg-config --cflags --libs "wlroots-0.18") \
	 $(shell pkg-config --cflags --libs wayland-server) \
	 $(shell pkg-config --cflags --libs xkbcommon)

build/lib/xdg-shell-protocol.h: | build/lib
	$(WAYLAND_SCANNER) server-header \
		$(WAYLAND_PROTOCOLS)/stable/xdg-shell/xdg-shell.xml $@

build/lib/wlr-layer-shell-unstable-v1-protocol.h: | build/lib
	$(WAYLAND_SCANNER) server-header \
		$(WLR_PROTOCOLS)/unstable/wlr-layer-shell-unstable-v1.xml $@

build:
	mkdir $@

build/lib: | build
	mkdir $@

build/main: src/main.c build/lib/xdg-shell-protocol.h build/lib/wlr-layer-shell-unstable-v1-protocol.h | build 
	$(CC) $(CFLAGS) \
		-g -Werror -I. \
		-DWLR_USE_UNSTABLE \
		-o $@ $< \
		$(LIBS)

clean:
	rm -rf build 

.DEFAULT_GOAL=build/main
.PHONY: clean
