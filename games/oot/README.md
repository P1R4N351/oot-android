# The Legend of Zelda: Ocarina of Time (Switch 2) — M0 Inventory, Reference Audit & M1 Gap Analysis

Status: **M0**, **M1**, **M2a**, **M2b**, **M2c**, and **M3** (`f5ac42716d0b`: `OoT-Native-0.2.0.apk` + `OoT-Native-0.2.0-dev.apk`, `companion_state.h` waist, Bifrost LED bridge, procedural launcher artwork, controller-first focus) complete on `0400486024DEC000` (`v131072`); **M4** (Display 4 second-screen companion + on-device `0.2.0` Bifrost / GPU lever profiling) is next. Roadmap, milestones, and technical hurdles (`H1`–`H5`): [../../MILESTONES.md](../../MILESTONES.md). Second-screen tie-in ledger: [SECOND_SCREEN_NOTES.md](SECOND_SCREEN_NOTES.md).

---

## 1. Title & Container Identity

| Field | Value |
|---|---|
| Title | The Legend of Zelda: Ocarina of Time (Nintendo Switch 2 Edition) |
| Title ID | `0400486024DEC000` (update `0400486024DEC800`) |
| Container | NUP v1 (`NUP\x01`, header v1, unencrypted ZSTD extents, 2 packages, `1,199` dirs, `372,587` total file entries) |
| Inspected `.nup` size / SHA-256 | `25,622,267,860` B / `85b08e81a2a08612f88de1063c8581fe088f7554303aa1ff207972d729b5916f` (CRC32 `9B6004D4`) |
| NUP TOC offset / size / SHA-256 | `128` / `36,814,592` B / `0cb62756768999d3f0eab2690d75a262521d4a5c3bebb7ea1cea5d3e60618ca3` |
| Engine | Nintendo EPD **Project Owl** (`sead` / `agl` / `gsys` / `Phive` — same engine family and `gmd::GameDataMgr` / `Pack/ZsDic.pack.zs` / `Mals/` layout as *Tears of the Kingdom*) |
| NintendoSDK / NVN | `NintendoSDK 22.2.8` for Switch 2 (`Ounce`), `NintendoSDK_NVN-22_2_8-Release` (NVN2 API 231.40), `NintendoSDK_movie-22_2_8-Release` |
| Target hardware | AYN Thor (`kalama` / Snapdragon 8 Gen 2 / QCS8550, **Adreno 740**, **16 GB RAM**, Android 13, top display 1080p + bottom companion display `Presentation` ID 4) |

### `main.npdm` Parameters (`v131072`)

