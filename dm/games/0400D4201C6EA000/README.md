# Mario Tennis Fever (`0400D4201C6EA000`)

A Switch 2 game: update v327680 (1.1.1) over the base game, SDK 20.5.17 built for the Switch 2 (`Ounce`), NVN2. Camelot's engine (`nxe`) on NintendoWare. Modules `main` and `sdk`, plus one NRO that `main` decodes at run time. **Status on Android: not run.** The profile and its shader tables come from nxnative (`b6dbb851`), where the game plays on the Mac; this tree has its shader split, not yet its host side.

## What it needs

| Setting | Where | Why |
|---|---|---|
| `mem_alias 0x500000000`, `mem_reserve 0x650000000` | [`game.tsv`](game.tsv) | nxnative's: `nninitStartup` asks `StandardAllocator` for 16 GB of address space, and this SDK keeps its 1 GB node area; with a 16 GB alias region `CentralHeap::Init` aborted on the Mac |
| `mem_stack 0x10000000`, `app_pool 0x240000000`, `sys_resource 0x2000000` | `game.tsv` | nxnative's values, not measured on a phone |
| `render_cond on` | `game.tsv` | as the console does |
| `pad_layout full` | `game.tsv` | the game asks for most of the controller |

## Files

- `main` is a boot stub: it unscrambles and unpacks the game's code, a 24.7 MB NRO, out of its data segment and loads it with `nn::ro`. The shader split needs that NRO in `DM_NRO_DIR`; today it comes from a run of nxnative's Mac host (`DM_NRO_CAPTURE`).
- The RomFS is `42` (7.6 GB, magic `CRA.`) and the NRR. `42` holds 8,671 files, listed only in the NRO ([One scrambled archive indexed by the NRO](../../../docs/reference/shaders.md#one-scrambled-archive-indexed-by-the-nro)).

## Shaders

35,045 in [`shader_map.tsv`](shader_map.tsv): 26,096 from models, 5,331 from `ssg_system`, 3,316 from effects, 138 from `shader/`, 124 from layouts, 40 in the NRO's data. No task or mesh shaders, so the mesh lowering does not concern this game. This tree's split of 1.1.1 (macOS, `DM_NRO_DIR` set) makes the four tables byte for byte (current, 2026-11-11). Not yet built: the Vulkan library.

## Open

- The Android host: running a game whose code is an NRO decoded at run time, and the other host changes of nxnative `b6dbb851` (NRR version 1, the service-connection time for started threads, the font engine's memory, a video decoder's ready event per packet). None is ported.
- The Android pack: `pack_android.py` and the phone's Prepare need the NRO; nothing captures it on a phone.
- The Vulkan library (`vk_android_build.py`) of the 35,045 shaders.

## Found on this game

- [One scrambled archive indexed by the NRO](../../../docs/reference/shaders.md#one-scrambled-archive-indexed-by-the-nro)
