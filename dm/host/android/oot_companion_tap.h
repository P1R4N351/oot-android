// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only Switch 2 Ocarina of Time (0400486024DEC000, main v131072, Build ID
// e0f3ccfaf50065ae4ea01eb5aeb13b1a45da3348) runtime companion state reader.
//
// Combines verified guest offsets and structs from open-source Switch 2 OoT
// community work:
//   - Longshot / Hookshot (GPL-3.0, host/oot_state_tap.c) & Longshot-Narrator
//     (GPL-3.0, MSG_HOLDER layout)
//   - daurosk/oot-remake-multiplayer (GPL-3.0, coop/host/src/addresses.hpp,
//     coop/host/include/coop/host/scene.hpp)
//   - jfhutchi/NaviScout (MIT, app/memory.py), Traiver/OOT-SW2SE (MIT),
//     losava/Links-Toolbox (MIT, oot_catalog.py), and BananaNative (E0F3CCFA)
//
// Zero dynamic memory allocation; all probe and string loops are statically
// bounded for NASA Power-of-Ten compliance.

#ifndef OOT_COMPANION_TAP_H
#define OOT_COMPANION_TAP_H

#include "oot_companion_catalog.h"
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OOT_COMPANION_ABI_VER 1U
#define OOT_TAP_MAX_PROBE 8192U
#define OOT_TAP_MAX_STR 64U
#define OOT_TAP_MAX_HASH_STR 256U

// Verified v131072 main BSS singleton pointer offsets (relative to main_base).
#define OOT_OFF_MSG_HOLDER_PTR    0x04E790B8ULL
#define OOT_OFF_GAMEUI_MGR_PTR    0x04E7CFA8ULL
#define OOT_OFF_GAMEDATA_MGR_PTR  0x04E842A8ULL
#define OOT_OFF_BANC_MGR_PTR      0x04E919D0ULL
#define OOT_OFF_EVENT_MGR_PTR     0x04E94840ULL
#define OOT_OFF_SCENE_MGR_PTR     0x04E97648ULL

// gmd::GameDataMgr typed-container offsets and element strides.
#define OOT_GDM_BOOL_OFF          0x008U
#define OOT_GDM_BOOL_STRIDE       0x18U
#define OOT_GDM_BOOL_VAL          0x16U
#define OOT_GDM_INT_OFF           0x0C8U
#define OOT_GDM_INT_STRIDE        0x28U
#define OOT_GDM_FLOAT_OFF         0x188U
#define OOT_GDM_FLOAT_STRIDE      0x28U
#define OOT_GDM_STR64_OFF         0x608U
#define OOT_GDM_STR64_STRIDE      0x108U
#define OOT_GDM_STR64_PTR         0x68U
#define OOT_GDM_NUM_VAL           0x1CU

// Scene / Actor / GameUI / EventMgr / Message struct offsets (e0f3ccfa...).
#define OOT_UI_POSE_FLAGS         0x58AU
#define OOT_UI_POSE_POS           0x590U
#define OOT_UI_IS_CHILD           0x5C0U
#define OOT_SCENE_TABLE           0x1F0U
#define OOT_TABLE_COUNT           0x50U
#define OOT_TABLE_ARRAY           0x58U
#define OOT_PLAYER_SLOT_OFF       0x78U
#define OOT_MGR_MUST_SET          0x30U
#define OOT_MGR_LINK0             0x40U
#define OOT_MGR_LINK1             0x50U
#define OOT_LINK_HOLDER           0x08U
#define OOT_LINK_ID               0x10U
#define OOT_HOLDER_ACTOR          0x40U
#define OOT_HOLDER_ID             0x48U
#define OOT_ACTOR_STATE           0x2CU
#define OOT_ACTOR_POS             0x31CU
#define OOT_ACTOR_COMPS           0x270U
#define OOT_COMP_AS               0x40U
#define OOT_COMP_PERF0            0x88U
#define OOT_COMP_PERF1            0x90U
#define OOT_AS_INNER              0x18U
#define OOT_AS_CTRL               0x18U
#define OOT_AS_SLOT_COUNT         0x18U
#define OOT_AS_SLOT_ARRAY         0x20U
#define OOT_AS_SLOT0_CMD          0x10U
#define OOT_PERF_LINK             0x18U
#define OOT_BANC_MUST_SET         0x38U
#define OOT_BANC_PATH_PTR         0x40U
#define OOT_EVENT_RUNNING         0x68U
#define OOT_MSG_CHOICES           0x04U
#define OOT_MSG_ID                0x30U

