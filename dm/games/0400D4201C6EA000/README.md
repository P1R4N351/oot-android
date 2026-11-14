# Mario Tennis Fever (`0400D4201C6EA000`)

A Switch 2 game: update v327680 (1.1.1) over the base game, SDK 20.5.17 built for the Switch 2 (`Ounce`), NVN2. Camelot's engine (`nxe`) on NintendoWare. Modules `main` and `sdk`, plus one NRO that `main` decodes at run time. **Status on Android: playable, in bring-up (2026-11-12).** On the Odin 3 (Adreno 830) a whole tournament ran at about 60 fps on Mainline Turnip v32; Qualcomm's driver hangs (see Open). Packs come from the desktop packer only. The profile and its shader tables come from nxnative (`b6dbb851`), where the game plays on the Mac.

## What it needs

| Setting | Where | Why |
|---|---|---|
| `mem_alias 0x500000000`, `mem_reserve 0x650000000` | [`game.tsv`](game.tsv) | nxnative's: `nninitStartup` asks `StandardAllocator` for 16 GB of address space, and this SDK keeps its 1 GB node area; with a 16 GB alias region `CentralHeap::Init` aborted on the Mac |
| `mem_stack 0x10000000`, `app_pool 0x240000000`, `sys_resource 0x2000000` | `game.tsv` | nxnative's values, not measured on a phone |
| `render_cond on` | `game.tsv` | as the console does |
| `pad_layout full` | `game.tsv` | the game asks for most of the controller |

## Files

