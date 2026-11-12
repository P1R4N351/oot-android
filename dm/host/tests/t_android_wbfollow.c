// Which tracked textures follow the CPU's writes to their storage (dm/gfx/vk_android/vka_wbfollow.h): one the GPU never wrote always does; a linear one the GPU wrote does once
// the copy of its GPU content back into the storage has run on the GPU (then the storage is the image, as on the Switch, where a linear texture is plain memory: a glyph cache
// the game clears on the GPU and fills on the CPU, Mario Tennis Fever's 8192x2048 R8); not while that copy is queued or its submission is unfinished, nor when no copy-back
// ever ran; a block-linear one the GPU wrote never does (its content stays the GPU's)
#include <stdio.h>
#include "../../gfx/vk_android/vka_wbfollow.h"

static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

int main(void) {
    CHECK(wbf_follows(0, 0, 0, 0, 0) == 1, "a texture the GPU never wrote stopped following its storage");
    CHECK(wbf_follows(0, 1, 0, 0, 5) == 1, "a linear texture the GPU never wrote stopped following its storage");
    CHECK(wbf_follows(1, 1, 0, 10, 10) == 1, "a GPU-written linear texture whose copy-back has run ignores the CPU's later writes (blank glyph cache)");
    CHECK(wbf_follows(1, 1, 0, 10, 12) == 1, "a GPU-written linear texture whose copy-back ran long ago ignores the CPU's later writes");
    CHECK(wbf_follows(1, 1, 0, 10, 9) == 0, "re-uploaded before its copy-back ran on the GPU: the storage still held older bytes than the image");
    CHECK(wbf_follows(1, 1, 1, 10, 12) == 0, "re-uploaded while a newer copy-back is queued: the GPU's latest write would be lost");
    CHECK(wbf_follows(1, 1, 0, 0, 12) == 0, "re-uploaded though no copy-back ever ran (the storage never held the GPU's content)");
    CHECK(wbf_follows(1, 0, 0, 10, 12) == 0, "a block-linear texture the GPU wrote was re-uploaded from its storage");
    printf("%s: %s\n", __FILE__, fails ? "FAILED" : "ok");
    return fails != 0;
}
