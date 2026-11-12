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

- `main` is a boot stub: it unscrambles and unpacks the game's code, a 24.7 MB NRO, out of its data segment and loads it with `nn::ro`. `dm/tools/nro_collect.py` decodes it offline from 1.1.1's `main` (`boot_nro.py`; current, 2026-11-12): the SHA-256 the NRR registers, in 3.5 s. The shader split takes its output folder as `DM_NRO_DIR`.
- The RomFS is `42` (7.6 GB, magic `CRA.`) and the NRR. `42` holds 8,671 files, listed only in the NRO ([One scrambled archive indexed by the NRO](../../../docs/reference/shaders.md#one-scrambled-archive-indexed-by-the-nro)).

## Shaders

35,045 in [`shader_map.tsv`](shader_map.tsv): 26,096 from models, 5,331 from `ssg_system`, 3,316 from effects, 138 from `shader/`, 124 from layouts, 40 in the NRO's data. No task or mesh shaders, so the mesh lowering does not concern this game. This tree's split of 1.1.1 (macOS, `DM_NRO_DIR` set) makes the four tables byte for byte (current, 2026-11-11). Not yet built: the Vulkan library.

## Open

- The Android host: the host changes of nxnative `b6dbb851` are ported (NRR version 1, the NROs' imports in `dm_game_imports`, the service-connection time for started threads, the font engine's memory, null vertex buffer binds past 16 streams); the video decoder's ready event per packet was already here. Not run on a phone. nxnative's run-time capture of the NRO is not ported (the packer decodes it instead).
- The Android pack: `pack_android.py` collects the NRO (`nro_collect.py`), rewrites it with the modules (`nro0`: 12,842 plain sites, no cuts) and lists it in the pack's `nro.tsv`; its shader split gets it as `DM_NRO_DIR`, and the targeted split of 1.1.1 makes the profile's tables byte for byte (current, 2026-11-12, macOS). The phone's Prepare does not collect it yet (open), so packs come from the desktop packer.
- The Vulkan library (`vk_android_build.py`) of the 35,045 shaders.

## Found on this game

- [One scrambled archive indexed by the NRO](../../../docs/reference/shaders.md#one-scrambled-archive-indexed-by-the-nro)
- [The game's code packed into `main` by a boot stub](../../../docs/reference/services.md#the-games-code-packed-into-main-by-a-boot-stub)
