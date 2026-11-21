// SPDX-License-Identifier: MIT
// Clean-room MIT metadata catalog for Switch 2 Ocarina of Time (0400486024DEC000):
// 12 Ocarina melodies, Switch 2 Normal/Classic note bindings, bottle contents,
// and internal scene ID -> display title + dungeon small-key index mapping.
// Synced from P1R4N351/nx-second-screen (adapters/oot_sw2/oot_sw2_catalog.h).
#ifndef NX_OOT_SW2_CATALOG_H
#define NX_OOT_SW2_CATALOG_H

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    OOT_NOTE_D4 = 0,
    OOT_NOTE_F4 = 1,
    OOT_NOTE_A4 = 2,
    OOT_NOTE_B4 = 3,
    OOT_NOTE_D5 = 4,
    OOT_NOTE_COUNT = 5
};

enum {
    OOT_BOTTLE_NONE = 0,
    OOT_BOTTLE_EMPTY = 1,
    OOT_BOTTLE_RED_POTION = 2,
    OOT_BOTTLE_GREEN_POTION = 3,
    OOT_BOTTLE_BLUE_POTION = 4,
    OOT_BOTTLE_FAIRY = 5,
    OOT_BOTTLE_MILK = 6,
    OOT_BOTTLE_BLUE_FIRE = 7,
    OOT_BOTTLE_FISH = 8,
    OOT_BOTTLE_BUG = 9,
    OOT_BOTTLE_POE = 10,
    OOT_BOTTLE_BIG_POE = 11,
    OOT_BOTTLE_LETTER = 12
};

typedef struct OotSongSpec {
    const char *name;
    uint8_t notes[8];
    uint8_t length;
    uint8_t is_warp;
} OotSongSpec;

static const OotSongSpec OOT_SW2_SONGS[12] = {
    { "Zelda's Lullaby",    { OOT_NOTE_B4, OOT_NOTE_D5, OOT_NOTE_A4, OOT_NOTE_B4, OOT_NOTE_D5, OOT_NOTE_A4, 0U, 0U }, 6U, 0U },
    { "Epona's Song",       { OOT_NOTE_D5, OOT_NOTE_B4, OOT_NOTE_A4, OOT_NOTE_D5, OOT_NOTE_B4, OOT_NOTE_A4, 0U, 0U }, 6U, 0U },
    { "Saria's Song",       { OOT_NOTE_F4, OOT_NOTE_A4, OOT_NOTE_B4, OOT_NOTE_F4, OOT_NOTE_A4, OOT_NOTE_B4, 0U, 0U }, 6U, 0U },
    { "Sun's Song",         { OOT_NOTE_A4, OOT_NOTE_F4, OOT_NOTE_D5, OOT_NOTE_A4, OOT_NOTE_F4, OOT_NOTE_D5, 0U, 0U }, 6U, 0U },
    { "Song of Time",       { OOT_NOTE_A4, OOT_NOTE_D4, OOT_NOTE_F4, OOT_NOTE_A4, OOT_NOTE_D4, OOT_NOTE_F4, 0U, 0U }, 6U, 0U },
    { "Song of Storms",     { OOT_NOTE_D4, OOT_NOTE_F4, OOT_NOTE_D5, OOT_NOTE_D4, OOT_NOTE_F4, OOT_NOTE_D5, 0U, 0U }, 6U, 0U },
    { "Minuet of Forest",   { OOT_NOTE_D4, OOT_NOTE_D5, OOT_NOTE_B4, OOT_NOTE_A4, OOT_NOTE_B4, OOT_NOTE_A4, 0U, 0U }, 6U, 1U },
    { "Bolero of Fire",     { OOT_NOTE_F4, OOT_NOTE_D4, OOT_NOTE_F4, OOT_NOTE_D4, OOT_NOTE_A4, OOT_NOTE_F4, OOT_NOTE_A4, OOT_NOTE_F4 }, 8U, 1U },
    { "Serenade of Water",  { OOT_NOTE_D4, OOT_NOTE_F4, OOT_NOTE_A4, OOT_NOTE_A4, OOT_NOTE_B4, 0U, 0U, 0U }, 5U, 1U },
    { "Nocturne of Shadow", { OOT_NOTE_B4, OOT_NOTE_A4, OOT_NOTE_A4, OOT_NOTE_D4, OOT_NOTE_B4, OOT_NOTE_A4, OOT_NOTE_F4, 0U }, 7U, 1U },
    { "Requiem of Spirit",  { OOT_NOTE_D4, OOT_NOTE_F4, OOT_NOTE_D4, OOT_NOTE_A4, OOT_NOTE_F4, OOT_NOTE_D4, 0U, 0U }, 6U, 1U },
    { "Prelude of Light",   { OOT_NOTE_D5, OOT_NOTE_A4, OOT_NOTE_D5, OOT_NOTE_A4, OOT_NOTE_B4, OOT_NOTE_D5, 0U, 0U }, 6U, 1U }
};

