// SPDX-License-Identifier: GPL-3.0-or-later
// Host unit tests for dm/host/android/oot_companion_tap.h using a synthetic
// guest address space (no game binary required).

#include "../android/oot_companion_tap.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define SIM_BASE 0x7000000000ULL
#define SIM_SIZE (80ULL * 1024ULL * 1024ULL)

typedef struct {
    uint8_t buf[SIM_SIZE];
} SimSpace;

static SimSpace g_sim;

static int sim_read(void *ctx, uint64_t addr, void *dst, size_t len) {
    const SimSpace *sim = (const SimSpace *)ctx;
    if (!sim || addr < SIM_BASE || addr + len > SIM_BASE + SIM_SIZE || addr + len < addr) return 0;
    memcpy(dst, sim->buf + (size_t)(addr - SIM_BASE), len);
    return 1;
}

static void sim_w8(uint64_t off, uint8_t v) { g_sim.buf[off] = v; }
static void sim_w16(uint64_t off, uint16_t v) { memcpy(g_sim.buf + off, &v, 2); }
static void sim_w32(uint64_t off, uint32_t v) { memcpy(g_sim.buf + off, &v, 4); }
static void sim_w64(uint64_t off, uint64_t v) { memcpy(g_sim.buf + off, &v, 8); }
static void sim_wf32(uint64_t off, float v) { memcpy(g_sim.buf + off, &v, 4); }
static void sim_wstr(uint64_t off, const char *s) { memcpy(g_sim.buf + off, s, strlen(s) + 1U); }

enum {
    VA_GDM = 0x04900000ULL,
    VA_BOOL_ENT = 0x04902000ULL,
    VA_BOOL_TBL = 0x04903000ULL,
    VA_INT_ENT = 0x04904000ULL,
    VA_INT_TBL = 0x04905000ULL,
    VA_FLT_ENT = 0x04906000ULL,
    VA_FLT_TBL = 0x04907000ULL,
    VA_STR_ENT = 0x04908000ULL,
    VA_STR_TBL = 0x0490A000ULL,
    VA_STR_BUF = 0x0490C000ULL,
    VA_UI = 0x04910000ULL,
    VA_BANC = 0x04911000ULL,
    VA_EVENT = 0x04912000ULL,
    VA_SCENE = 0x04913000ULL,
    VA_STBL = 0x04914000ULL,
    VA_SARR = 0x04915000ULL,
    VA_PMGR = 0x04916000ULL,
    VA_LINK = 0x04917000ULL,
    VA_HOLD = 0x04918000ULL,
    VA_ACT = 0x04919000ULL,
    VA_COMP = 0x0491A000ULL,
    VA_ASC = 0x0491B000ULL,
    VA_ASI = 0x0491C000ULL,
    VA_CTRL = 0x0491D000ULL,
    VA_SAR = 0x0491E000ULL,
    VA_S0 = 0x0491F000ULL,
    VA_CMD = 0x04920000ULL,
    VA_PERF = 0x04921000ULL,
    VA_MSG = 0x04922000ULL
};

static uint32_t g_bool_n, g_int_n, g_flt_n, g_str_n;
static uint32_t g_str_cursor = 0U;

static void add_gdm_slot(uint64_t tbl, uint32_t slots, uint32_t hash, uint32_t idx) {
    uint32_t s = hash % slots;
    for (uint32_t p = 0U; p < slots; p++) {
        uint32_t cur = 0U;
        memcpy(&cur, g_sim.buf + tbl + (size_t)s * 8U, 4);
        if (cur == 0U) {
            sim_w32(tbl + (uint64_t)s * 8ULL, hash);
            sim_w32(tbl + (uint64_t)s * 8ULL + 4ULL, idx);
            return;
        }
        s = (s + 1U < slots) ? (s + 1U) : 0U;
    }
    assert(0 && "hash table full");
}

static void put_bool(uint32_t hash, uint8_t val) {
    uint32_t idx = g_bool_n++;
    sim_w8(VA_BOOL_ENT + (uint64_t)idx * OOT_GDM_BOOL_STRIDE + OOT_GDM_BOOL_VAL, val);
    add_gdm_slot(VA_BOOL_TBL, 64U, hash, idx);
}

