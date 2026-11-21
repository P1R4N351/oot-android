// SPDX-License-Identifier: MIT
// Runtime glue between OoT's guest memory / present hook and the common CompanionState waist.
#include "oot_companion.h"
#include "input_android.h"
#include "dm.h"
#include "plat/plat.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

_Static_assert(OOT_BTN_A == DM_BTN_A && OOT_BTN_B == DM_BTN_B &&
               OOT_BTN_X == DM_BTN_X && OOT_BTN_Y == DM_BTN_Y &&
               OOT_BTN_L == DM_BTN_L && OOT_BTN_R == DM_BTN_R,
               "OoT companion button bits must match dm_input.h");

static pthread_mutex_t g_companion_mu = PTHREAD_MUTEX_INITIALIZER;
static OotCompanionCtx g_companion_ctx;
static int g_companion_inited;
static uint64_t g_prev_seq_btn;

static void companion_ensure_init_locked(void) {
    if (!g_companion_inited) {
        oot_companion_ctx_init(&g_companion_ctx);
        g_companion_inited = 1;
        g_prev_seq_btn = 0ULL;
    }
}

static int companion_enabled(void) {
    const char *e = getenv("DM_COMPANION");
    return !(e && strcmp(e, "0") == 0);
}

static int companion_plat_read(void *ctx, uint64_t addr, void *out, size_t len) {
    (void)ctx;
    return plat_mem_read(addr, out, len) == 0 ? 1 : 0;
}

static uint64_t companion_main_base(void) {
    for (int i = 0; i < g_nmod; i++) {
        if (strcmp(g_mod[i].name, "main") == 0) return (uint64_t)(uintptr_t)g_mod[i].base;
    }
    return 0ULL;
}

static void companion_push_seq_buttons(uint64_t seq_btn, int force_push) {
    if (force_push || seq_btn != g_prev_seq_btn) {
        g_prev_seq_btn = seq_btn;
        nxhost_companion_buttons_set(seq_btn);
    }
}

void oot_companion_note_canvas(uint64_t vtable_rel) {
    if (pthread_mutex_lock(&g_companion_mu) != 0) return;
    companion_ensure_init_locked();
    oot_companion_ctx_note_canvas(&g_companion_ctx, vtable_rel);
    (void)pthread_mutex_unlock(&g_companion_mu);
}

void dm_companion_present(uint64_t frame) {
    (void)frame;
#ifdef DM_GAME_ID
    if (strcmp(DM_GAME_ID, "0400486024DEC000") != 0) return;
#endif
    int en = companion_enabled();
    const char *build = dm_main_build();
    uint64_t base = companion_main_base();
    uint64_t now_ns = plat_clock_ns(CLOCK_MONOTONIC);
    OotTapMem mem = {companion_plat_read, NULL};
    uint64_t seq_btn = 0ULL;
    int push = 0;
    if (pthread_mutex_lock(&g_companion_mu) != 0) return;
    companion_ensure_init_locked();
    oot_companion_ctx_present(&g_companion_ctx, en, build, &mem, base, now_ns);
    seq_btn = g_companion_ctx.seq_buttons;
    if (seq_btn != g_prev_seq_btn) {
        g_prev_seq_btn = seq_btn;
        push = 1;
    }
    (void)pthread_mutex_unlock(&g_companion_mu);
    if (push) nxhost_companion_buttons_set(seq_btn);
}

