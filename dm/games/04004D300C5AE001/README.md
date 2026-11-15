# Kirby and the Forgotten Land - Nintendo Switch 2 Edition (`04004D300C5AE001`)

Program 1 of application `01004D300C5AE000`, display version 1.1.0 (update v65536), SDK 19.3.7 (Ounce). The Switch 2 Edition + Star-Crossed World is another program beside the original game, [Kirby and the Forgotten Land](../01004D300C5AE000/README.md) (program 0), which keeps its own profile: its own main (`395fc326c2e5b1651d937074f8cfd75d`), sdk (`5e81e79381e8be36fe8bcf05ae18d6dd`) and `subsdk0` (libc), and NVN2. Status: **bring-up**, not yet run on Android.

## What it needs

| Setting | Where | Why |
|---|---|---|
| 5 GB block: `mem_reserve 0x140000000`, `mem_alias 0x118000000`, `mem_stack 0x10000000`, `app_pool 0x240000000`, `sys_resource 0x1000000` | [`game.tsv`](game.tsv) | 16 MB of system resource (virtual address memory); the survey finds no heap set by main, so the game maps its memory in the alias region itself; the Switch 2's 9 GB pool. The survey's guess (`dm/games/profile_guess.py`), unchecked: Super Mario Bros. Wonder's program 1 (`040015100B514001`) set a heap its guess did not leave room for |
| `modules main subsdk0 sdk` | `game.tsv` | every module main imports from |

## Files and shaders

- Program 1 is in the update, `updates/65536/program/1/` in the console's layout (`program1/` in the flattened one). Its own RomFS holds one file (`Empty`); it reads program 0's RomFS (the update's, 9,386 files, 5.7 GB) through the program-indexed `MountRom` (`DM_ROMFS_PROGRAM_0`, `dm/host/hle_fs.c`) and add-on `01004D300C5AF001` (Star-Crossed World and the Switch 2 Edition's content, 11,326 files, 10.8 GB).
- 93,312 shaders in the map: 66,010 from the add-on (its `gfx/` models and effects), the rest from program 0's RomFS and main. The `shader_groups` are program 0's (`/gfx/`, `/ptcl/`, `/lyt/`, `/shader/`).
- The survey and the split ran on a folder of program 1's ExeFS, the update's RomFS and the add-on, on Windows: the survey's own steps without the Mac build (no heap check).

## Open

- Running on Android: the memory layout, and whether the game's NVN2 use and the add-on mounts behave as Wonder's program 1 does.
- `shader_groups` and the memory layout are unchecked; the Mac host, where the heap check runs, does not build in this repository.