static void put_int(uint32_t hash, uint32_t val) {
    uint32_t idx = g_int_n++;
    sim_w32(VA_INT_ENT + (uint64_t)idx * OOT_GDM_INT_STRIDE + OOT_GDM_NUM_VAL, val);
    add_gdm_slot(VA_INT_TBL, 64U, hash, idx);
}

static void put_flt(uint32_t hash, float val) {
    uint32_t idx = g_flt_n++;
    sim_wf32(VA_FLT_ENT + (uint64_t)idx * OOT_GDM_FLOAT_STRIDE + OOT_GDM_NUM_VAL, val);
    add_gdm_slot(VA_FLT_TBL, 16U, hash, idx);
}

static void put_str(uint32_t hash, const char *val) {
    uint32_t idx = g_str_n++;
    uint64_t buf_off = VA_STR_BUF + g_str_cursor;
    g_str_cursor += 64U;
    sim_wstr(buf_off, val);
    sim_w64(VA_STR_ENT + (uint64_t)idx * OOT_GDM_STR64_STRIDE + OOT_GDM_STR64_PTR, SIM_BASE + buf_off);
    add_gdm_slot(VA_STR_TBL, 32U, hash, idx);
}

static void init_box(uint32_t kind_off, uint32_t count, uint64_t ent, uint64_t tbl, uint32_t slots) {
    uint64_t b = VA_GDM + kind_off;
    sim_w32(b + 8U, count);
    sim_w64(b + 16U, SIM_BASE + ent);
    sim_w64(b + 24U, SIM_BASE + tbl);
    sim_w32(b + 36U, slots);
}