- `META`: `name = Application`, `program_id = 0400486024DEC000`, `mmu_flags = 0x37` (64-bit, 39-bit address space), `main_thread_priority = 44`, `main_thread_core = 0`, `system_resource_size = 0x3000000` (`48 MiB`), `main_thread_stack_size = 0x100000` (`1 MiB`), Switch 2 `ACID` size `0x4d4` (`0x300`-byte signature/key block vs Switch 1's `0x200`).
- Startup heap: requests a **`6.20 GiB` heap** inside a `9 GiB` Switch 2 application pool (`12 GiB` reservation + `8 GiB` alias region avoids `OptimizeChangeBanc` stalls in Kokiri Forest). Fits comfortably on the 16 GB AYN Thor (`15.62 GB` total, `~11.6 GB` available).
- Declared `ACI0` services (`46`): `acc:u0`, `aoc:u`, `apm`, `appletOE`, `aud:u`, `auddmg:u`, `audsmx:u`, `banana`, `bcat:u`, `bsd:u`, `caps:su`, `caps:u`, `chat:u`, `csrng`, `ectx:aw`, `friend:u`, `fsp-srv`, `hid`, `htc`, `htc:tenv`, `htcs`, `hwopus`, `imf`, `irs`, `ldn:u`, `ldr:ro`, `lm`, `mii:u`, `mnpp:app`, `nfc:mf:u`, `nfc:user`, `nfp:user`, `ngc:u`, `nifm:u`, `nim:eca`, `notif:a`, `nsd:u`, `ntc`, `pctl`, `pl:u`, `prepo:u`, `set`, `sfdnsres`, `ssl`, `stpl:u`, `time:u`.

---

## 2. ExeFS & RomFS Inventory

All `PKG[1]` (`v131072`) ExeFS SHA-256 digests and Build IDs match `MacNativeGames/games/oot/assets/expected-v131072.json` and `BananaNative/dm/games/0400486024DEC000/game.tsv` byte-for-byte (pinned in [assets/expected-v131072.json](assets/expected-v131072.json)).

### `PKG[1]` (`v131072` / `2.0.0`)

| Module | NSO Size (B) | Flags | NSO SHA-256 | Build ID (first 20B) | `text` / `rodata` / `data` / `bss` (B) | Imports / Exports | `RELA` / `RELR` / `JMPREL` (B) |
|---|---:|---|---|---|---|---:|---:|
| `rtld` | 10,524 | `0x3f` (LZ4) | `219f25ef4a64a704e86dacfeb25d9e8977045f2145e629268b94baf34d5e0bbe` | `8e213f228d12c7b0f8c21c3ce81b95e214417369` | `10,048` / `2,532` / `49,160` / `65,528` | 2 / 0 | `24` / `16` / `24` |
| `main` | 29,953,289 | **`0xff` (ZBIC)** | `7db9ddfda088c22d219ef13496decd7077c68cd67d4b37f53f4ac46a082768f2` | `e0f3ccfaf50065ae4ea01eb5aeb13b1a45da3348` | `63,201,744` / `12,603,520` / `5,217,152` / `1,414,272` | 1,183 / 65 | `49,224` / `81,104` / `26,640` |
| `subsdk0` | 533,396 | `0x3f` (LZ4) | `0d32c4bdbcfcfa1c4110c37eff18eaf70061a55090b7fedc741a2e37224065ba` | `b0110087a7b339ef74588d3bd0b4dd5d5c7abd0f` | `542,576` / `234,548` / `69,648` / `65,520` | 258 / 824 | `11,088` / `128` / `12,144` |
| `sdk` | 7,833,714 | `0x3f` (LZ4) | `3ae3cd4cf340d72bdaba9e6f66016a74c1eae13737a5f7299375ed6654b9b1c5` | `1643a838814f9c10c696f930ed681e7fa3fb9af8` | `8,906,832` / `6,093,948` / `435,448` / `1,862,408` | 8 / 28,698 | `260,568` / `4,248` / `241,968` |
| `main.npdm` | 1,828 | — | `94c154d83cbaca00d092958cc2b3a0494d7308fe49de8cb33ece7e9e67303ea7` | — | — | — | — |

- `main` `DT_NEEDED`: `['movie.nss', 'nnSdk.nss']`.
- `PKG[0]` (`v65536` / `1.0.0`): `main` `29,840,183` B (`efe7b93d...`, Build ID `33fa6c2c23848282a77e8b88a0a93f4887c55920`), `sdk` `7,834,834` B (`f9286045...`, Build ID `5fd6d40b...`), `subsdk0` `533,397` B (`0b63d94d...`, Build ID `ecf61f74...`), `rtld` `10,521` B (`ecc75209...`, Build ID `599a9175...`).
- **RomFS (`v131072`)**: `186,583` files (`26,236,422,768` uncompressed bytes), dictionary-compressed with `Pack/ZsDic.pack.zs` (`SARC` containing 4 Zstandard dictionaries). Ships 16 message locales in `Mals/` (`JPja`, `USen`, `EUen`, `EUfr`, `USfr`, `EUde`, `EUit`, `EUes`, `USes`, `EUnl`, `USpt`, `EUru`, `KRko`, `CNzh`, `TWzh`, `EUpl`; passing unsupported `th` crashes `main+0xa7ec18`).
- **Saves**: identical container format to *Tears of the Kingdom* (`slot_00`..`slot_05` holding `progress.sav` + `caption.sav`, plus root `option.sav`; `progress.sav` header magic `0x01020304`, version `0x002D80EB`, 8-byte `(murmur3_key, u32_val)` pairs + 64-bit sorted collected-object table at `0xA3DB7114`).

---

## 3. Reference Codebases & Licence Audit

| Project | Role | Licence | Key Findings for `0400486024DEC000` |
|---|---|---|---|
| **Hokkaido** (`pavelgajdo/Hokkaido`, branch `nx2`) | Switch 2 Android emulator (+ embedded Banana Native in 0.7.0) | **GPL-3.0-or-later** | Loads `.nup` via `AppLoader_NUP` (`nup_container.cpp`), synthesizes 16-slot `NACP`, decodes `ZBIC`. In 0.6.6 (NCE emulator engine), OoT hits title screen ~60 fps then stalls/OOMs due to unbypassed NDVE movie tensor passes (`HMMA`), guest page write-protection churn, syncpoint waits (`nx2_gsv`), and `vm.max_map_count` (`65,530`) exhaustion on 12–16 GB Android kernels. |
| **Eden SX2A** (`JaviiDeveloper/eden-nx2-android` / PokeDinner fork) | Experimental Switch 2 Android emulator (unmaintained) | **GPL-3.0-or-later** | Aborts at boot (`0xF001`): predates SDK 22 `ZBIC` (`flags & 0x80`) segment decompression on `main` (`flags=0xff`) and lacks SDK 22.2.8 `Ounce` / 64-bit address-arbiter service coverage. Read-only reference only. |
| **MacNativeGames** (`ly05010419/MacNativeGames`, `games/oot` & `docs/ns2-port.md`) | Native AArch64 macOS/iOS port of Switch 2 OoT `v131072` | **MIT** (`dav1d` BSD-2-Clause; external `zbic` PyPI build tool GPLv2, not linked) | Proves full native AArch64 gameplay in Kokiri Forest on Apple Silicon (65–83 fps on M4): NUP reader (`a55uka/nup`, MIT), `patch_nso.py` + `RELR`, SDK 22.2.8 services, AV1 (`hle_movie_av1.c`), real-time frame step (`main+0x6e6cc4`, `0x3b4eaf8`), and `42,845` Ampere `sm86` shaders. **Gap for Android:** renders via **Apple Metal 4 native hardware mesh shaders** + MetalFX; no Vulkan backend and no compute-mesh lowering for GPUs lacking `VK_EXT_mesh_shader`. |
| **Banana Native** (`moceure/BananaNative`, branch `android`, based on `seedds/nxnative`) | Native AArch64 Android + Vulkan 1.3 Switch 2 compatibility layer | **MIT core + GPL-3.0-or-later combined APK** (`InstEmitAmpereMesh.cs`, `vka_memu.c`, `lsfg` are GPL-3.0) | **Already runs OoT `0400486024DEC000` (`v131072`) natively into 3D gameplay on Snapdragon/Adreno (~30–33 fps in Kokiri Forest)**: `patch_nso.py --target android`, SDK 22.2.8 Android host (`AAudio`, `AMediaCodec` AV1), NVN2→Vulkan (`dm/gfx/vk_android`), Ampere `sm86`→SPIR-V (`libdmsh_xlate.so`), **Vulkan compute-mesh lowering (`vka_memu.c`)** for GPUs without `VK_EXT_mesh_shader`, Adreno 740 null-descriptor fix (#249), Turnip `vm.max_map_count` VMA coalescing (#127/#132), and OoT `v131072` profile/patches (`dm/games/0400486024DEC000/`). |
| **NaviScout** (`jfhutchi/NaviScout`), **NX2navi-Trainer** (`Miragui/NX2navi-Trainer`), **OOT-SW2SE** (`Traiver/OOT-SW2SE`) | Live memory reader / radar & offline save workshop for `v131072` (`E0F3CCFAF50065AE`) | **MIT** | Complete verified `gmd::GameDataMgr` hash/index tables, `Player` XYZ + camera `<6d>` pointers, transition/dim flags, and `progress.sav` / `caption.sav` parser — directly feeding [SECOND_SCREEN_NOTES.md](SECOND_SCREEN_NOTES.md). |

---

## 4. Empirical Gap List Against `P1R4N351/botw-android` (`3bb123a`)

Measured directly against `PKG[1]` (`v131072`) modules and live `cmd gpu vkjson` on the AYN Thor (`Adreno (TM) 740`, Vulkan `1.3.128` + bundled Mesa Turnip `27.0.0-devel`):

### 4.1 Container & Module Compression
1. **`.nup` v1 container (`NUP\x01`)**: `botw-android` expects Switch 1 NCAs/RomFS; requires `a55uka/nup` (MIT, 4 small Python modules) to stream/extract verified ExeFS and RomFS extents.
2. **SDK 22 `ZBIC` NSO0 compression (`flags & 0x80`)**: `rtld`, `subsdk0`, and `sdk` use `flags = 0x3f` (LZ4), whereas `main` uses **`flags = 0xff`** (bit 7 selects Nintendo's SDK 22 `ZBIC` compression rather than LZ4). Offline preparation requires external `zbic` decompression prior to rewriting (never linked into the app).

### 4.2 AArch64 Binary Rewriter (`patch_nso.py` / `structck.py`) & ISA
1. **ARMv8.2/v8.3 instructions in `main` (`63.2 MB` `.text`)**:
   - `126,456` `retab`/`retaa` (PAC), `70,970` `ldapr` (RCPC acquire), `24,536` LSE atomics (`ldadd`/`cas`/`swp`), **`0` plain `ldxr`** (no Switch 1 `sead` weak-acquire spin-lock hazard in `main`; `sdk` has `3` `ldaxr` and `0` plain `ldxr`), `1,264` `mrs tpidr_el0`, `0` `svc`.
   - Unlike CPU emulators (Dynarmic), **Snapdragon 8 Gen 2 (Cortex-X3 / A715 / A510, ARMv9.0-A) executes PAC, RCPC `ldapr`, and LSE atomics natively in silicon with zero rewrite overhead**.
2. **`DT_GNU_HASH` without `DT_HASH` (empirical test on `botw-android` `patch_nso.py`)**:
   - `subsdk0` (`3,369` sites, `0` gates) and `sdk` (`25,983` sites, `102` gates) **pass `botw-android`'s unmodified `patch_nso.py` (`DM_HOST_TP=tpidr_el0`)**.
   - `main` failed `botw-android`'s `patch_nso.py` at line 620 (`KeyError: 4`) solely because SDK 22 emits `DT_GNU_HASH` (`0x6ffffef5`) without legacy `DT_HASH` (`4`). With the 1-line `(tags[5] - tags[6]) // 24` fallback and uncompressed `ZBIC` segments, **`main` passes `patch_nso.py` (`DM_HOST_TP=tpidr_el0`) on the first run**: `51,654` slot refs (`plain=48889`, `tpidr_r=1264`, `cond=861`, `pcrel=541`, `br=94`, `call_window=5`), `0` gates, `PATCH_OFF=0x4eb0000`, patch size `58,959,876` B (within ±128 MiB `B` reach of ordered sites).
3. **`RELR` relative relocations**: `main` carries `81,104` B of `DT_RELR` (`tag 35`), `sdk` `4,248` B, `subsdk0` `128` B; runtime `loader.c` must apply `DT_RELR` alongside `DT_RELA` (`botw-android`'s `patch_nso.py` already decodes `relr.py` offline, but `games/totk/runtime/loader.c` needs runtime `DT_RELR` application).

### 4.3 Switch 2 (`Ounce`) Kernel SVCs & Horizon Services
1. **New SVCs in `sdk`**: in addition to Switch 1 SVCs, `sdk` invokes **`svc 0x35` (`WaitForAddress`)**, **`svc 0x36` (`SignalToAddress`)** (with 64-bit arbiter variants in SDK 22), **`svc 0x4b`**, **`svc 0x70`**, **`svc 0x72`**, and **`svc 0x80` (`Ounce` custom syscall)**, plus `GetInfo` ID `0x21` (VAMM granule) and a `9 GiB` application memory pool (`12 GiB` virtual reservation + `8 GiB` alias region).
2. **Switch 2 (`Ounce`) services & codecs**:
   - `appletOE`: `oe::GetCurrentGraphicsScale` (`main+0xe376d8`) + handheld/console display crop table (`main+0x3f58540` — no Switch 1 `movz w0,#imm; ret` framebuffer getters).
   - `hid`: `nn::hid::IsOunceVibrationDeviceMounted` (without it, playing an Ocarina note aborts!).
   - `movie.nss` / `nn::codec`: **AV1** (`1920x1080`) hardware/software decode (`AMediaCodec` / `dav1d`) + **multi-stream Opus** (`ns2_opus_ms.c`), plus bypassing the `47.5 ms/frame` NDVE movie neural-enhancement pass.
   - `audsmx:u` / `auddmg:u` / `banana` / `stpl:u` / `imf` / `mnpp:app`: Switch 2 service stubs.

### 4.4 NVN 2 Graphics Layer (`259` Missing Entry Points)
Comparing all `nvn*` symbols in `v131072` `main` + `sdk` (`775` functions) against `botw-android` `d2r_gfx/gfx/nvn/names.txt` (`523` NVN 1 functions):
- **`516` match**, **`259` NVN 2 entry points are absent** in `botw-android`:
  - **Mesh/task draws (`8`)**: `nvnCommandBufferDrawMeshTasks{,3D,Indirect,3DIndirect}`, `MultiDrawMeshTasks{,3D}Indirect{,Count}`.
  - **Push constants & deferred bindings (`10`)**: `Push{Graphics,Compute}Constants{,Deferred}`, `Bind{Textures,SeparateTextures,SeparateSamplers,StorageBuffers,UniformBuffers,Images}Deferred`.
  - **NVN 2 descriptor tables & builders**: `nvnProgramInitializeFromBuilder`, `nvnProgramBuilderSetShaders`, `nvnProgramGetScratchBytesPerWarp`, `nvnTexturePoolWriteDescriptorTableData`, `nvnSamplerPoolWriteDescriptorTableData`, multiple active descriptor pools (`OP_SET_TEX_POOL` per-pool capacity tracking).
  - **DLSS / Cooperative Vector (`48`)**: `nvnDlssNetwork*`, `nvnDlssParameters*`, `nvnCommandBufferEvaluateDlss`, `ConvertCooperativeVectorMatrix` (stubbed/bypassed when native/SGSR1 upscaling is used).
  - **Switch 2 GOB deswizzle, events & syncpoints (`nvnEvent*`, `nvnSync*SyncpointsNVX`, `CopyEngineNVX`, `DirtyTileMap*`, `DeviceCommandBuffer*`)**.

### 4.5 Shader ISA (`Ampere sm86`) & Adreno 740 Hardware Constraints (Critical Gate)
1. **Binary format & corpus**: `5` `.Nin_Ounce_NVN2.bfsha.zs` archives (`FSHA` → `BNSH`), NVN 2 control magic **`0x98761234` version `0xc9`**, **NVIDIA Ampere `sm86` SASS** (`42,845` total shaders: `15,851` vertex, `22,352` pixel, **`20` task**, **`500` mesh**, `4,122` compute — task/mesh sit in BNSH program slots 7 & 8; includes `ISBEWR`/`ISBERD` output-payload reads, `F2IP`, surface atomics, and `HMMA` tensor ops). `botw-android`'s `libtotk_xlate.so` decodes Switch 1 **Maxwell (`0x00220015`)** only.
2. **Live Adreno 740 (`AYN Thor`) Vulkan Audit**:
   - `VK_EXT_shader_atomic_float`: **PRESENT** (handles OoT's 34 float-atomic shaders natively).
   - `VK_KHR_buffer_device_address`, `VK_EXT_descriptor_indexing`, `VK_KHR_dynamic_rendering`, `VK_KHR_synchronization2`: **PRESENT**.
   - `VK_EXT_map_memory_placed`: **ABSENT on stock Qualcomm `1.3.128`**, **PRESENT via bundled Mesa Turnip `27.0.0-devel` (`libtotk_turnip.so`)** + VMA coalescing to stay under Android's `vm.max_map_count = 65,530`.
   - **`VK_EXT_mesh_shader`: ABSENT on Adreno 740** (both stock Qualcomm `1.3.128` and Mesa Turnip `a740`). **Therefore, OoT's `20` task + `500` mesh shaders MUST be lowered to Vulkan compute dispatches (`vka_memu`).**

---

## 5. Architecture Options & Effort Estimate

| Path | Approach | Licence | Estimated Effort to Thor Gameplay | Risks & Trade-Offs |
|---|---|---|---|---|
| **Path A (Recommended)** | Build **OoT Native (`oot-android`)** on **`moceure/BananaNative`** (`seedds/nxnative` Android port, which already implements NVN2→Vulkan, Ampere `sm86`→SPIR-V, `vka_memu` compute-mesh lowering on Adreno, and OoT `v131072` patches at ~30–33 fps) + integrate `botw-android`'s bundled **Mesa Turnip `27.0-devel`**, Thor controller menu/identity, **`companion_state.h` waist**, **Bifrost LED bridge**, and a new **`0400486024DEC000` Eden Duo second screen** (`Presentation` on display 4). | **GPL-3.0-or-later** as a whole (identical license tier to `BotW Native` & `TotK Native` APKs carrying `Eden Duo`; full source kept in public GitHub/Gitea lockstep). | **~2–4 days** to verified 3D gameplay in Kokiri Forest on the 16 GB AYN Thor; **+3–5 days** for the OoT Eden Duo second-screen companion. | Keeps `P1R4N351/botw-android` 100% MIT-clean for upstreaming while avoiding reinventing ~10,000+ lines of NVN2 + Ampere + compute-mesh compiler plumbing. Watch Turnip `ir3` register spills on heavy vertex shaders (`4671e5e647fc5455`, ~6–7 s cold compile — mitigated by `vk_prewarm.bin` seed) and `vm.max_map_count` over long play sessions. |
| **Path B** | Port Switch 2 (`ZBIC`, `RELR`, SDK 22.2.8 services, AV1, `259` NVN2 entry points, Ampere `sm86` translator, and clean-room MIT Vulkan compute-mesh lowering) from scratch into `botw-android`'s MIT `common` branch. | MIT core + GPL-3.0 `duo/` | **~6–10+ weeks** | Massive duplication of solved work; clean-room reimplementation of Ampere task/mesh → Vulkan compute lowering is high-risk compiler engineering with zero user-visible advantage once `Eden Duo` already makes the distributed APK GPL-3.0-or-later. |
| **Path C (Fallback)** | Route B: N64 decompilation (*Ship of Harkinian* on Android + `gSaveContext`/`gPlayState` second screen on display 4). | CC0 / MIT / GPL (depending on modules) | **~3–5 days** | Runs the 1998 N64 asset/engine build rather than the owner's Switch 2 remake (`0400486024DEC000`). Kept strictly as fallback if Switch 2 gameplay hits a showstopper on Adreno 740. |

---

## 6. Proposed Milestones (Upon Path Confirmation)

- **M0–M1 (Complete)**: Verify `.nup` & ExeFS digests, audit Hokkaido / Eden SX2A / MacNativeGames / BananaNative, measure Adreno 740 Vulkan extensions, test rewriter, establish [SECOND_SCREEN_NOTES.md](SECOND_SCREEN_NOTES.md).
- **M2 (Pack & Cold/Warm Boot on 16 GB Thor)**: Stage `v131072` pack + bundled Mesa Turnip `27.0-devel` (`VK_EXT_map_memory_placed`), verify Kokiri Forest 3D gameplay, frame pacing (30 / 60 fps), audio, AV1 cutscenes, Ocarina vibration query, and save round-trip (`slot_00/progress.sav`), logging VMA count (`/proc/<pid>/maps`) and thermals.
- **M3 (Thor App Identity & Bridges)**: `io.github.p1r4n351.oot` package, controller-first launcher/settings, procedural launcher icon (no Nintendo art), `companion_state.h` waist, and Bifrost stick-LED integration.
- **M4 (Second-Screen Companion on Display 4)**: Author `OcarinaOfTime` (`0400486024DEC000`) Eden Duo native module + declarative `1240x1080` pages following `companion-convergence.md` (Rodin NTLG B font decoded on-device from `Font/*.bfarc.zs`, north-up/rotating scene & dungeon minimap with Link XYZ + camera forward, item/equipment grids, C-button assignment with safe-state refusal reasons, learned Ocarina songs reference + optional live note playback, Gold Skulltula & Spiritual Stone / Medallion status).

---

## 7. M2a / M2b Build & Verification Snapshot (2026-11-18)

| Artifact | Verified Output | Notes |
|---|---|---|
| Base integration commit | `538ff8a29131` (`main`) | Upstream `BananaNative` `af1ed596` + `-PnxApplicationId` / `-PnxLabel`; upstream `gradle.properties` kept intact (`test_android_games.py` green) |
| Dev APK | `OoT-Native-0.1.0-dev.apk` (`9,236,908` B, SHA-256 `ec4e5410bcec523adc30706dfb33b9c12a64405efdf2fb77c5846100623133ce`) | `io.github.p1r4n351.oot.dev`, `arm64-v8a`, `NX_DMSH_VK_CHAIN=19662e4c4a25`, `NX_DMSH_VK_ABI=4fbeb79f5ab7`, `NX_LSFG=true`, `NX_PACK_ENGINE=fake`, `nintendo_ext.ttf` (`29` glyphs); `apksigner` verified |
| Turnip driver zip | `turnip-27.0.0-cs-slabs-adreno740.zip` (`2,708,741` B, SHA-256 `6ed3a8369e058b807f731dbea2616fd6aecaf470eb26c5ed413b5a990ce27650`) | Wraps Mesa Turnip `libtotk_turnip.so` (`14,070,224` B, `9b87afeb3d8d30c6a0ad5aab3e0703bc1500e0321e9f17a922106fa556389ec6`), `minApi=33` |
| Rewritten `v131072` modules | `main.patched.bin` (`85,148,044` B), `sdk.patched.bin` (`18,508,604` B), `subsdk0.patched.bin` (`1,131,516` B) | `51,739` / `25,995` / `3,371` slot refs; `85` / `114` / `2` gates; `0` failures |
| Shader library (`dmsh_vk.lib`) | `v-19662e4c4a25` (`695,628,760` B, SHA-256 `06314ba3c4b7c3893d219741cf1e3d1958552ce3ee85b788c14f86793473c9b7`) | `42,843 / 42,848` shaders (`52,858` SPIR-V modules passing `spirv-val --target-env vulkan1.3` via `SPIRV-Tools v2025.1`); `5` point-mesh keys (`2582bd7ea6490d33`, `28f82c129deea140`, `3a6340a04bf09797`, `58d06e7230b59741`, `a203347bb7bdbd03`) omitted on `VUID-CullPrimitiveEXT-CullPrimitiveEXT-07036` |
| Pack manifest & overlays | `android-pack` (`766 MiB` apparent) | `pack.tsv` (`format 1`), `7` `scene_res` overlays, `71`-line `cheats/E0F3CCFAF50065AE.txt`, `.nup:update/131072/romfs` external stream |
| M2c Pass 1 & 2 | `d5d735664c8b` + `a5fc6b359894` (`OoT-Native-0.1.0-dev.apk`, `9,400,900` B, SHA-256 `6f4755b8e546191025ed0fd0bbc5a81249d9e31237b39d6d2c2392dde0f6ca3e`) | Pass 1 (`22.4 min`, `26,109` frames, `0` crashes, `170 MiB` / `3,320` cached pipelines, `267,000` `audren` frames `0` underruns, `25,246 / 65,530` VMAs); Pass 2 (`206 ms` warm pipeline load, `60.0–60.2 fps` title, stable `15 fps` Kokiri Forest playback with `1.000x` synced audio under Turnip `driver.id=18` defaults) |
| M3 Waist, Bifrost & `0.2.0` APKs | `f5ac42716d0b` (`OoT-Native-0.2.0.apk`, `9,278,180` B; `OoT-Native-0.2.0-dev.apk`, `9,278,356` B) | Added `companion_state.h` + `oot_companion.h/.c` waist (`20 Hz` reader, `0x4BE4FB0` 3-frame pause hold, Ocarina `page 4`), pure-JVM `CompanionState` / `BifrostLooks` + `BifrostBridge` (`DM_BIFROST`, localized `GameSettings` toggle), procedural vector launcher icons & `ic_banner.xml` (`scripts/make_oot_launcher_art.py`), and `PadFocus.hasGamepad()`; SHA-256 `20d6dc4aa3ff2d4820694c8567c8d6753762205e2373d3dc09a14c7c01db92ee` (`io.github.p1r4n351.oot`) & `042324786f867e6b2a305634f8f4ba3efb1178c0dbaadf50b8d15e23f1442040` (`io.github.p1r4n351.oot.dev`) |

Support this port: [README](../../README.md#support)
