# Super Mario Bros. Wonder - Nintendo Switch 2 Edition (`040015100B514001`)

Program 1 of application `010015100B514000`, display version 1.2.1 (update v327680), SDK 21.4.1 (Ounce). The Switch 2 Edition is another program beside the original game, [Super Mario Bros. Wonder](../010015100B514000/README.md) (program 0), which keeps its own profile: its own main (`ec47e6f8846d2a7b6892e2682845c725`), sdk (`31945f745022d0232d2562252641b3d3`) and `subsdk0` (NintendoSDK_movie), and NVN2. Status: **bring-up, playable**: on an AYN Thor (Snapdragon 8 Gen 2, Adreno 740, Turnip) it runs from a dump folder with the edition switch through the intro and a first course at 30 fps (2026-11-14, 9.5 minutes of play, the application's save; see Open).

## What it needs

| Setting | Where | Why |
|---|---|---|
| 9.875 GB block: `mem_reserve 0x278000000`, `mem_alias 0x118000000`, `mem_stack 0x10000000`, `app_pool 0x240000000`, `sys_resource 0x1000000` | [`game.tsv`](game.tsv) | main asks for a 4.94 GB heap (`SetHeapSize 0x13be00000`) with the Switch 2's 9 GB pool and writes to it without checking the result: with the survey's 5 GB block the heap region was 128 MB and the first run crashed in `nninitStartup` (resolved, 2026-11-14). The heap check on the Mac set the block (a 5 GB heap region); the address region check found the game's regions at +2.09 GB of the 4.375 GB alias region in its first 15 s (+3.25 GB in a course on the phone). 16 MB of system resource (virtual address memory) |
| `modules main subsdk0 sdk` | `game.tsv` | `subsdk0` is the movie library |
| `render_cond on` | `game.tsv` | without it the half-resolution depth and occlusion targets are wiped between their draws and the overworld turns blue (nxnative PR #1639) |
| resource heap +16 KB, `main+0x130514` | [`patches.py`](patches.py) | New Game crashed in `nvnMemoryPoolGetFlags`: a resource heap runs short by the host's NVN2 command-memory sizes (nxnative #1580; open there) |

## Files and shaders

- The game reads three trees: its own RomFS, mounted as `content` (one file, `System/RegionLangMask.txt`); program 0's RomFS (update v327680, 19,034 files, 3.47 GB), mounted as `conorig` through the program-indexed `MountRom` (`DM_ROMFS_PROGRAM_0`, `dm/host/hle_fs.c`); and add-on `010015100B515001` (v131072, 26,452 files, 5.0 GB), the Switch 2 Edition and Meetup in Bellabel Park content, mounted as `aocup` and add-on content.
- 30,630 shaders in the map: 17,807 from the add-on's `Nin_Ounce_NVN2` archives (`Shader/SecredUber.Product.110.product.Nin_Ounce_NVN2.bfsha.zs`, `Shader/Secred.Nin_Ounce_NVN2.release.sarc.zs`, the effects), the rest from program 0's `Nin_NX_NVN` ones, which the game can reach through `conorig`. The `shader_groups` are program 0's, by where Wonder keeps its shaders (`Shader/SecredUber` 11,060, `Effect/` 10,184, agl's `.sharcb` 9,039, `Layout/` 271): the provisional profile's guess from the RomFS's top folders left 20,135 in `other`.
- The dump folder in the console's layout holds program 1 at `updates/327680/program/1/` beside program 0. The survey and the split ran on a folder of program 1's ExeFS, program 0's newest RomFS and the add-on (`scripts/pack_dump.py` takes a program ID with its index in the low byte, and relates add-ons to the application).

## Found on this game

- [Built-in fonts of SDK 21](../../../docs/reference/services.md#built-in-fonts-and-the-audio-device-notification-of-sdk-21): main calls `GetBuiltInFontData` and imports `ScalableFontEngineHelper::Decode`, so the host gave it the font with its BFTTF header; it now gives the TTF past the header, as the service's offsets do (taken from nxnative PR #1639; what the header did to this game's text was not recorded).
- [A gate opened by its time while the host held the game's thread](../../../docs/reference/nvn-objects.md#a-gate-opened-by-its-time-while-the-host-held-the-games-thread-android): turning frame generation on mid-run banned an event for the rest of the run. The first fix hung the GPU when frame generation was switched on and off (a gate nothing could open while frame generation waited for the queue); fixed again, not yet checked on the phone.

## Open

- Android: `pack_android.py --program 1` packs program 1 from the dump folder, and the launcher's game card switches between the editions and keeps one save for both (the application's, as on the console). One pipeline is made on the frame path at the start (85.5 ms: the prewarm threads had not reached it yet). The system driver is not yet tested.
- The checks on the Mac booted program 1 from a folder of its ExeFS and its own RomFS, with program 0's RomFS passed as `DM_ROMFS_PROGRAM_0`: `build_ipa_from_nsp.py` has no option for another program yet.
