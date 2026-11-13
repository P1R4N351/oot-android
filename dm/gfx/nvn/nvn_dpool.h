// The descriptor pools' ranges in the host's texture and sampler tables (hle_nvn.c "Descriptor pools"): each pool gets a base, its registrations go to base + ID, so two pools
// that use the same IDs do not meet. Free of locks and of NVN, so dm/host/tests/t_nvn_dpool.c can check it; the caller holds its lock.
#ifndef NVN_DPOOL_H
#define NVN_DPOOL_H

#define NVN_NDPOOL 16
typedef struct { const void *pool; int base, cap; } nvn_dpool_ent;   // cap: the entries its range holds (a multiple of 256); pool NULL: a finalized pool's range
typedef struct { nvn_dpool_ent e[NVN_NDPOOL]; int n, next; } nvn_dpool_tab;   // n ranges made, next: where a new one starts (the table's size)

// The pool's base: its own if it has one, else a new range at the table's end. -1: 16 ranges are taken (the caller numbers it from 0, sharing the first pool's IDs)
static inline int nvn_dpool_add(nvn_dpool_tab *t, const void *pool, int n) {
    int need = (n + 255) & ~255;
    for (int i = 0; i < t->n; i++) if (t->e[i].pool == pool) return t->e[i].base;
    if (t->n == NVN_NDPOOL) return -1;
    t->e[t->n++] = (nvn_dpool_ent){pool, t->next, need}; t->next += need;
    return t->e[t->n - 1].base;
}
static inline int nvn_dpool_find(const nvn_dpool_tab *t, const void *pool) {   // its base; 0 when it has none
    for (int i = 0; i < t->n; i++) if (t->e[i].pool == pool) return t->e[i].base;
    return 0;
}
// nvnTexturePoolFinalize / nvnSamplerPoolFinalize
static inline void nvn_dpool_release(nvn_dpool_tab *t, const void *pool) { (void)t; (void)pool; }
#endif
