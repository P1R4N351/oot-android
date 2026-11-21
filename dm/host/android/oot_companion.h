// SPDX-License-Identifier: MIT
// Ocarina of Time (Switch 2 v131072, build e0f3ccfa) adapter feeding companion_state.h,
// 3D spatial breadcrumb trail, waypoint pins, and fail-closed Ocarina note sequencer.
#pragma once

#include "companion_seq.h"
#include "companion_state.h"
#include "companion_trail.h"
#include "oot_companion_tap.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OOT_MAIN_BUILD "e0f3ccfaf50065ae4ea01eb5aeb13b1a"
#define OOT_SCREEN_VT_PAUSE 0x4BE4FB0ULL
#define OOT_PAUSE_HOLD_FRAMES 3U
#define OOT_COMPANION_SAMPLE_INTERVAL_NS 50000000ULL
#define OOT_NOTE_HOLD_NS 130000000ULL
#define OOT_NOTE_GAP_NS   70000000ULL

#define OOT_BTN_A (1ULL << 0)
#define OOT_BTN_B (1ULL << 1)
#define OOT_BTN_X (1ULL << 2)
#define OOT_BTN_Y (1ULL << 3)
#define OOT_BTN_L (1ULL << 6)
#define OOT_BTN_R (1ULL << 7)

#define OOT_DETAIL_INTS    52
#define OOT_DETAIL_FLOATS  412
#define OOT_DETAIL_TXT_CAP 1024

typedef struct OotCompanionCtx {
    CompanionState state;
    OotCompanionSnapshot snap;
    uint64_t last_mem_ns;
    uint64_t state_ns;
    uint32_t pause_hold;
    int32_t user_page;
    int have_snap;
    int enabled_last;
    NxCompanionTrail trail;
    NxCompanionSeq seq;
    uint64_t seq_buttons;
} OotCompanionCtx;

static inline void oot_companion_ctx_init(OotCompanionCtx *c) {
    if (!c) return;
    companion_state_clear(&c->state);
    memset(&c->snap, 0, sizeof(c->snap));
    c->last_mem_ns = 0ULL;
    c->state_ns = 0ULL;
    c->pause_hold = 0U;
    c->user_page = -1;
    c->have_snap = 0;
    c->enabled_last = 0;
    nx_trail_init(&c->trail, 1.5f, 3.0f);
    nx_seq_init(&c->seq);
    c->seq_buttons = 0ULL;
}

static inline int oot_companion_is_title(const OotCompanionSnapshot *s) {
    if (!s) return 1;
    if (s->max_life == 0U || s->scene[0] == '\0') return 1;
    return strcmp(s->scene, "Title") == 0;
}

static inline int oot_companion_ctx_gate_ok(const OotCompanionCtx *c) {
    if (!c || !c->enabled_last || !c->have_snap) return 0;
    if (c->state.status != COMPANION_STATE_READY || c->state.menu_open != 0) return 0;
    if (c->snap.playing_ocarina == 0U) return 0;
    return oot_companion_write_safe(&c->snap);
}

static inline void oot_companion_eval_snap(
    int enabled, const char *main_build, int have_snap, const OotCompanionSnapshot *snap,
    uint32_t pause_hold, int32_t user_page, CompanionState *out) {
    if (!out) return;
    companion_state_clear(out);
    if (!enabled) return;
    if (!main_build || main_build[0] == '\0') { out->status = COMPANION_STATE_STARTING; return; }
    if (strcmp(main_build, OOT_MAIN_BUILD) != 0) { out->status = COMPANION_STATE_UNSUPPORTED; return; }
    if (!have_snap || !snap || !snap->valid) { out->status = COMPANION_STATE_STARTING; return; }
    if (snap->is_loading || snap->in_transition) { out->status = COMPANION_STATE_LOADING; return; }
    if (oot_companion_is_title(snap)) { out->status = COMPANION_STATE_TITLE; return; }
    out->status = COMPANION_STATE_READY;
    out->in_game = 1;
    out->menu_open = (pause_hold > 0U) ? 1 : 0;
    out->tablet_owned = 1;
    out->life = (int32_t)snap->life;
    out->max_life = (int32_t)snap->max_life;
    out->page = snap->playing_ocarina ? COMPANION_PAGE_OCARINA : user_page;
}