// MurmurHash3_x86_32 (seed 0) keys for gmd::GameDataMgr flags.
#define OOT_KEY_CURRENT_BANC      0x1D6189DAU
#define OOT_KEY_RADAR_LOCATION    0xF74E0E8EU
#define OOT_KEY_LOADING_BANC      0xEF2B60C2U
#define OOT_KEY_IS_CHILDHOOD      0xCEA220D3U
#define OOT_KEY_ROT_FOR_MAP       0xA4AEE8CDU
#define OOT_KEY_LIFE              0x31AB5580U
#define OOT_KEY_MAX_LIFE          0xFBE01DA1U
#define OOT_KEY_MP                0x76B18200U
#define OOT_KEY_MP_MAX            0x12D63A97U
#define OOT_KEY_MP_UNLOCKED       0xB1309073U
#define OOT_KEY_RUPEES            0xA77921D7U
#define OOT_KEY_WALLET_CAP        0xE0010A50U
#define OOT_KEY_GOLD_TOKENS       0x04735978U
#define OOT_KEY_ACTION_PROMPT     0x9824206CU
#define OOT_KEY_EQUIP_WEAPON      0xA95A3521U
#define OOT_KEY_EQUIP_SHIELD      0xEE463235U
#define OOT_KEY_EQUIP_ARMOR       0x008D7F0CU
#define OOT_KEY_EQUIP_BOOT        0xD107A85DU
#define OOT_KEY_EQUIP_OCARINA     0xB72CD3FCU
#define OOT_KEY_EQUIP_EXTRA0      0xA1F7EE2BU
#define OOT_KEY_EQUIP_EXTRA1      0x2342B619U
#define OOT_KEY_STICKS            0xB9494411U
#define OOT_KEY_STICKS_CAP        0x457616B0U
#define OOT_KEY_NUTS              0xF30D9F54U
#define OOT_KEY_NUTS_CAP          0x735B00F7U
#define OOT_KEY_SEEDS             0x5FADF7FCU
#define OOT_KEY_SEEDS_CAP         0xAE8CC742U
#define OOT_KEY_ARROWS            0x518EC74BU
#define OOT_KEY_ARROWS_CAP        0x418DB791U
#define OOT_KEY_BOMBS             0xDB1BCD4FU
#define OOT_KEY_BOMBCHUS          0xDF7C517AU
#define OOT_KEY_BOMBS_CAP         0xE753508BU
#define OOT_KEY_BEANS             0xB003124DU

#define OOT_AMMO_STICKS           0U
#define OOT_AMMO_NUTS             1U
#define OOT_AMMO_SEEDS            2U
#define OOT_AMMO_ARROWS           3U
#define OOT_AMMO_BOMBS            4U
#define OOT_AMMO_BOMBCHUS         5U
#define OOT_AMMO_COUNT            6U
#define OOT_KEY_DUNGEON_N         8U
#define OOT_BOTTLE_SLOTS          5U

typedef int (*oot_guest_read_fn)(void *ctx, uint64_t addr, void *dst, size_t len);

typedef struct {
    oot_guest_read_fn read;
    void *ctx;
} OotTapMem;

typedef struct {
    uint32_t version;
    uint32_t valid;
    uint32_t is_child;
    uint32_t is_loading;
    uint32_t in_transition;
    uint32_t in_cutscene;
    uint32_t playing_ocarina;
    uint32_t riding_horse;
    uint32_t dialogue_open;
    uint32_t dialogue_choices;
    uint32_t pose_valid;
    uint32_t life;
    uint32_t max_life;
    uint32_t magic;
    uint32_t max_magic;
    uint32_t magic_unlocked;
    uint32_t rupees;
    uint32_t wallet_cap;
    uint32_t tokens;
    uint32_t beans;
    uint32_t ammo[OOT_AMMO_COUNT];
    uint32_t ammo_cap[OOT_AMMO_COUNT];
    uint32_t small_keys[OOT_KEY_DUNGEON_N];
    uint8_t bottles[OOT_BOTTLE_SLOTS];
    float pos[3];
    float yaw_deg;
    uint32_t stones_mask;
    uint32_t medallions_mask;
    uint32_t songs_mask;
    char scene[64];
    char radar_loc[64];
    char as_command[64];
    char action_prompt[64];
    char equip_weapon[32];
    char equip_shield[32];
    char equip_armor[32];
    char equip_boot[32];
    char equip_ocarina[32];
    char c_slots[4][32];
} OotCompanionSnapshot;