int nxhost_companion_state(int64_t out[COMPANION_STATE_FIELDS]) {
    if (!out) return 0;
    CompanionState st;
    uint64_t st_ns = 0ULL;
    if (pthread_mutex_lock(&g_companion_mu) != 0) return 0;
    companion_ensure_init_locked();
    st = g_companion_ctx.state;
    st_ns = g_companion_ctx.state_ns;
    (void)pthread_mutex_unlock(&g_companion_mu);
    if (!companion_enabled()) {
        companion_state_clear(&st);
        companion_state_words(&st, 0, out);
        return 0;
    }
    int64_t age_ms = -1;
    if (st_ns > 0ULL) {
        uint64_t now_ns = plat_clock_ns(CLOCK_MONOTONIC);
        age_ms = (now_ns >= st_ns) ? (int64_t)((now_ns - st_ns) / 1000000ULL) : 0;
    }
    companion_state_words(&st, age_ms, out);
    return (st.status != COMPANION_STATE_NONE && st_ns > 0ULL) ? 1 : 0;
}

void nxhost_companion_set_page(int page) {
    if (pthread_mutex_lock(&g_companion_mu) != 0) return;
    companion_ensure_init_locked();
    g_companion_ctx.user_page = (int32_t)page;
    (void)pthread_mutex_unlock(&g_companion_mu);
}

int nxhost_companion_detail(
    int32_t *ints, int max_ints, float *flts, int max_flts, char *txt, int max_txt) {
    if (!companion_enabled()) return 0;
    if (pthread_mutex_lock(&g_companion_mu) != 0) return 0;
    companion_ensure_init_locked();
    int ready = (g_companion_ctx.state.status != COMPANION_STATE_NONE && g_companion_ctx.state_ns > 0ULL);
    if (ready) {
        oot_companion_ctx_pack_detail(&g_companion_ctx, ints, max_ints, flts, max_flts, txt, max_txt);
    }
    (void)pthread_mutex_unlock(&g_companion_mu);
    return ready ? 1 : 0;
}

int nxhost_companion_play_song(int song_idx, int classic_mode) {
    if (!companion_enabled()) return 0;
    uint64_t now_ns = plat_clock_ns(CLOCK_MONOTONIC);
    int ok = 0;
    uint64_t seq_btn = 0ULL;
    if (pthread_mutex_lock(&g_companion_mu) != 0) return 0;
    companion_ensure_init_locked();
    ok = oot_companion_ctx_play_song(&g_companion_ctx, song_idx, classic_mode, now_ns);
    seq_btn = g_companion_ctx.seq_buttons;
    if (ok) g_prev_seq_btn = seq_btn;
    (void)pthread_mutex_unlock(&g_companion_mu);
    if (ok) nxhost_companion_buttons_set(seq_btn);
    return ok;
}

int nxhost_companion_tap_note(int note_idx, int classic_mode) {
    if (!companion_enabled()) return 0;
    uint64_t now_ns = plat_clock_ns(CLOCK_MONOTONIC);
    int ok = 0;
    uint64_t seq_btn = 0ULL;
    if (pthread_mutex_lock(&g_companion_mu) != 0) return 0;
    companion_ensure_init_locked();
    ok = oot_companion_ctx_tap_note(&g_companion_ctx, note_idx, classic_mode, now_ns);
    seq_btn = g_companion_ctx.seq_buttons;
    if (ok) g_prev_seq_btn = seq_btn;
    (void)pthread_mutex_unlock(&g_companion_mu);
    if (ok) nxhost_companion_buttons_set(seq_btn);
    return ok;
}

void nxhost_companion_abort_seq(void) {
    if (pthread_mutex_lock(&g_companion_mu) != 0) return;
    companion_ensure_init_locked();
    oot_companion_ctx_reset_seq(&g_companion_ctx);
    g_prev_seq_btn = 0ULL;
    (void)pthread_mutex_unlock(&g_companion_mu);
    companion_push_seq_buttons(0ULL, 1);
}

int nxhost_companion_pin(int op, int slot, const char *label) {
    int rc = -1;
    if (pthread_mutex_lock(&g_companion_mu) != 0) return -1;
    companion_ensure_init_locked();
    rc = oot_companion_ctx_pin(&g_companion_ctx, op, slot, label);
    (void)pthread_mutex_unlock(&g_companion_mu);
    return rc;
}