static inline void oot_companion_eval(
    int enabled, const char *main_build, const OotTapMem *mem, uint64_t main_addr,
    uint32_t pause_hold, int32_t user_page, CompanionState *out) {
    if (!out) return;
    if (!enabled) { companion_state_clear(out); return; }
    if (!main_build || main_build[0] == '\0') {
        companion_state_clear(out);
        out->status = COMPANION_STATE_STARTING;
        return;
    }
    if (strcmp(main_build, OOT_MAIN_BUILD) != 0) {
        companion_state_clear(out);
        out->status = COMPANION_STATE_UNSUPPORTED;
        return;
    }
    if (main_addr < 0x10000ULL || !mem || !mem->read) {
        companion_state_clear(out);
        out->status = COMPANION_STATE_STARTING;
        return;
    }
    OotCompanionSnapshot snap;
    int ok = oot_companion_sample(mem, main_addr, &snap);
    oot_companion_eval_snap(enabled, main_build, ok, &snap, pause_hold, user_page, out);
}

static inline void oot_companion_ctx_note_canvas(OotCompanionCtx *c, uint64_t vtable_rel) {
    if (c && vtable_rel == OOT_SCREEN_VT_PAUSE) c->pause_hold = OOT_PAUSE_HOLD_FRAMES;
}

static inline void oot_companion_ctx_reset_seq(OotCompanionCtx *c) {
    if (!c) return;
    nx_seq_abort(&c->seq);
    c->seq_buttons = 0ULL;
}

static inline void oot_companion_ctx_present(
    OotCompanionCtx *c, int enabled, const char *main_build, const OotTapMem *mem,
    uint64_t main_addr, uint64_t now_ns) {
    if (!c) return;
    c->state_ns = now_ns;
    c->enabled_last = enabled ? 1 : 0;
    if (!enabled) {
        companion_state_clear(&c->state);
        c->have_snap = 0;
        c->pause_hold = 0U;
        oot_companion_ctx_reset_seq(c);
        return;
    }
    if (!main_build || main_build[0] == '\0' || strcmp(main_build, OOT_MAIN_BUILD) != 0) {
        companion_state_clear(&c->state);
        c->state.status = (!main_build || !main_build[0]) ? COMPANION_STATE_STARTING : COMPANION_STATE_UNSUPPORTED;
        c->have_snap = 0;
        c->pause_hold = 0U;
        oot_companion_ctx_reset_seq(c);
        return;
    }
    if (main_addr < 0x10000ULL || !mem || !mem->read) {
        companion_state_clear(&c->state);
        c->state.status = COMPANION_STATE_STARTING;
        c->have_snap = 0;
        if (c->pause_hold > 0U) c->pause_hold--;
        oot_companion_ctx_reset_seq(c);
        return;
    }
    if (!c->have_snap || c->last_mem_ns == 0ULL || now_ns < c->last_mem_ns ||
        (now_ns - c->last_mem_ns) >= OOT_COMPANION_SAMPLE_INTERVAL_NS) {
        c->last_mem_ns = now_ns;
        c->have_snap = oot_companion_sample(mem, main_addr, &c->snap) ? 1 : 0;
        if (c->have_snap && !oot_companion_is_title(&c->snap) &&
            c->snap.pose_valid && !c->snap.is_loading) {
            (void)nx_trail_push(&c->trail, c->snap.scene,
                                c->snap.pos[0], c->snap.pos[1], c->snap.pos[2]);
        }
    }
    oot_companion_eval_snap(enabled, main_build, c->have_snap, &c->snap, c->pause_hold, c->user_page, &c->state);
    if (c->pause_hold > 0U) c->pause_hold--;
    c->seq_buttons = nx_seq_tick(&c->seq, oot_companion_ctx_gate_ok(c), now_ns);
}