static inline uint32_t oot_rotl32(uint32_t v, uint32_t r) {
    return (v << r) | (v >> (32U - r));
}

static inline uint32_t oot_murmur3_32(const char *s, size_t len) {
    if (!s || len > OOT_TAP_MAX_HASH_STR) return 0U;
    uint32_t h = 0U;
    size_t i = 0U;
    for (; i + 4U <= len && i < OOT_TAP_MAX_HASH_STR; i += 4U) {
        uint32_t k = (uint32_t)(uint8_t)s[i] | ((uint32_t)(uint8_t)s[i + 1U] << 8U) |
                     ((uint32_t)(uint8_t)s[i + 2U] << 16U) | ((uint32_t)(uint8_t)s[i + 3U] << 24U);
        k = oot_rotl32(k * 0xCC9E2D51U, 15U) * 0x1B873593U;
        h = oot_rotl32(h ^ k, 13U) * 5U + 0xE6546B64U;
    }
    uint32_t tail = 0U;
    size_t rem = len & 3U;
    if (rem >= 3U) tail ^= (uint32_t)(uint8_t)s[i + 2U] << 16U;
    if (rem >= 2U) tail ^= (uint32_t)(uint8_t)s[i + 1U] << 8U;
    if (rem >= 1U) {
        tail ^= (uint32_t)(uint8_t)s[i];
        h ^= oot_rotl32(tail * 0xCC9E2D51U, 15U) * 0x1B873593U;
    }
    h ^= (uint32_t)len;
    h ^= h >> 16U;
    h *= 0x85EBCA6BU;
    h ^= h >> 13U;
    h *= 0xC2B2AE35U;
    return h ^ (h >> 16U);
}

static inline int oot_tap_read_mem(const OotTapMem *mem, uint64_t addr, void *dst, size_t len) {
    if (!mem || !mem->read || !dst || addr < 0x10000U || len == 0U || len > 256U) return 0;
    return mem->read(mem->ctx, addr, dst, len) != 0;
}

static inline int oot_tap_u8(const OotTapMem *mem, uint64_t addr, uint8_t *out) {
    return oot_tap_read_mem(mem, addr, out, sizeof(*out));
}

static inline int oot_tap_u16(const OotTapMem *mem, uint64_t addr, uint16_t *out) {
    return oot_tap_read_mem(mem, addr, out, sizeof(*out));
}

static inline int oot_tap_u32(const OotTapMem *mem, uint64_t addr, uint32_t *out) {
    return oot_tap_read_mem(mem, addr, out, sizeof(*out));
}

static inline int oot_tap_u64(const OotTapMem *mem, uint64_t addr, uint64_t *out) {
    return oot_tap_read_mem(mem, addr, out, sizeof(*out));
}

static inline int oot_tap_f32(const OotTapMem *mem, uint64_t addr, float *out) {
    float v = 0.0f;
    if (!oot_tap_read_mem(mem, addr, &v, sizeof(v))) return 0;
    if (!isfinite(v)) return 0;
    *out = v;
    return 1;
}

static inline int oot_tap_cstr(const OotTapMem *mem, uint64_t addr, char *dst, size_t cap) {
    if (!dst || cap == 0U) return 0;
    dst[0] = '\0';
    if (!mem || !mem->read || addr < 0x10000U) return 0;
    size_t max_len = cap - 1U;
    if (max_len > OOT_TAP_MAX_STR) max_len = OOT_TAP_MAX_STR;
    uint8_t raw[OOT_TAP_MAX_STR + 1U];
    memset(raw, 0, sizeof(raw));
    if (!mem->read(mem->ctx, addr, raw, max_len)) return 0;
    size_t n = 0U;
    for (size_t i = 0U; i < max_len; i++) {
        uint8_t c = raw[i];
        if (c == 0U) break;
        if (c < 0x20U && c != '\t' && c != '\n') return 0;
        dst[n++] = (char)c;
    }
    dst[n] = '\0';
    return 1;
}

