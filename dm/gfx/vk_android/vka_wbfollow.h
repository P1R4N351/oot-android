// Whether a tracked texture's storage, which the CPU wrote, is uploaded into its image again (vka_tex.c, the prepare's scan of tracked images), free of Vulkan so the Mac's
// unit test (dm/host/tests/t_android_wbfollow.c) can check it.
#ifndef VKA_WBFOLLOW_H
#define VKA_WBFOLLOW_H
#include <stdint.h>

// gpu_owned: the GPU has written the image; linear_wb: a linear texture whose GPU writes are copied back into its storage (vka_tex.c img_wb_kind); wb_pending: a copy-back is
// queued and not yet recorded; wb_fence: the fence of the submission that holds the last copy-back (0: none ran); done: the fence the GPU has passed
static inline int wbf_follows(int gpu_owned, int linear_wb, int wb_pending, uint64_t wb_fence, uint64_t done) {
    (void)linear_wb; (void)wb_pending; (void)wb_fence; (void)done;
    return !gpu_owned;
}
#endif
