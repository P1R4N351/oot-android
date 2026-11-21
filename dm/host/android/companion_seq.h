// SPDX-License-Identifier: MIT
// Deterministic, zero-allocation fail-closed button/melody sequencer for
// second-screen triggered musical notes and short input chains.
// Synced from P1R4N351/nx-second-screen (include/nx_companion/companion_seq.h).
#ifndef NX_COMPANION_SEQ_H
#define NX_COMPANION_SEQ_H

#include <assert.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NX_SEQ_MAX_STEPS 16U

typedef struct NxCompanionSeq {
    uint64_t chords[NX_SEQ_MAX_STEPS];
    uint32_t count;
    uint32_t idx;
    uint32_t in_gap;
    int32_t tag;
    uint64_t phase_ns;
    uint64_t hold_ns;
    uint64_t gap_ns;
    int active;
} NxCompanionSeq;

static inline void nx_seq_abort(NxCompanionSeq *s) {
    assert(s != NULL);
    memset(s->chords, 0, sizeof(s->chords));
    s->count = 0U;
    s->idx = 0U;
    s->in_gap = 0U;
    s->tag = -1;
    s->phase_ns = 0ULL;
    s->hold_ns = 0ULL;
    s->gap_ns = 0ULL;
    s->active = 0;
}

static inline void nx_seq_init(NxCompanionSeq *s) {
    assert(s != NULL);
    nx_seq_abort(s);
}

static inline int nx_seq_start(
    NxCompanionSeq *s, const uint64_t *chords, uint32_t count, int32_t tag,
    uint64_t hold_ns, uint64_t gap_ns, uint64_t now_ns) {
    if (!s || !chords || count == 0U || count > NX_SEQ_MAX_STEPS || hold_ns == 0ULL) {
        return 0;
    }
    for (uint32_t i = 0U; i < count; i++) {
        if (chords[i] == 0ULL) return 0;
    }
    nx_seq_abort(s);
    for (uint32_t i = 0U; i < count; i++) {
        s->chords[i] = chords[i];
    }
    s->count = count;
    s->idx = 0U;
    s->in_gap = 0U;
    s->tag = tag;
    s->phase_ns = now_ns;
    s->hold_ns = hold_ns;
    s->gap_ns = gap_ns;
    s->active = 1;
    return 1;
}

static inline uint64_t nx_seq_tick(
    NxCompanionSeq *s, int gate_ok, uint64_t now_ns) {
    if (!s || !s->active) return 0ULL;
    if (!gate_ok || now_ns < s->phase_ns) {
        nx_seq_abort(s);
        return 0ULL;
    }
    for (uint32_t iter = 0U; iter < NX_SEQ_MAX_STEPS * 2U; iter++) {
        uint64_t span = s->in_gap ? s->gap_ns : s->hold_ns;
        if (span == 0ULL) span = 1ULL;
        if ((now_ns - s->phase_ns) < span) break;
        s->phase_ns += span;
        if (!s->in_gap) {
            if (s->idx + 1U >= s->count) {
                nx_seq_abort(s);
                return 0ULL;
            }
            if (s->gap_ns > 0ULL) s->in_gap = 1U;
            else s->idx++;
        } else {
            s->in_gap = 0U;
            s->idx++;
        }
    }
    return s->in_gap ? 0ULL : s->chords[s->idx];
}

#ifdef __cplusplus
}
#endif

#endif
