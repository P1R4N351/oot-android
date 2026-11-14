// How a texture-to-texture copy is done on Vulkan (vka_exec.c copy_t2t)
#ifndef VKA_T2TPATH_H
#define VKA_T2TPATH_H
enum {
    T2T_COPY = 1,     // vkCmdCopyImage: same size, no mirror, same block size and sample count
    T2T_BLIT,         // vkCmdBlitImage: scaled or mirrored, between formats of one kind
    T2T_DEPTH_DRAW,   // a full-viewport draw that samples the source and writes gl_FragDepth
    T2T_NONE,         // needs a draw the replay has not got: not done
};
// exact: a plain copy is possible; skind, dkind: vka_fmt.kind of the source and destination (0 float / unorm / snorm, 1 uint, 2 sint, 3 depth, 4 stencil, 5 depth-stencil);
// compressed: either is block-compressed; ms: either is multisampled; blit_dst: the destination's VkFormat has VK_FORMAT_FEATURE_BLIT_DST_BIT (optimal tiling).
static inline int t2t_path(int exact, int skind, int dkind, int compressed, int ms, int blit_dst) {
    (void)blit_dst;
    if (exact) return T2T_COPY;
    if (skind != dkind || (skind && skind < 3) || compressed || ms) return T2T_NONE;
    return T2T_BLIT;
}
#endif
