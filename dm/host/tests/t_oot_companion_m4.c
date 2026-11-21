// SPDX-License-Identifier: MIT
// Host unit tests for M4 Ocarina sequencer, 3D breadcrumb trail, waypoint pins,
// and detailed companion packing in dm/host/android/oot_companion.h.
#include "../android/oot_companion.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define SIM_BASE 0x7000000000ULL
#define SIM_SIZE (80ULL * 1024ULL * 1024ULL)

enum {
    VA_GDM = 0x04900000ULL, VA_BOOL_ENT = 0x04902000ULL, VA_BOOL_TBL = 0x04903000ULL,
    VA_INT_ENT = 0x04904000ULL, VA_INT_TBL = 0x04905000ULL, VA_FLT_ENT = 0x04906000ULL,
    VA_FLT_TBL = 0x04907000ULL, VA_STR_ENT = 0x04908000ULL, VA_STR_TBL = 0x0490A000ULL,
    VA_STR_BUF = 0x0490C000ULL, VA_UI = 0x04910000ULL, VA_BANC = 0x04911000ULL,
    VA_EVENT = 0x04912000ULL, VA_SCENE = 0x04913000ULL, VA_STBL = 0x04914000ULL,
    VA_SARR = 0x04915000ULL, VA_PMGR = 0x04916000ULL, VA_LINK = 0x04917000ULL,
    VA_HOLD = 0x04918000ULL, VA_ACT = 0x04919000ULL, VA_COMP = 0x0491A000ULL,
    VA_ASC = 0x0491B000ULL, VA_ASI = 0x0491C000ULL, VA_CTRL = 0x0491D000ULL,
    VA_SAR = 0x0491E000ULL, VA_S0 = 0x0491F000ULL, VA_CMD = 0x04920000ULL,
    VA_MSG = 0x04922000ULL, VA_LNK_STR = 0x04923000ULL
};

static uint8_t g_buf[SIM_SIZE];
static uint32_t g_bool_n, g_int_n, g_flt_n, g_str_n, g_str_cursor;

static int sim_read(void *ctx, uint64_t addr, void *out, size_t len) {
    (void)ctx;
    if (addr < SIM_BASE || addr + len > SIM_BASE + SIM_SIZE || addr + len < addr) return 0;
    memcpy(out, g_buf + (size_t)(addr - SIM_BASE), len);
    return 1;
}

static void w8(uint64_t off, uint8_t v) { g_buf[off] = v; }
static void w16(uint64_t off, uint16_t v) { memcpy(g_buf + off, &v, 2); }
static void w32(uint64_t off, uint32_t v) { memcpy(g_buf + off, &v, 4); }
static void w64(uint64_t off, uint64_t v) { memcpy(g_buf + off, &v, 8); }
static void wf32(uint64_t off, float v) { memcpy(g_buf + off, &v, 4); }
static void wstr(uint64_t off, const char *s) { memcpy(g_buf + off, s, strlen(s) + 1U); }

static void add_slot(uint64_t tbl, uint32_t slots, uint32_t hash, uint32_t idx) {
    uint32_t s = hash % slots;
    for (uint32_t p = 0U; p < slots; p++) {
        uint32_t cur = 0U;
        memcpy(&cur, g_buf + tbl + (size_t)s * 8U, 4);
        if (cur == 0U) {
            w32(tbl + (uint64_t)s * 8ULL, hash);
            w32(tbl + (uint64_t)s * 8ULL + 4ULL, idx);
            return;
        }
        s = (s + 1U < slots) ? (s + 1U) : 0U;
    }
    assert(0);
}

static void put_bool(uint32_t hash, uint8_t v) {
    uint32_t idx = g_bool_n++;
    w8(VA_BOOL_ENT + (uint64_t)idx * OOT_GDM_BOOL_STRIDE + OOT_GDM_BOOL_VAL, v);
    add_slot(VA_BOOL_TBL, 64U, hash, idx);
}