static void setup_synthetic_world(void) {
    memset(&g_sim, 0, sizeof(g_sim));
    g_bool_n = g_int_n = g_flt_n = g_str_n = g_str_cursor = 0U;
    sim_w64(OOT_OFF_GAMEDATA_MGR_PTR, SIM_BASE + VA_GDM);
    sim_w64(OOT_OFF_GAMEUI_MGR_PTR, SIM_BASE + VA_UI);
    sim_w64(OOT_OFF_BANC_MGR_PTR, SIM_BASE + VA_BANC);
    sim_w64(OOT_OFF_EVENT_MGR_PTR, SIM_BASE + VA_EVENT);
    sim_w64(OOT_OFF_SCENE_MGR_PTR, SIM_BASE + VA_SCENE);
    sim_w64(OOT_OFF_MSG_HOLDER_PTR, SIM_BASE + VA_MSG);

    put_bool(OOT_KEY_IS_CHILDHOOD, 1U);
    put_bool(OOT_KEY_LOADING_BANC, 0U);
    put_bool(OOT_KEY_MP_UNLOCKED, 1U);
    put_bool(0x01C20002U, 1U);  // Zelda's Lullaby
    put_bool(0xD3F8BFCCU, 1U);  // Saria's Song
    put_bool(0x2B39DD4FU, 1U);  // Song of Storms
    put_int(OOT_KEY_LIFE, 24U);
    put_int(OOT_KEY_MAX_LIFE, 36U);
    put_int(OOT_KEY_MP, 120U);
    put_int(OOT_KEY_MP_MAX, 240U);
    put_int(OOT_KEY_RUPEES, 195U);
    put_int(OOT_KEY_GOLD_TOKENS, 14U);
    put_int(OOT_KEY_WALLET_CAP, 500U);
    put_int(OOT_KEY_ARROWS, 22U);
    put_int(OOT_KEY_ARROWS_CAP, 40U);
    put_int(0x877BDCEEU, 4U);          // Water Temple Small Keys
    put_int(0x7D48DF76U, 0x1C293517U); // Bottle 0 -> Blue Fire
    put_int(0xCBD1E58CU, 1U);  // Kokiri Emerald
    put_int(0x7D69AD47U, 1U);  // Goron Ruby
    put_int(0x692D57CBU, 1U);  // Forest Medallion
    put_flt(OOT_KEY_ROT_FOR_MAP, 135.5f);
    put_str(OOT_KEY_CURRENT_BANC, "KokiriForest_Child");
    put_str(OOT_KEY_RADAR_LOCATION, "Kokiri Shop");
    put_str(OOT_KEY_ACTION_PROMPT, "Talk");
    put_str(OOT_KEY_EQUIP_WEAPON, "Weapon_Sword_Kokiri");
    put_str(OOT_KEY_EQUIP_SHIELD, "Weapon_Shield_Deku");
    put_str(OOT_KEY_EQUIP_OCARINA, "Item_Ocarina_Fairy");
    init_box(OOT_GDM_BOOL_OFF, g_bool_n, VA_BOOL_ENT, VA_BOOL_TBL, 64U);
    init_box(OOT_GDM_INT_OFF, g_int_n, VA_INT_ENT, VA_INT_TBL, 64U);
    init_box(OOT_GDM_FLOAT_OFF, g_flt_n, VA_FLT_ENT, VA_FLT_TBL, 16U);
    init_box(OOT_GDM_STR64_OFF, g_str_n, VA_STR_ENT, VA_STR_TBL, 32U);

    sim_w16(VA_UI + OOT_UI_POSE_FLAGS, 1U);
    sim_wf32(VA_UI + OOT_UI_POSE_POS, -12.5f);
    sim_wf32(VA_UI + OOT_UI_POSE_POS + 4U, 3.0f);
    sim_wf32(VA_UI + OOT_UI_POSE_POS + 8U, 48.25f);
    sim_w8(VA_UI + OOT_UI_IS_CHILD, 1U);

    sim_w64(VA_SCENE + OOT_SCENE_TABLE, SIM_BASE + VA_STBL);
    sim_w32(VA_STBL + OOT_TABLE_COUNT, 16U);
    sim_w64(VA_STBL + OOT_TABLE_ARRAY, SIM_BASE + VA_SARR);
    sim_w64(VA_SARR + OOT_PLAYER_SLOT_OFF, SIM_BASE + VA_PMGR);
    sim_w64(VA_PMGR + OOT_MGR_MUST_SET, 1U);
    sim_w64(VA_PMGR + OOT_MGR_LINK0, SIM_BASE + VA_LINK);
    sim_w64(VA_LINK + OOT_LINK_HOLDER, SIM_BASE + VA_HOLD);
    sim_w32(VA_LINK + OOT_LINK_ID, 77U);
    sim_w64(VA_HOLD + OOT_HOLDER_ACTOR, SIM_BASE + VA_ACT);
    sim_w32(VA_HOLD + OOT_HOLDER_ID, 77U);
    sim_w32(VA_ACT + OOT_ACTOR_STATE, 4U);
    sim_w64(VA_ACT + OOT_ACTOR_COMPS, SIM_BASE + VA_COMP);
    sim_w64(VA_COMP + OOT_COMP_AS, SIM_BASE + VA_ASC);
    sim_w64(VA_ASC + OOT_AS_INNER, SIM_BASE + VA_ASI);
    sim_w64(VA_ASI + OOT_AS_CTRL, SIM_BASE + VA_CTRL);
    sim_w32(VA_CTRL + OOT_AS_SLOT_COUNT, 1U);
    sim_w64(VA_CTRL + OOT_AS_SLOT_ARRAY, SIM_BASE + VA_SAR);
    sim_w64(VA_SAR, SIM_BASE + VA_S0);
    sim_w64(VA_S0 + OOT_AS_SLOT0_CMD, SIM_BASE + VA_CMD);
    sim_wstr(VA_CMD, "Walk");
    sim_w64(VA_COMP + OOT_COMP_PERF0, SIM_BASE + VA_PERF);
    sim_w32(VA_MSG + OOT_MSG_ID, 0xFFFFFFFFU);
}

static void test_murmur_hashes(void) {
    assert(oot_murmur3_32("PlayerStatus.CurrentRupee", 25U) == 0xA77921D7U);
    assert(oot_murmur3_32("PlayerStatus.Life", 17U) == 0x31AB5580U);
    assert(oot_murmur3_32("PlayerStatus.MaxLife", 20U) == 0xFBE01DA1U);
    assert(oot_murmur3_32("PlayerStatus.MP", 15U) == 0x76B18200U);
    assert(oot_murmur3_32("PlayerStatus.EquipWeapon", 24U) == 0xA95A3521U);
    assert(oot_murmur3_32("PlayerStatus.EquipShield", 24U) == 0xEE463235U);
    assert(oot_murmur3_32("Sequence_CurrentBanc", 20U) == 0x1D6189DAU);
    assert(oot_murmur3_32("Sequence_IsChildhood", 20U) == 0xCEA220D3U);
    assert(oot_murmur3_32("PouchData.Item_Token_Gold.StockNum", 34U) == 0x04735978U);
}

