# HotWl
## About
Another wayland compositor based off of wlroots. The aim of this compositor is
to get me where I want to go as fast as possible. I'm heavily drawing
inspiration from some of my favorite tools such as Tridactyl, Hyprland, Sway, Nvim, and
Harpoon.

One of the key features at this point distinguishing this compositor from
others is 'Marks' (a feature from nvim where you can "mark" a particular
location to a specific key and then press that same key with a modifier to go
back). I also aim to extend this in the future with automatically assigning
marks on window creation. And 'branding' a window with a permanent mark that will
open that application if its closed and bring you to it if its open.

DISCLAIMER: This software has likely has major jank you probably don't want to 
use this rn lol. I am mostly making this for fun and learning.

## Building
(Make sure you clone the submodules, I include a forked version of wlroots as a submodule so I can
make modifications when there are problems with c/cpp interop)

```sh
meson setup build
meson compile -C build
```