static inline int oot_tap_starts_with(const char *s, const char *prefix) {
    if (!s || !prefix) return 0;
    for (size_t i = 0U; i < OOT_TAP_MAX_STR; i++) {
        if (prefix[i] == '\0') return 1;
        if (s[i] != prefix[i]) return 0;
    }
    return 0;
}

static inline int oot_tap_contains(const char *hay, const char *needle) {
    if (!hay || !needle || needle[0] == '\0') return 0;
    for (size_t i = 0U; i < OOT_TAP_MAX_STR && hay[i] != '\0'; i++) {
        size_t j = 0U;
        for (; j < OOT_TAP_MAX_STR && needle[j] != '\0' && (i + j) < OOT_TAP_MAX_STR; j++) {
            if (hay[i + j] != needle[j]) break;
        }
        if (needle[j] == '\0') return 1;
    }
    return 0;
}

static inline void oot_tap_norm_scene(const char *src, char *dst, size_t cap) {
    if (!dst || cap == 0U) return;
    dst[0] = '\0';
    if (!src || src[0] == '\0') return;
    size_t start = 0U;
    for (size_t i = 0U; i < OOT_TAP_MAX_STR && src[i] != '\0'; i++) {
        if (src[i] == '/' || src[i] == '\\') start = i + 1U;
    }
    size_t n = 0U;
    for (size_t i = start; i < OOT_TAP_MAX_STR && src[i] != '\0' && n + 1U < cap; i++) {
        if (src[i] == '.') break;
        dst[n++] = src[i];
    }
    dst[n] = '\0';
}

static inline uint64_t oot_tap_gdm_entry(
    const OotTapMem *mem, uint64_t mgr, uint32_t kind_off, uint32_t stride, uint32_t hash) {
    if (!mgr || !hash) return 0U;
    const uint64_t box = mgr + kind_off;
    uint32_t count = 0U, slots = 0U;
    uint64_t entries = 0U, table = 0U;
    if (!oot_tap_u32(mem, box + 8U, &count) || !oot_tap_u64(mem, box + 16U, &entries) ||
        !oot_tap_u64(mem, box + 24U, &table) || !oot_tap_u32(mem, box + 36U, &slots)) {
        return 0U;
    }
    if (!entries || !table || slots == 0U || count == 0U || slots > (1U << 20U)) return 0U;
    uint32_t slot = hash % slots;
    uint32_t limit = slots < OOT_TAP_MAX_PROBE ? slots : OOT_TAP_MAX_PROBE;
    for (uint32_t probe = 0U; probe < limit; probe++) {
        uint32_t pair[2] = {0U, 0U};
        if (!oot_tap_read_mem(mem, table + (uint64_t)slot * 8ULL, pair, sizeof(pair)) || pair[0] == 0U) {
            return 0U;
        }
        if (pair[0] == hash) {
            return pair[1] < count ? entries + (uint64_t)pair[1] * (uint64_t)stride : 0U;
        }
        slot = (slot + 1U < slots) ? (slot + 1U) : 0U;
    }
    return 0U;
}

static inline int oot_tap_gdm_bool(const OotTapMem *mem, uint64_t mgr, uint32_t hash, uint32_t *out) {
    uint64_t e = oot_tap_gdm_entry(mem, mgr, OOT_GDM_BOOL_OFF, OOT_GDM_BOOL_STRIDE, hash);
    uint8_t v = 0U;
    if (!e || !oot_tap_u8(mem, e + OOT_GDM_BOOL_VAL, &v)) return 0;
    *out = (v != 0U) ? 1U : 0U;
    return 1;
}

static inline int oot_tap_gdm_u32(const OotTapMem *mem, uint64_t mgr, uint32_t hash, uint32_t *out) {
    uint64_t e = oot_tap_gdm_entry(mem, mgr, OOT_GDM_INT_OFF, OOT_GDM_INT_STRIDE, hash);
    return e && oot_tap_u32(mem, e + OOT_GDM_NUM_VAL, out);
}

static inline int oot_tap_gdm_f32(const OotTapMem *mem, uint64_t mgr, uint32_t hash, float *out) {
    uint64_t e = oot_tap_gdm_entry(mem, mgr, OOT_GDM_FLOAT_OFF, OOT_GDM_FLOAT_STRIDE, hash);
    return e && oot_tap_f32(mem, e + OOT_GDM_NUM_VAL, out);
}