static void put_int(uint32_t hash, uint32_t v) {
    uint32_t idx = g_int_n++;
    w32(VA_INT_ENT + (uint64_t)idx * OOT_GDM_INT_STRIDE + OOT_GDM_NUM_VAL, v);
    add_slot(VA_INT_TBL, 128U, hash, idx);
}

static void put_flt(uint32_t hash, float v) {
    uint32_t idx = g_flt_n++;
    wf32(VA_FLT_ENT + (uint64_t)idx * OOT_GDM_FLOAT_STRIDE + OOT_GDM_NUM_VAL, v);
    add_slot(VA_FLT_TBL, 16U, hash, idx);
}

static void put_str(uint32_t hash, const char *v) {
    uint32_t idx = g_str_n++;
    uint64_t off = VA_STR_BUF + g_str_cursor;
    g_str_cursor += 64U;
    wstr(off, v);
    w64(VA_STR_ENT + (uint64_t)idx * OOT_GDM_STR64_STRIDE + OOT_GDM_STR64_PTR, SIM_BASE + off);
    add_slot(VA_STR_TBL, 32U, hash, idx);
}

static void init_box(uint32_t kind_off, uint32_t count, uint64_t ent, uint64_t tbl, uint32_t slots) {
    uint64_t b = VA_GDM + kind_off;
    w32(b + 8U, count);
    w64(b + 16U, SIM_BASE + ent);
    w64(b + 24U, SIM_BASE + tbl);
    w32(b + 36U, slots);
}