static inline uint64_t oot_sw2_note_button(
    uint32_t note, int classic_mode,
    uint64_t btn_a, uint64_t btn_b, uint64_t btn_x,
    uint64_t btn_y, uint64_t btn_l, uint64_t btn_r) {
    const uint64_t normal_map[OOT_NOTE_COUNT] = { btn_l, btn_r, btn_a, btn_y, btn_x };
    const uint64_t classic_map[OOT_NOTE_COUNT] = { btn_r, btn_b, btn_a, btn_y, btn_x };
    if (note >= OOT_NOTE_COUNT) return 0ULL;
    return classic_mode ? classic_map[note] : normal_map[note];
}

static inline uint8_t oot_sw2_bottle_decode(uint32_t hash_val) {
    static const uint32_t hashes[12] = {
        0xBE69DE49U, 0xEABA6067U, 0x049D37C1U, 0xF383D6E5U,
        0x6F49E357U, 0x73356827U, 0x1C293517U, 0x1E834EF8U,
        0x74D62ECBU, 0x02084F9EU, 0xCDC21471U, 0x0F24E808U
    };
    for (uint32_t i = 0U; i < 12U; i++) {
        if (hash_val == hashes[i]) return (uint8_t)(i + 1U);
    }
    return OOT_BOTTLE_NONE;
}

static inline const char *oot_sw2_bottle_label(uint8_t kind) {
    static const char *const kLabels[13] = {
        "---", "Empty", "Red Potion", "Green Potion", "Blue Potion",
        "Fairy", "Milk", "Blue Fire", "Fish", "Bugs",
        "Poe", "Big Poe", "Letter"
    };
    return (kind < 13U) ? kLabels[kind] : "---";
}

typedef struct OotSceneEntry {
    const char *prefix;
    const char *display;
    int32_t dungeon_key_idx;
} OotSceneEntry;

static inline const char *oot_sw2_resolve_scene(
    const char *raw, int32_t *out_dungeon_key_idx) {
    static const OotSceneEntry kScenes[40] = {
        { "Spot00",      "Hyrule Field",              -1 },
        { "Spot01",      "Kakariko Village",          -1 },
        { "Spot02",      "Graveyard",                 -1 },
        { "Spot03",      "Zora's River",              -1 },
        { "Spot04",      "Kokiri Forest",             -1 },
        { "KokiriForest","Kokiri Forest",             -1 },
        { "Spot05",      "Sacred Forest Meadow",      -1 },
        { "Spot06",      "Lake Hylia",                -1 },
        { "Spot07",      "Zora's Domain",             -1 },
        { "Spot08",      "Zora's Fountain",           -1 },
        { "Spot09",      "Gerudo Valley",             -1 },
        { "Spot10",      "Lost Woods",                -1 },
        { "Spot11",      "Desert Colossus",           -1 },
        { "Spot12",      "Gerudo's Fortress",          6 },
        { "Spot13",      "Haunted Wasteland",         -1 },
        { "Spot15",      "Hyrule Castle",             -1 },
        { "Spot16",      "Death Mountain Trail",      -1 },
        { "Spot17",      "Death Mountain Crater",     -1 },
        { "Spot18",      "Goron City",                -1 },
        { "Spot20",      "Lon Lon Ranch",             -1 },
        { "Market",      "Market",                    -1 },
        { "Castle",      "Temple / Castle",           -1 },
        { "Tokinoma",    "Temple of Time",            -1 },
        { "Link",        "Link's House",              -1 },
        { "Shop",        "Bazaar / Shop",             -1 },
        { "Haka",        "Bottom of the Well",         0 },
        { "HAKAdan",     "Shadow Temple",              4 },
        { "Ydan",        "Inside the Deku Tree",      -1 },
        { "Ddan",        "Dodongo's Cavern",          -1 },
        { "Bdan",        "Inside Jabu-Jabu's Belly",  -1 },
        { "Bmori",       "Forest Temple",              1 },
        { "HIDAN",       "Fire Temple",                2 },
        { "MIZUsin",     "Water Temple",               3 },
        { "Jyasinzou",   "Spirit Temple",              5 },
        { "Gerudoway",   "Thieves' Hideout",           6 },
        { "Men",         "Gerudo Training Ground",     7 },
        { "Ice_doukutu", "Ice Cavern",                -1 },
        { "Ganon",       "Ganon's Castle",            -1 },
        { "Besitu",      "Chamber of Sages",          -1 },
        { "Title",       "Title Screen",              -1 }
    };
    if (out_dungeon_key_idx) *out_dungeon_key_idx = -1;
    if (!raw || raw[0] == '\0') return "Hyrule";
    for (uint32_t i = 0U; i < 40U; i++) {
        size_t plen = strlen(kScenes[i].prefix);
        if (strncmp(raw, kScenes[i].prefix, plen) == 0) {
            if (out_dungeon_key_idx) *out_dungeon_key_idx = kScenes[i].dungeon_key_idx;
            return kScenes[i].display;
        }
    }
    return raw;
}

#ifdef __cplusplus
}
#endif

#endif