static inline int oot_tap_gdm_str(const OotTapMem *mem, uint64_t mgr, uint32_t hash, char *dst, size_t cap) {
    if (!dst || cap == 0U) return 0;
    dst[0] = '\0';
    uint64_t e = oot_tap_gdm_entry(mem, mgr, OOT_GDM_STR64_OFF, OOT_GDM_STR64_STRIDE, hash);
    uint64_t ptr = 0U;
    if (!e || !oot_tap_u64(mem, e + OOT_GDM_STR64_PTR, &ptr)) return 0;
    return oot_tap_cstr(mem, ptr, dst, cap);
}

static inline uint64_t oot_tap_find_actor(const OotTapMem *mem, uint64_t main_addr) {
    uint64_t scene = 0U, table = 0U, arr = 0U, mgr = 0U, must = 0U;
    uint32_t count = 0U;
    if (!oot_tap_u64(mem, main_addr + OOT_OFF_SCENE_MGR_PTR, &scene) ||
        !oot_tap_u64(mem, scene + OOT_SCENE_TABLE, &table) ||
        !oot_tap_u32(mem, table + OOT_TABLE_COUNT, &count) || count < 16U ||
        !oot_tap_u64(mem, table + OOT_TABLE_ARRAY, &arr) ||
        !oot_tap_u64(mem, arr + OOT_PLAYER_SLOT_OFF, &mgr) ||
        !oot_tap_u64(mem, mgr + OOT_MGR_MUST_SET, &must) || must == 0U) {
        return 0U;
    }
    const uint64_t link_offs[2] = {OOT_MGR_LINK0, OOT_MGR_LINK1};
    for (uint32_t i = 0U; i < 2U; i++) {
        uint64_t link = 0U, holder = 0U, actor = 0U;
        uint32_t id = 0U, hid = 0U, st = 0U;
        if (!oot_tap_u64(mem, mgr + link_offs[i], &link) || !link) continue;
        if (!oot_tap_u32(mem, link + OOT_LINK_ID, &id) || id == 0xFFFFFFFFU) continue;
        if (!oot_tap_u64(mem, link + OOT_LINK_HOLDER, &holder) || !holder) continue;
        if (!oot_tap_u32(mem, holder + OOT_HOLDER_ID, &hid) || hid != id) continue;
        if (!oot_tap_u64(mem, holder + OOT_HOLDER_ACTOR, &actor) || !actor) continue;
        if (oot_tap_u32(mem, actor + OOT_ACTOR_STATE, &st) && st == 4U) return actor;
    }
    return 0U;
}

static inline void oot_tap_read_actor_state(
    const OotTapMem *mem, uint64_t actor, OotCompanionSnapshot *out, uint32_t *bound_out) {
    uint64_t comps = 0U, as_comp = 0U, as_inner = 0U, as_ctrl = 0U, slot0 = 0U, cmd_ptr = 0U;
    uint32_t slots = 0U;
    uint64_t slot_arr = 0U;
    *bound_out = 0U;
    if (!actor || !oot_tap_u64(mem, actor + OOT_ACTOR_COMPS, &comps) || !comps) return;
    if (oot_tap_u64(mem, comps + OOT_COMP_AS, &as_comp) &&
        oot_tap_u64(mem, as_comp + OOT_AS_INNER, &as_inner) &&
        oot_tap_u64(mem, as_inner + OOT_AS_CTRL, &as_ctrl) &&
        oot_tap_u32(mem, as_ctrl + OOT_AS_SLOT_COUNT, &slots) && slots > 0U &&
        oot_tap_u64(mem, as_ctrl + OOT_AS_SLOT_ARRAY, &slot_arr) &&
        oot_tap_u64(mem, slot_arr, &slot0) &&
        oot_tap_u64(mem, slot0 + OOT_AS_SLOT0_CMD, &cmd_ptr) &&
        oot_tap_cstr(mem, cmd_ptr, out->as_command, sizeof(out->as_command))) {
        if ((oot_tap_starts_with(out->as_command, "DemoOcarina") &&
             !oot_tap_contains(out->as_command, "MelodyGet")) ||
            oot_tap_starts_with(out->as_command, "Ocarina")) {
            out->playing_ocarina = 1U;
        }
        if (oot_tap_contains(out->as_command, "Horse") &&
            !oot_tap_starts_with(out->as_command, "Dismoun") &&
            strcmp(out->as_command, "HorseAccess") != 0) {
            out->riding_horse = 1U;
        }
    }
    const uint64_t perf_offs[2] = {OOT_COMP_PERF0, OOT_COMP_PERF1};
    for (uint32_t i = 0U; i < 2U; i++) {
        uint64_t perf = 0U, link = 0U;
        if (oot_tap_u64(mem, comps + perf_offs[i], &perf) &&
            oot_tap_u64(mem, perf + OOT_PERF_LINK, &link) && link != 0U) {
            *bound_out = 1U;
        }
    }
}