static void test_world_modes(void) {
    setup_synthetic_world();
    OotTapMem mem = {sim_read, &g_sim};
    OotCompanionSnapshot s;
    assert(oot_companion_sample(&mem, SIM_BASE, &s) == 1);
    assert(s.valid == 1U && s.is_child == 1U && s.is_loading == 0U && s.in_cutscene == 0U);
    assert(s.life == 24U && s.max_life == 36U && s.magic == 120U && s.max_magic == 240U);
    assert(s.rupees == 195U && s.tokens == 14U && s.magic_unlocked == 1U);
    assert(s.stones_mask == 0x3U && s.medallions_mask == 0x2U);
    assert(s.songs_mask == ((1U << 0U) | (1U << 2U) | (1U << 5U)));
    assert(s.pose_valid == 1U && fabsf(s.pos[0] + 12.5f) < 1e-5f && fabsf(s.yaw_deg - 135.5f) < 1e-4f);
    assert(strcmp(s.scene, "KokiriForest_Child") == 0);
    assert(strcmp(s.radar_loc, "Kokiri Shop") == 0);
    assert(s.wallet_cap == 500U && s.ammo[OOT_AMMO_ARROWS] == 22U && s.ammo_cap[OOT_AMMO_ARROWS] == 40U);
    assert(s.small_keys[3] == 4U && s.bottles[0] == OOT_BOTTLE_BLUE_FIRE);
    assert(strcmp(s.equip_weapon, "Weapon_Sword_Kokiri") == 0);
    assert(oot_companion_write_safe(&s) == 1);

    // Free Ocarina play activates DemoOcarina while keeping cutscene hide off.
    sim_wstr(VA_CMD, "DemoOcarina");
    sim_w64(VA_EVENT + OOT_EVENT_RUNNING, 0x1234ULL);
    assert(oot_companion_sample(&mem, SIM_BASE, &s) == 1);
    assert(s.playing_ocarina == 1U && s.in_cutscene == 0U);
    assert(oot_companion_write_safe(&s) == 1);

    // Non-ocarina cutscene OR open dialogue box OR performer link refuses writes.
    sim_wstr(VA_CMD, "EvtWait");
    assert(oot_companion_sample(&mem, SIM_BASE, &s) == 1);
    assert(s.playing_ocarina == 0U && s.in_cutscene == 1U && oot_companion_write_safe(&s) == 0);

    sim_w64(VA_EVENT + OOT_EVENT_RUNNING, 0ULL);
    sim_w32(VA_MSG + OOT_MSG_ID, 42U);
    sim_w32(VA_MSG + OOT_MSG_CHOICES, 2U);
    assert(oot_companion_sample(&mem, SIM_BASE, &s) == 1);
    assert(s.dialogue_open == 1U && s.dialogue_choices == 2U && oot_companion_write_safe(&s) == 0);
}

static void test_corrupt_pointers(void) {
    memset(&g_sim, 0, sizeof(g_sim));
    sim_w64(OOT_OFF_GAMEDATA_MGR_PTR, 0xDEADBEEFULL);
    OotTapMem mem = {sim_read, &g_sim};
    OotCompanionSnapshot s;
    assert(oot_companion_sample(&mem, SIM_BASE, &s) == 1);
    assert(s.life == 0U && s.scene[0] == '\0' && s.pose_valid == 0U);
    sim_w64(OOT_OFF_GAMEDATA_MGR_PTR, 0ULL);
    assert(oot_companion_sample(&mem, SIM_BASE, &s) == 0);
    assert(s.valid == 0U && oot_companion_write_safe(&s) == 0);
}

int main(void) {
    test_murmur_hashes();
    test_world_modes();
    test_corrupt_pointers();
    printf("t_oot_companion_tap: PASS\n");
    return 0;
}