static inline int oot_companion_ctx_play_song(
    OotCompanionCtx *c, int song_idx, int classic_mode, uint64_t now_ns) {
    if (!oot_companion_ctx_gate_ok(c) || song_idx < 0 || song_idx >= 12) return 0;
    if ((c->snap.songs_mask & (1U << (uint32_t)song_idx)) == 0U) return 0;
    const OotSongSpec *spec = &OOT_SW2_SONGS[song_idx];
    uint64_t chords[8] = {0};
    for (uint8_t i = 0U; i < spec->length && i < 8U; i++) {
        chords[i] = oot_sw2_note_button(spec->notes[i], classic_mode,
                                        OOT_BTN_A, OOT_BTN_B, OOT_BTN_X,
                                        OOT_BTN_Y, OOT_BTN_L, OOT_BTN_R);
    }
    if (!nx_seq_start(&c->seq, chords, spec->length, song_idx,
                      OOT_NOTE_HOLD_NS, OOT_NOTE_GAP_NS, now_ns)) {
        return 0;
    }
    c->seq_buttons = nx_seq_tick(&c->seq, 1, now_ns);
    return 1;
}

static inline int oot_companion_ctx_tap_note(
    OotCompanionCtx *c, int note_idx, int classic_mode, uint64_t now_ns) {
    if (!oot_companion_ctx_gate_ok(c) || note_idx < 0 || note_idx >= OOT_NOTE_COUNT) return 0;
    uint64_t btn = oot_sw2_note_button((uint32_t)note_idx, classic_mode,
                                       OOT_BTN_A, OOT_BTN_B, OOT_BTN_X,
                                       OOT_BTN_Y, OOT_BTN_L, OOT_BTN_R);
    if (btn == 0ULL) return 0;
    if (!nx_seq_start(&c->seq, &btn, 1U, 100 + note_idx, 120000000ULL, 0ULL, now_ns)) return 0;
    c->seq_buttons = nx_seq_tick(&c->seq, 1, now_ns);
    return 1;
}

static inline int oot_companion_ctx_pin(
    OotCompanionCtx *c, int op, int slot, const char *label) {
    if (!c) return -1;
    if (op == 0) {
        if (!c->have_snap || !c->snap.pose_valid || oot_companion_is_title(&c->snap)) return -1;
        return nx_trail_pin_add(&c->trail, c->snap.pos[0], c->snap.pos[1], c->snap.pos[2], label);
    }
    if (op == 1 && slot >= 0) return nx_trail_pin_remove(&c->trail, (uint32_t)slot) ? slot : -1;
    if (op == 2) { nx_trail_pins_clear(&c->trail); return 0; }
    return -1;
}

static inline void oot_companion_pack_ints(
    const OotCompanionCtx *c, int32_t *ints, int max_ints, int32_t dungeon_idx) {
    if (!c || !ints || max_ints < OOT_DETAIL_INTS) return;
    memset(ints, 0, (size_t)max_ints * sizeof(*ints));
    ints[0] = c->state.status;
    ints[1] = oot_companion_ctx_gate_ok(c);
    ints[2] = oot_companion_write_safe(&c->snap);
    ints[3] = (int32_t)c->snap.is_child;
    ints[4] = (int32_t)c->snap.playing_ocarina;
    ints[5] = (int32_t)c->snap.riding_horse;
    ints[6] = c->state.menu_open;
    ints[7] = (int32_t)c->snap.dialogue_open;
    ints[8] = (int32_t)c->snap.pose_valid;
    ints[9] = (int32_t)c->snap.life;
    ints[10] = (int32_t)c->snap.max_life;
    ints[11] = (int32_t)c->snap.magic;
    ints[12] = (int32_t)c->snap.max_magic;
    ints[13] = (int32_t)c->snap.magic_unlocked;
    ints[14] = (int32_t)c->snap.rupees;
    ints[15] = (int32_t)c->snap.wallet_cap;
    ints[16] = (int32_t)c->snap.tokens;
    ints[17] = (int32_t)c->snap.beans;
    ints[18] = (int32_t)c->snap.stones_mask;
    ints[19] = (int32_t)c->snap.medallions_mask;
    ints[20] = (int32_t)c->snap.songs_mask;
    for (int i = 0; i < (int)OOT_AMMO_COUNT; i++) {
        ints[21 + i] = (int32_t)c->snap.ammo[i];
        ints[27 + i] = (int32_t)c->snap.ammo_cap[i];
    }
    for (int i = 0; i < (int)OOT_KEY_DUNGEON_N; i++) ints[33 + i] = (int32_t)c->snap.small_keys[i];
    for (int i = 0; i < (int)OOT_BOTTLE_SLOTS; i++) ints[41 + i] = (int32_t)c->snap.bottles[i];
    ints[46] = (int32_t)c->trail.count;
    ints[47] = c->seq.active ? c->seq.tag : -1;
    ints[48] = c->seq.active ? (int32_t)c->seq.idx : -1;
    int32_t pin_mask = 0;
    for (uint32_t i = 0U; i < NX_WAYPOINT_CAP; i++) {
        if (c->trail.pins[i].active) pin_mask |= (1 << (int)i);
    }
    ints[49] = pin_mask;
    ints[50] = dungeon_idx;
    ints[51] = c->state.page;
}