static inline void oot_tap_read_quest_bits(
    const OotTapMem *mem, uint64_t mgr, OotCompanionSnapshot *out) {
    static const uint32_t kTrans[5] = {0xF5DEEF0AU, 0x9A9506E7U, 0xF8633C7FU, 0xF96989B6U, 0xE19DFDD6U};
    static const uint32_t kStones[3] = {0xCBD1E58CU, 0x7D69AD47U, 0xB8E3BF45U};
    static const uint32_t kMedals[6] = {
        0x235C7D9AU, 0x692D57CBU, 0x5CE254A6U, 0x0151A68FU, 0x204D2063U, 0x0C7A5FD0U};
    static const uint32_t kSongs[12] = {
        0x01C20002U, 0xB492AC33U, 0xD3F8BFCCU, 0x165DD1ADU, 0x03B3450EU, 0x2B39DD4FU,
        0xDC0A0B54U, 0x49041AB9U, 0x3FA63986U, 0xC2FB95F4U, 0x802A36A8U, 0xD6320BA4U};
    uint32_t v = 0U;
    for (uint32_t i = 0U; i < 5U; i++) {
        if (oot_tap_gdm_bool(mem, mgr, kTrans[i], &v) && v != 0U) out->in_transition = 1U;
    }
    for (uint32_t i = 0U; i < 3U; i++) {
        if (oot_tap_gdm_u32(mem, mgr, kStones[i], &v) && v > 0U) out->stones_mask |= (1U << i);
    }
    for (uint32_t i = 0U; i < 6U; i++) {
        if (oot_tap_gdm_u32(mem, mgr, kMedals[i], &v) && v > 0U) out->medallions_mask |= (1U << i);
    }
    for (uint32_t i = 0U; i < 12U; i++) {
        if (oot_tap_gdm_bool(mem, mgr, kSongs[i], &v) && v != 0U) out->songs_mask |= (1U << i);
    }
}

static inline void oot_tap_read_world_context(
    const OotTapMem *mem, uint64_t main_addr, uint64_t actor, OotCompanionSnapshot *out) {
    uint64_t ui = 0U, banc = 0U, must = 0U, path = 0U, msg = 0U;
    uint16_t pflags = 0U;
    uint8_t child_u8 = 0U;
    char raw_scene[64];
    if (oot_tap_u64(mem, main_addr + OOT_OFF_GAMEUI_MGR_PTR, &ui) && ui) {
        if (oot_tap_u8(mem, ui + OOT_UI_IS_CHILD, &child_u8) && child_u8 <= 1U && !out->valid) {
            out->is_child = child_u8;
        }
        if (oot_tap_u16(mem, ui + OOT_UI_POSE_FLAGS, &pflags) && (pflags & 1U) &&
            oot_tap_f32(mem, ui + OOT_UI_POSE_POS, &out->pos[0]) &&
            oot_tap_f32(mem, ui + OOT_UI_POSE_POS + 4U, &out->pos[1]) &&
            oot_tap_f32(mem, ui + OOT_UI_POSE_POS + 8U, &out->pos[2])) {
            out->pose_valid = 1U;
        }
    }
    if (!out->pose_valid && actor &&
        oot_tap_f32(mem, actor + OOT_ACTOR_POS, &out->pos[0]) &&
        oot_tap_f32(mem, actor + OOT_ACTOR_POS + 4U, &out->pos[1]) &&
        oot_tap_f32(mem, actor + OOT_ACTOR_POS + 8U, &out->pos[2]) &&
        fabsf(out->pos[0]) < 1e5f && fabsf(out->pos[1]) < 1e5f && fabsf(out->pos[2]) < 1e5f) {
        out->pose_valid = 1U;
    }
    if (out->scene[0] == '\0' &&
        oot_tap_u64(mem, main_addr + OOT_OFF_BANC_MGR_PTR, &banc) &&
        oot_tap_u64(mem, banc + OOT_BANC_MUST_SET, &must) && must &&
        oot_tap_u64(mem, banc + OOT_BANC_PATH_PTR, &path) &&
        oot_tap_cstr(mem, path, raw_scene, sizeof(raw_scene))) {
        oot_tap_norm_scene(raw_scene, out->scene, sizeof(out->scene));
    }
    uint32_t msg_id = 0xFFFFFFFFU, choices = 0U;
    if (oot_tap_u64(mem, main_addr + OOT_OFF_MSG_HOLDER_PTR, &msg) && msg &&
        oot_tap_u32(mem, msg + OOT_MSG_ID, &msg_id) && msg_id != 0xFFFFFFFFU) {
        out->dialogue_open = 1U;
        if (oot_tap_u32(mem, msg + OOT_MSG_CHOICES, &choices) && choices <= 8U) {
            out->dialogue_choices = choices;
        }
    }
}