- `main` is a boot stub: it unscrambles and unpacks the game's code, a 24.7 MB NRO, out of its data segment and loads it with `nn::ro`. `dm/tools/nro_collect.py` decodes it offline from 1.1.1's `main` (`boot_nro.py`; current, 2026-11-12): the SHA-256 the NRR registers, in 3.5 s. The shader split takes its output folder as `DM_NRO_DIR`.
- The RomFS is `42` (7.6 GB, magic `CRA.`) and the NRR. `42` holds 8,671 files, listed only in the NRO ([One scrambled archive indexed by the NRO](../../../docs/reference/shaders.md#one-scrambled-archive-indexed-by-the-nro)).

## Shaders

35,045 in [`shader_map.tsv`](shader_map.tsv): 26,096 from models, 5,331 from `ssg_system`, 3,316 from effects, 138 from `shader/`, 124 from layouts, 40 in the NRO's data. No task or mesh shaders, so the mesh lowering does not concern this game. This tree's split of 1.1.1 (macOS, `DM_NRO_DIR` set) makes the four tables byte for byte (current, 2026-11-11). The Vulkan library (macOS, 2026-11-12): all 35,045 shaders translated, every module valid for `spirv-val` (174 MB).

## Open

- Android emulator (test APK at `8be9db6b`, SwiftShader, desktop pack of 1.1.1, 2026-11-12): all eight stages, then `ro::RegisterModuleInfo` with the NRR's one hash and `NRO nro0 loaded` from the pack's `nro.tsv` (77,303 relative relocations; the NVN shader compiler's weak `glslc*` imports bound to 0), and the frame loop at 35 draws a frame, 51 fps median, still running at 96 s. Draws are off there (no placed memory), so nothing is drawn; the `[RED]` lines are the emulator's placed-memory ones, plus `sampler ID 4352 beyond the table (4096)` and one linear texture that cannot be copied back (open, unchecked on a phone).
- The Android host: the host changes of nxnative `b6dbb851` are ported in pull request #275, which the game needs to boot (NRR version 1, the NROs' imports in `dm_game_imports`, the service-connection time for started threads, the font engine's memory, null vertex buffer binds past 16 streams); the video decoder's ready event per packet was already here. Run on the Odin 3 (below). nxnative's run-time capture of the NRO is not ported (the packer decodes it instead).
- The Android pack: `pack_android.py` collects the NRO (`nro_collect.py`), rewrites it with the modules (`nro0`: 12,842 plain sites, no cuts) and lists it in the pack's `nro.tsv`; its shader split gets it as `DM_NRO_DIR`, and the targeted split of 1.1.1 makes the profile's tables byte for byte (current, 2026-11-12, macOS). The phone's Prepare reads it from `main` itself (resolved, 2026-11-12: [Prepare stops at the split …](../../../docs/reference/packing.md#prepare-stops-at-the-split-of-a-game-whose-code-a-boot-stub-packs-into-main)), and its split unscrambles in C ([about an hour on one core](../../../docs/reference/packing.md#the-phones-split-of-camelots-archive-takes-about-an-hour-on-one-core): 6 min 23 s on an Odin 3). Its shaders step took 2 h 13 min or more on an Odin 3 until its results moved to the app's own storage (pull request #280; about 14 minutes on a Galaxy Z Fold 7).
- Odin 3 (Adreno 830, Android app at `ac9249e8`, desktop pack of 1.1.1, 2026-11-12):
  - Mainline Turnip v32: a whole tournament, about 16.5 minutes (60,000 frames), at 60 fps median and 56.6 at the lowest. 4 frames of 250-420 ms, all on the game's main thread with the GPU idle (loads). No GPU fault in the kernel log.
  - Qualcomm's driver V@0800.72 (Vulkan `0x80320048`, dated 2026-02-12): kernel GPU hangs (`Fault id:2`, no page fault), each a stall of about 250 ms, the first at the first match's intro. Dark square patches on the player intros at match start. One run lost the device after 4 minutes (3 hangs in 2 s). Neither showed on Turnip; the cause is open (no GPU snapshot yet).
  - Text drew blank until a GPU-written linear texture followed the CPU's writes (pull request #276). In-match banners then lost letters ("GAME POINT" drawn as "G M PO"): the text compute only reads the glyph cache, yet its binding marked it written, so it was copied back over new glyphs (pull request #276).
  - Menu text went missing after about 15 sub-menus: each sub-menu makes and finalizes a texture and a sampler pool, and the 17th pool shared the first one's IDs (pull request #277).
  - The shadow map: a 3584x3584 depth texture over level 1 of the 7168x7168 shadow map is now that level (pull request #278); on Qualcomm's driver its scaled copy from level 0 is drawn, as D32 has no `BLIT_DST` there (pull request #279). Open: the shadow of the court's umbrella is still cut on both drivers (the console draws it whole): the shadow casters reach only the first cascade, because the passthrough geometry shader's viewport-mask broadcast is not translated (cosmetic, deferred).
  - Button icons drew as "?" (`[YELLOW] Nintendo extended shared font missing`). An icon font made by `dm/tools/make_ext_font.py`, given as `DM_SHARED_FONT_EXT`, draws them. The app now ships that font (made at the APK build, which needs fontTools), and it draws them with no switch set (resolved, 2026-11-14, Turnip: [Button icons drawn as "?" on Android](../../../docs/reference/services.md#button-icons-drawn-as--on-android)).
  - The match start: one frame of 240-275 ms as a match loads, on both drivers (2026-11-13): the replay makes about 550 images (about 450 MB) there, and making the images and their memory takes 117-177 ms of it, the uploads about 70 ms (open).
  - Audio: 0 underruns on both drivers, but about one `audren` catch-up a minute, heard as short skips (open).
  - `[RED]` lines: `tessellation patches are not on Vulkan yet` (4 draws skipped at the match intro); `linear texture serial … format 0x26 … cannot be copied back`; `virtual pool … aliasing physical pool pages is not yet on Android`; `sampler ID 4352 beyond the table (4096)` (no shader of this game reads the pools).

## Found on this game

- [One scrambled archive indexed by the NRO](../../../docs/reference/shaders.md#one-scrambled-archive-indexed-by-the-nro)
- [The game's code packed into `main` by a boot stub](../../../docs/reference/services.md#the-games-code-packed-into-main-by-a-boot-stub)
- [Prepare stops at the split of a game whose code a boot stub packs into `main`](../../../docs/reference/packing.md#prepare-stops-at-the-split-of-a-game-whose-code-a-boot-stub-packs-into-main)
- [The phone's split of Camelot's archive takes about an hour on one core](../../../docs/reference/packing.md#the-phones-split-of-camelots-archive-takes-about-an-hour-on-one-core)
- A GPU-written linear texture the CPU writes afterwards (pull request #276)
- A shader image the shader only reads marked GPU-written (pull request #276)
- A texture over mip levels of another (pull request #278)
- A scaled depth copy into a format with no BLIT_DST (pull request #279)
- Descriptor pools made and finalized as the game goes (pull request #277)