static void setup_world(const char *scene, float x, float y, float z, const char *cmd) {
    memset(g_buf, 0, sizeof(g_buf));
    g_bool_n = g_int_n = g_flt_n = g_str_n = g_str_cursor = 0U;
    w64(OOT_OFF_GAMEDATA_MGR_PTR, SIM_BASE + VA_GDM);
    w64(OOT_OFF_GAMEUI_MGR_PTR, SIM_BASE + VA_UI);
    w64(OOT_OFF_EVENT_MGR_PTR, SIM_BASE + VA_EVENT);
    w64(OOT_OFF_SCENE_MGR_PTR, SIM_BASE + VA_SCENE);
    w64(OOT_OFF_MSG_HOLDER_PTR, SIM_BASE + VA_MSG);

    put_bool(OOT_KEY_LOADING_BANC, 0U);
    put_bool(OOT_KEY_IS_CHILDHOOD, 0U);
    put_bool(OOT_KEY_MP_UNLOCKED, 1U);
    put_bool(0x01C20002U, 1U); // Zelda's Lullaby learned
    put_bool(0x2B39DD4FU, 1U); // Song of Storms learned
    put_int(OOT_KEY_LIFE, 40U);
    put_int(OOT_KEY_MAX_LIFE, 48U);
    put_int(OOT_KEY_MP, 120U);
    put_int(OOT_KEY_MP_MAX, 240U);
    put_int(OOT_KEY_RUPEES, 320U);
    put_int(OOT_KEY_WALLET_CAP, 500U);
    put_int(OOT_KEY_GOLD_TOKENS, 23U);
    put_int(0x877BDCEEU, 5U);          // Water Temple Small Keys
    put_int(0x7D48DF76U, 0x6F49E357U); // Bottle 0 -> Fairy
    put_int(0xCBD1E58CU, 1U);
    put_int(0x692D57CBU, 1U);
    put_flt(OOT_KEY_ROT_FOR_MAP, 90.0f);
    put_str(OOT_KEY_CURRENT_BANC, scene);
    put_str(OOT_KEY_RADAR_LOCATION, "Central Pillar");
    put_str(OOT_KEY_EQUIP_WEAPON, "Weapon_Sword_Master");
    put_str(OOT_KEY_EQUIP_ARMOR, "Armor_Zora");

    init_box(OOT_GDM_BOOL_OFF, g_bool_n, VA_BOOL_ENT, VA_BOOL_TBL, 64U);
    init_box(OOT_GDM_INT_OFF, g_int_n, VA_INT_ENT, VA_INT_TBL, 128U);
    init_box(OOT_GDM_FLOAT_OFF, g_flt_n, VA_FLT_ENT, VA_FLT_TBL, 16U);
    init_box(OOT_GDM_STR64_OFF, g_str_n, VA_STR_ENT, VA_STR_TBL, 32U);

    w16(VA_UI + OOT_UI_POSE_FLAGS, 1U);
    wf32(VA_UI + OOT_UI_POSE_POS, x);
    wf32(VA_UI + OOT_UI_POSE_POS + 4U, y);
    wf32(VA_UI + OOT_UI_POSE_POS + 8U, z);

    w64(VA_SCENE + OOT_SCENE_TABLE, SIM_BASE + VA_STBL);
    w32(VA_STBL + OOT_TABLE_COUNT, 16U);
    w64(VA_STBL + OOT_TABLE_ARRAY, SIM_BASE + VA_SARR);
    w64(VA_SARR + OOT_PLAYER_SLOT_OFF, SIM_BASE + VA_PMGR);
    w64(VA_PMGR + OOT_MGR_MUST_SET, 1U);
    w64(VA_PMGR + OOT_MGR_LINK0, SIM_BASE + VA_LINK);
    w64(VA_LINK + OOT_LINK_HOLDER, SIM_BASE + VA_HOLD);
    w32(VA_LINK + OOT_LINK_ID, 77U);
    w64(VA_HOLD + OOT_HOLDER_ACTOR, SIM_BASE + VA_ACT);
    w32(VA_HOLD + OOT_HOLDER_ID, 77U);
    w32(VA_ACT + OOT_ACTOR_STATE, 4U);
    w64(VA_ACT + OOT_ACTOR_COMPS, SIM_BASE + VA_COMP);
    w64(VA_COMP + OOT_COMP_AS, SIM_BASE + VA_ASC);
    w64(VA_ASC + OOT_AS_INNER, SIM_BASE + VA_ASI);
    w64(VA_ASI + OOT_AS_CTRL, SIM_BASE + VA_CTRL);
    w32(VA_CTRL + OOT_AS_SLOT_COUNT, 1U);
    w64(VA_CTRL + OOT_AS_SLOT_ARRAY, SIM_BASE + VA_SAR);
    w64(VA_SAR, SIM_BASE + VA_S0);
    w64(VA_S0 + OOT_AS_SLOT0_CMD, SIM_BASE + VA_CMD);
    wstr(VA_CMD, cmd);
    w32(VA_MSG + OOT_MSG_ID, 0xFFFFFFFFU);
}

static void test_trail_waypoints_and_pack(void) {
    OotTapMem mem = { sim_read, NULL };
    OotCompanionCtx ctx;
    oot_companion_ctx_init(&ctx);
    ctx.user_page = 0;

    setup_world("MIZUsin", 10.0f, 5.0f, -20.0f, "Walk");
    uint64_t t = 1000000000ULL;
    oot_companion_ctx_present(&ctx, 1, OOT_MAIN_BUILD, &mem, SIM_BASE, t);
    assert(ctx.trail.count == 1U);

    assert(oot_companion_ctx_pin(&ctx, 0, -1, "Switch") == 0);
    wf32(VA_UI + OOT_UI_POSE_POS, 16.0f);
    t += OOT_COMPANION_SAMPLE_INTERVAL_NS;
    oot_companion_ctx_present(&ctx, 1, OOT_MAIN_BUILD, &mem, SIM_BASE, t);
    assert(ctx.trail.count == 2U);

    int32_t ints[OOT_DETAIL_INTS];
    float flts[OOT_DETAIL_FLOATS];
    char txt[OOT_DETAIL_TXT_CAP];
    oot_companion_ctx_pack_detail(&ctx, ints, OOT_DETAIL_INTS, flts, OOT_DETAIL_FLOATS, txt, sizeof(txt));
    assert(ints[0] == COMPANION_STATE_READY && ints[1] == 0 && ints[2] == 1);
    assert(ints[14] == 320 && ints[15] == 500 && ints[16] == 23);
    assert(ints[36] == 5 && ints[41] == OOT_BOTTLE_FAIRY);
    assert(ints[46] == 2 && ints[49] == 1 && ints[50] == 3);
    assert(fabsf(flts[0] - 16.0f) < 1e-4f && fabsf(flts[3] - 90.0f) < 1e-4f);
    assert(strstr(txt, "Water Temple\nCentral Pillar\nWeapon_Sword_Master\n") != NULL);
    assert(strstr(txt, "\nSwitch\n") != NULL);
}