static inline void oot_tap_read_supplies_and_keys(
    const OotTapMem *mem, uint64_t mgr, OotCompanionSnapshot *out) {
    static const uint32_t kAmmo[OOT_AMMO_COUNT] = {
        OOT_KEY_STICKS, OOT_KEY_NUTS, OOT_KEY_SEEDS,
        OOT_KEY_ARROWS, OOT_KEY_BOMBS, OOT_KEY_BOMBCHUS
    };
    static const uint32_t kCap[OOT_AMMO_COUNT] = {
        OOT_KEY_STICKS_CAP, OOT_KEY_NUTS_CAP, OOT_KEY_SEEDS_CAP,
        OOT_KEY_ARROWS_CAP, OOT_KEY_BOMBS_CAP, OOT_KEY_BOMBS_CAP
    };
    static const uint32_t kSmallKeys[OOT_KEY_DUNGEON_N] = {
        0x7DD79D10U, 0x90237092U, 0x1DF11487U, 0x877BDCEEU,
        0x5EDDA9E3U, 0x79EEEC8EU, 0x3A28B6A4U, 0xEA9786F5U
    };
    (void)oot_tap_gdm_u32(mem, mgr, OOT_KEY_WALLET_CAP, &out->wallet_cap);
    (void)oot_tap_gdm_u32(mem, mgr, OOT_KEY_BEANS, &out->beans);
    for (uint32_t i = 0U; i < OOT_AMMO_COUNT; i++) {
        (void)oot_tap_gdm_u32(mem, mgr, kAmmo[i], &out->ammo[i]);
        (void)oot_tap_gdm_u32(mem, mgr, kCap[i], &out->ammo_cap[i]);
    }
    for (uint32_t i = 0U; i < OOT_KEY_DUNGEON_N; i++) {
        (void)oot_tap_gdm_u32(mem, mgr, kSmallKeys[i], &out->small_keys[i]);
    }
}

static inline void oot_tap_read_bottles_and_slots(
    const OotTapMem *mem, uint64_t mgr, OotCompanionSnapshot *out) {
    static const uint32_t kBottles[OOT_BOTTLE_SLOTS][3] = {
        { 0xD53CE3A7U, 0xF39CE410U, 0x7D48DF76U },
        { 0xFCA29A04U, 0x000FF6EAU, 0x1EDD164AU },
        { 0x893216A1U, 0xF63B1194U, 0xB80C70A8U },
        { 0x5324F9D3U, 0xB7F052CEU, 0xE4C1B1FCU },
        { 0xC92B16BEU, 0xE27B136DU, 0x365E1953U }
    };
    static const uint32_t kSlots[4] = {
        OOT_KEY_EQUIP_WEAPON, OOT_KEY_EQUIP_SHIELD,
        OOT_KEY_EQUIP_EXTRA0, OOT_KEY_EQUIP_EXTRA1
    };
    (void)oot_tap_gdm_str(mem, mgr, OOT_KEY_RADAR_LOCATION, out->radar_loc, sizeof(out->radar_loc));
    for (uint32_t i = 0U; i < 4U; i++) {
        (void)oot_tap_gdm_str(mem, mgr, kSlots[i], out->c_slots[i], sizeof(out->c_slots[i]));
    }
    for (uint32_t i = 0U; i < OOT_BOTTLE_SLOTS; i++) {
        for (uint32_t k = 0U; k < 3U; k++) {
            uint32_t raw = 0U;
            if (oot_tap_gdm_u32(mem, mgr, kBottles[i][k], &raw)) {
                uint8_t kind = oot_sw2_bottle_decode(raw);
                if (kind != OOT_BOTTLE_NONE) {
                    out->bottles[i] = kind;
                    break;
                }
            }
        }
    }
}