static inline void oot_companion_pack_floats(
    const OotCompanionCtx *c, float *flts, int max_flts) {
    if (!c || !flts || max_flts < OOT_DETAIL_FLOATS) return;
    memset(flts, 0, (size_t)max_flts * sizeof(*flts));
    flts[0] = c->snap.pos[0];
    flts[1] = c->snap.pos[1];
    flts[2] = c->snap.pos[2];
    flts[3] = c->snap.yaw_deg;
    for (uint32_t i = 0U; i < c->trail.count && i < NX_TRAIL_CAP; i++) {
        NxTrailPoint pt;
        if (nx_trail_get_oldest(&c->trail, i, &pt)) {
            flts[4U + i * 3U + 0U] = pt.x;
            flts[4U + i * 3U + 1U] = pt.y;
            flts[4U + i * 3U + 2U] = pt.z;
        }
    }
    for (uint32_t i = 0U; i < NX_WAYPOINT_CAP; i++) {
        if (c->trail.pins[i].active) {
            flts[388U + i * 3U + 0U] = c->trail.pins[i].x;
            flts[388U + i * 3U + 1U] = c->trail.pins[i].y;
            flts[388U + i * 3U + 2U] = c->trail.pins[i].z;
        }
    }
}

static inline size_t oot_companion_append_line(
    char *dst, size_t cap, size_t off, const char *src) {
    if (!dst || cap == 0U || off >= cap) return off;
    if (src) {
        for (size_t i = 0U; src[i] != '\0' && off + 2U < cap && i < 128U; i++) {
            char ch = (src[i] == '\n' || src[i] == '\r') ? ' ' : src[i];
            dst[off++] = ch;
        }
    }
    if (off + 1U < cap) dst[off++] = '\n';
    dst[off] = '\0';
    return off;
}

static inline void oot_companion_ctx_pack_detail(
    const OotCompanionCtx *c, int32_t *ints, int max_ints,
    float *flts, int max_flts, char *txt, int max_txt) {
    if (!c) return;
    int32_t d_idx = -1;
    const char *scene_disp = oot_sw2_resolve_scene(c->snap.scene, &d_idx);
    oot_companion_pack_ints(c, ints, max_ints, d_idx);
    oot_companion_pack_floats(c, flts, max_flts);
    if (!txt || max_txt <= 0) return;
    size_t cap = (size_t)max_txt;
    size_t off = 0U;
    txt[0] = '\0';
    off = oot_companion_append_line(txt, cap, off, scene_disp);
    off = oot_companion_append_line(txt, cap, off, c->snap.radar_loc[0] ? c->snap.radar_loc : c->snap.scene);
    off = oot_companion_append_line(txt, cap, off, c->snap.equip_weapon);
    off = oot_companion_append_line(txt, cap, off, c->snap.equip_shield);
    off = oot_companion_append_line(txt, cap, off, c->snap.equip_armor);
    off = oot_companion_append_line(txt, cap, off, c->snap.equip_boot);
    off = oot_companion_append_line(txt, cap, off, c->snap.c_slots[2]);
    off = oot_companion_append_line(txt, cap, off, c->snap.c_slots[3]);
    for (uint32_t i = 0U; i < NX_WAYPOINT_CAP; i++) {
        off = oot_companion_append_line(txt, cap, off, c->trail.pins[i].active ? c->trail.pins[i].label : "");
    }
}

void oot_companion_note_canvas(uint64_t vtable_rel);
void dm_companion_present(uint64_t frame);

#ifdef __cplusplus
}
#endif
