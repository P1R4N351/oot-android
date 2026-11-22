// What the tests that link hle_nvn.c and hle_bigstack.c without the host need of it: weak definitions, so a test that defines its own keeps it
#include <stddef.h>
#include <stdint.h>
__attribute__((weak)) uint64_t g_t0_ns;   // process start time (host main.c), read by hle_nvn's per-frame timing
__attribute__((weak)) _Atomic uint64_t dm_present_ns;   // when the last present was made, for DM_HITCH_DUMP (hle_thread.c)
__attribute__((weak)) int dm_guest_as_contains(uintptr_t address) { (void)address; return 0; }   // no guest address space (hle_bigstack.c)
__attribute__((weak)) void dm_vi_layer_crop(int c[4]) { c[0] = c[1] = c[2] = c[3] = 0; }   // no vi layer crop (hle_vi.c), read by hle_nvn's present
__attribute__((weak)) uint64_t dm_guest_unmapped(uintptr_t a, size_t n) { (void)a; (void)n; return 0; }   // every page counts as mapped (hle_svc.c), read by hle_nvn's shader and present checks
__attribute__((weak)) void dm_vi_frame_queued(uint64_t frame) { (void)frame; }   // no vi layer frame history (hle_vi), written by hle_nvn's present
__attribute__((weak)) int nxhost_fps_limit_get(void) { return 0; }   // no frame limit (hle_fpslock.c); Mach-O needs a definition of a weak reference at link time
__attribute__((weak)) int dm_fps_hold60(void) { return 0; }   // no Hold 60 fps (hle_fpslock.c), likewise