static inline int oot_companion_sample(
    const OotTapMem *mem, uint64_t main_addr, OotCompanionSnapshot *out) {
    if (!mem || !mem->read || !out || main_addr < 0x10000U) return 0;
    memset(out, 0, sizeof(*out));
    out->version = OOT_COMPANION_ABI_VER;
    uint64_t mgr = 0U;
    if (oot_tap_u64(mem, main_addr + OOT_OFF_GAMEDATA_MGR_PTR, &mgr) && mgr) {
        out->valid = 1U;
        (void)oot_tap_gdm_bool(mem, mgr, OOT_KEY_IS_CHILDHOOD, &out->is_child);
        (void)oot_tap_gdm_bool(mem, mgr, OOT_KEY_LOADING_BANC, &out->is_loading);
        (void)oot_tap_gdm_bool(mem, mgr, OOT_KEY_MP_UNLOCKED, &out->magic_unlocked);
        (void)oot_tap_gdm_u32(mem, mgr, OOT_KEY_LIFE, &out->life);
        (void)oot_tap_gdm_u32(mem, mgr, OOT_KEY_MAX_LIFE, &out->max_life);
        (void)oot_tap_gdm_u32(mem, mgr, OOT_KEY_MP, &out->magic);
        (void)oot_tap_gdm_u32(mem, mgr, OOT_KEY_MP_MAX, &out->max_magic);
        (void)oot_tap_gdm_u32(mem, mgr, OOT_KEY_RUPEES, &out->rupees);
        (void)oot_tap_gdm_u32(mem, mgr, OOT_KEY_GOLD_TOKENS, &out->tokens);
        (void)oot_tap_gdm_f32(mem, mgr, OOT_KEY_ROT_FOR_MAP, &out->yaw_deg);
        (void)oot_tap_gdm_str(mem, mgr, OOT_KEY_CURRENT_BANC, out->scene, sizeof(out->scene));
        (void)oot_tap_gdm_str(mem, mgr, OOT_KEY_ACTION_PROMPT, out->action_prompt, sizeof(out->action_prompt));
        (void)oot_tap_gdm_str(mem, mgr, OOT_KEY_EQUIP_WEAPON, out->equip_weapon, sizeof(out->equip_weapon));
        (void)oot_tap_gdm_str(mem, mgr, OOT_KEY_EQUIP_SHIELD, out->equip_shield, sizeof(out->equip_shield));
        (void)oot_tap_gdm_str(mem, mgr, OOT_KEY_EQUIP_ARMOR, out->equip_armor, sizeof(out->equip_armor));
        (void)oot_tap_gdm_str(mem, mgr, OOT_KEY_EQUIP_BOOT, out->equip_boot, sizeof(out->equip_boot));
        (void)oot_tap_gdm_str(mem, mgr, OOT_KEY_EQUIP_OCARINA, out->equip_ocarina, sizeof(out->equip_ocarina));
        oot_tap_read_quest_bits(mem, mgr, out);
        oot_tap_read_supplies_and_keys(mem, mgr, out);
        oot_tap_read_bottles_and_slots(mem, mgr, out);
    }
    uint64_t actor = oot_tap_find_actor(mem, main_addr);
    uint32_t link_bound = 0U;
    oot_tap_read_actor_state(mem, actor, out, &link_bound);
    uint64_t ev_mgr = 0U, ev_running = 0U;
    (void)oot_tap_u64(mem, main_addr + OOT_OFF_EVENT_MGR_PTR, &ev_mgr);
    (void)oot_tap_u64(mem, ev_mgr + OOT_EVENT_RUNNING, &ev_running);
    if ((ev_running != 0U || link_bound != 0U) && !out->playing_ocarina) {
        out->in_cutscene = 1U;
    }
    oot_tap_read_world_context(mem, main_addr, actor, out);
    return (int)out->valid;
}

static inline int oot_companion_write_safe(const OotCompanionSnapshot *snap) {
    if (!snap || !snap->valid) return 0;
    return !snap->is_loading && !snap->in_transition && !snap->in_cutscene && !snap->dialogue_open;
}

#ifdef __cplusplus
}
#endif

#endif  // OOT_COMPANION_TAP_H
