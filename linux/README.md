# Banana Native on aarch64 Linux (experimental, community / best effort)

The Android host (`dm/host`, `dm/gfx/vk_android`) built as a native aarch64 glibc executable, for Snapdragon handhelds
running a desktop Linux (SteamOS-like images). It plays the same packs as Android (`target android`): a pack made by the
Android app, or by `scripts/pack_android.py`, runs unchanged. Linux has no maintainer of its own: Linux-only problems may
stay open for a while.

## Tested

| | |
|---|---|
| Device | AYN Odin 2 Pro (SM8550, Adreno 740, 12 GB RAM, no swap) |
| System | SteamOS-like Ubuntu 26.04 image, kernel 7.2.10, glibc 2.43, system SDL3, Plasma (Wayland) and Steam Game Mode (Gamescope, X11) |
| Driver | Mesa 26.2.7 Turnip (`VK_EXT_map_memory_placed` yes; no mesh shaders, no sparse residency) |
| Games | Mario Kart World 1.8.0 (picture, sound, controls, saves, full races; 30-35 fps average at 720p handheld, start grid 24-29, mid-race 38-53); Donkey Kong Bananza, update 458752, with its add-on (boots, intro and menus) |

## What works and what is missing

Works: the Vulkan replay on Turnip, sound (SDL3 audio, PipeWire), gamepads, touch and rumble (SDL3), saves, `.nup` game
files and their add-ons, the replay's selftest (`--vk-selftest`).

Missing: H.264 / AAC (no AMediaCodec: `linux/shim/shim_media.c` is a stub; Mario Kart World's intros and cutscenes play
without it), motion sensors (SDL3 can read a pad's own gyro, but the Odin 2 Pro's built-in pad exposes none on this image), a
settings UI (settings are the host's `DM_*` variables), pack preparation on the device (a later change), host unit tests
run on Linux.

## How it works

- The host's sources are compiled as they are, with `__ANDROID__` and `NX_LINUX=1` defined (`linux/host/CMakeLists.txt`),
  against stand-ins for the NDK headers in `linux/shim/include`. Their implementations are in `linux/shim/*.c`:
  - `shim_aaudio.c`: AAudio on SDL3 audio.
  - `shim_window.c`: `ANativeWindow` as a small record of the SDL window.
  - `shim_sys.c`: `__system_property_get`, `pthread_gettid_np`.
  - `shim_media.c`, `shim_sensor.c`: stubs.
- `linux/host/front_sdl.c` stands in for the Android launcher: SDL3 window, gamepads, touch, rumble, forwarded to the
  functions of `dm/host/android/input_android.h`. On X11 the Vulkan surface is an xcb one on its own connection (sharing
  SDL's Xlib connection with the presenting thread froze or crashed under Gamescope).
- `linux/host/tls_slot.c`: rewritten game code reads its thread slots through `[tpidr_el0 + 0x10]` (Bionic's
  `TLS_SLOT_APP`). With glibc that word is the first thread-local variable of the executable, so the host is an
  executable (not a shared library), this file is linked first, and `main_linux.c` checks the address at start. That is
  why Android packs run unchanged.
- `-Dmain=nx_host_main`: `dm/host/main.c`'s `main` is called by `nxhost_main`; the real `main` is in `main_linux.c`.
- Left out: `dm/host/android/jni_android.c`, libadrenotools (custom GPU drivers).
- Changes to shared files, all inside `#ifdef NX_LINUX` (an Android build compiles them out):
  `dm/gfx/vk_android/vka_dev.c` (instance extensions from the front end; display timing off unless asked, below) and
  `dm/gfx/vk_android/vka_present.c` (the surface from the front end).

## Build

    linux/build.sh <base game title ID>        # e.g. 0400C3F00006E000 → linux/build/<id>/nxhost_<id>

Cross build with Docker on an x86_64 or arm64 machine. The image (`linux/Dockerfile`: Ubuntu 24.04, clang, lld, the arm64
libraries) is built on first use. SDL3: `linux/sysroot/{include/SDL3,lib/libSDL3.so}` when there is one (copied from the
target system), else SDL3 built from source by `linux/build-sdl3.sh` (git tag `release-3.4.2`). The first build takes
some minutes. One binary per game: the game profile is compiled in, as on Android. Run time needs glibc ≥ 2.39, libSDL3,
libxcb, libz, and a Vulkan driver with `VK_EXT_map_memory_placed` (Turnip).

## Run

    nxhost_<id> --vk-selftest [frames]         # the replay's own selftest; NX_HEADLESS=1 for no window
    cd <pack folder> && DM_ROMFS=<game.nup or RomFS folder> DM_SAVE=… DM_TEMP=… DM_ROOT=… DM_AOC_DIR=… \
        HOME=… TMPDIR=… nxhost_<id> <pack folder>

Game settings are the host's `DM_*` variables (`docs/env_switches.tsv`); what the Android launcher sets is in
`GameActivity.java` (720p: `DM_GRAPHICS_SCALE=2 DM_OPMODE=handheld`; RAM saver:
`DM_MEM_FILEBACK=1 DM_MEM_FILEBACK_KINDS=shader,texture`). Linux-only switches: `NX_WINDOW=<w>x<h>` (a window instead of
full screen), `NX_HEADLESS=1` (no window), `NX_NO_WINDOW_OK=1` (go on when no window can be made; by default the host
ends with a `[RED]` line naming the display variables), `NX_VK_DISPLAY_TIMING=1` (below). Quit: close the window.

## Device notes (docs/reference)

- Gamescope's Vulkan layer and `VK_GOOGLE_display_timing`: [rendering.md](../docs/reference/rendering.md), "Present times
  through Gamescope's layer fault the GPU (Linux)". The host gives no present times unless `NX_VK_DISPLAY_TIMING=1`.
- No swap: [memory.md](../docs/reference/memory.md), "The measured image cache limit with no swap (Linux)". Run with
  `DM_VK_IMG_LIMIT_MB=2300` on a 12 GB device with no swap. A swap file is not a way out on Turnip: it corrupted the
  picture.
- With `DM_VK_PRUNE_MS=3` a texture cache prune does not hold one frame for a second
  ([memory.md](../docs/reference/memory.md), "A texture cache prune that holds one frame for a second").
