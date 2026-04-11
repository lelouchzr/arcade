# Arcade graphics backends

Required: **ncurses, SDL2, SFML**. Ten bonus modules use the additional libraries listed in the subject:

| Module in `lib/` | Library | Presentation |
|---|---|---|
| `arcade_ndk++.so` | Official NDK++ / ncursesxx, adapted below | Panel, pen, semantic game cells |
| `arcade_aalib.so` | AA-lib | Monochrome ASCII, menus, semantic board |
| `arcade_libcaca.so` | libcaca | Color dithering and readable cells/text |
| `arcade_allegro5.so` | Allegro 5 | Native primitives and events |
| `arcade_xlib.so` | Xlib | TrueColor XImage, X11 events |
| `arcade_gtk+.so` | GTK 3 / Cairo | Drawing area, GDK keyboard |
| `arcade_irrlicht.so` | Irrlicht | Burning's Video software driver, 2D primitives |
| `arcade_opengl.so` | OpenGL 2.1 | OpenGL texture/quad; SDL window/events |
| `arcade_vulkan.so` | Vulkan | Surface, swapchain, staging buffer, transfer commands; SDL window/events |
| `arcade_qt5.so` | Qt 5 Widgets | QWidget, QPainter, Qt events |

OpenGL/Vulkan present a CPU raster through their respective APIs. No GPU acceleration of game rules is claimed.

## Linux installation

```sh
sudo apt-get install libaa1-dev libcaca-dev libslang2-dev liballegro5-dev libx11-dev \
  libgtk-3-dev libirrlicht-dev libgl1-mesa-dev libvulkan-dev mesa-vulkan-drivers \
  qtbase5-dev libxtst-dev xvfb python3
python3 bonus/graphics/ndk/build_dependencies.py --prefix "$PWD/build-deps/ndk"
cmake -U 'NDK_*' -U NCURSESXX_LIBRARY -S . -B build -DARCADE_NDK_ROOT="$PWD/build-deps/ndk" -DARCADE_REQUIRE_ALL_GRAPHICS=ON
cmake --build build -j
ctest --test-dir build --output-on-failure
./arcade './lib/arcade_gtk+.so'
./arcade './lib/arcade_ndk++.so'
```

The NDK script verifies SHA256 for three official archives; `--cache` supports offline reuse. CMake itself downloads nothing. Licenses are copied into the prefix.

| Option | Effect |
|---|---|
| `ARCADE_REQUIRE_ALL_GRAPHICS=ON` | Missing dependency is an error |
| `ARCADE_EXTRA_GRAPHICS=OFF` | Disable the ten extras |
| `ARCADE_BONUS=OFF` | Required core only |
| `ARCADE_AA_DRIVER=auto\|slang\|curses` | AA-lib driver |
| `CACA_DRIVER` / `AAOPTS` | libcaca driver / AA-lib options |

All thirteen modules are qualified together on Linux. Some macOS extras need X11; not every dependency has been qualified there.

## Lifecycle

| Area | Decision |
|---|---|
| Terminals | Slang for AA-lib/libcaca hot switching, avoiding incompatible ncurses screen state |
| Signals | Restore handlers before unloading |
| System ncurses | Release each screen; on Linux, retain the runtime that owns process-wide caches across plugin unloading |
| GTK | Release widgets, retain GType runtime for Qt compatibility |
| NDK++ | Scripted C++98 PIC adaptations: types, palette/pool order, `resume/suspend` |
| libpanel | Private ncurses/libpanel 6.4, root and caches released; do not substitute system libpanel |

Rebuild older NDK prefixes after script updates. Arcade code remains strict C++20.

364 native renders, pixels, keyboard, resizing · libpanel fix and Valgrind.

Sources: [NDK++/ncursesxx](https://sourceforge.net/projects/ndk-xx/files/), [Allegro](https://allegro5.org/docs/html/refman/index.html), [libcaca drivers](https://github.com/cacalabs/libcaca/tree/main/caca/driver).