static void test_ocarina_sequencer_and_gates(void) {
    OotTapMem mem = { sim_read, NULL };
    OotCompanionCtx ctx;
    oot_companion_ctx_init(&ctx);

    setup_world("Spot04", 0.0f, 0.0f, 0.0f, "Walk");
    uint64_t t = 2000000000ULL;
    oot_companion_ctx_present(&ctx, 1, OOT_MAIN_BUILD, &mem, SIM_BASE, t);

    // Refuses Ocarina playback when not holding Ocarina or when unlearned.
    assert(oot_companion_ctx_play_song(&ctx, 0, 0, t) == 0);
    assert(oot_companion_ctx_tap_note(&ctx, OOT_NOTE_A4, 0, t) == 0);

    wstr(VA_CMD, "DemoOcarina");
    t += OOT_COMPANION_SAMPLE_INTERVAL_NS;
    oot_companion_ctx_present(&ctx, 1, OOT_MAIN_BUILD, &mem, SIM_BASE, t);
    assert(oot_companion_ctx_play_song(&ctx, 1, 0, t) == 0); // Epona unlearned

    // Zelda's Lullaby (index 0, learned): Y, X, A, Y, X, A in Normal mode.
    assert(oot_companion_ctx_play_song(&ctx, 0, 0, t) == 1);
    assert(ctx.seq_buttons == OOT_BTN_Y && ctx.seq.idx == 0U);
    t += OOT_NOTE_HOLD_NS;
    oot_companion_ctx_present(&ctx, 1, OOT_MAIN_BUILD, &mem, SIM_BASE, t);
    assert(ctx.seq_buttons == 0ULL && ctx.seq.in_gap == 1U);
    t += OOT_NOTE_GAP_NS;
    oot_companion_ctx_present(&ctx, 1, OOT_MAIN_BUILD, &mem, SIM_BASE, t);
    assert(ctx.seq_buttons == OOT_BTN_X && ctx.seq.idx == 1U);

    // Dialogue opening aborts playback immediately on next sample!
    w32(VA_MSG + OOT_MSG_ID, 10U);
    t += OOT_COMPANION_SAMPLE_INTERVAL_NS;
    oot_companion_ctx_present(&ctx, 1, OOT_MAIN_BUILD, &mem, SIM_BASE, t);
    assert(ctx.seq.active == 0 && ctx.seq_buttons == 0ULL);

    // Clear dialogue and test Classic single-note tap (F4 -> B).
    w32(VA_MSG + OOT_MSG_ID, 0xFFFFFFFFU);
    t += OOT_COMPANION_SAMPLE_INTERVAL_NS;
    oot_companion_ctx_present(&ctx, 1, OOT_MAIN_BUILD, &mem, SIM_BASE, t);
    assert(oot_companion_ctx_tap_note(&ctx, OOT_NOTE_F4, 1, t) == 1);
    assert(ctx.seq_buttons == OOT_BTN_B);
    t += 120000000ULL;
    oot_companion_ctx_present(&ctx, 1, OOT_MAIN_BUILD, &mem, SIM_BASE, t);
    assert(ctx.seq.active == 0 && ctx.seq_buttons == 0ULL);
}

int main(void) {
    test_trail_waypoints_and_pack();
    test_ocarina_sequencer_and_gates();
    printf("t_oot_companion_m4: PASS\n");
    return 0;
}
