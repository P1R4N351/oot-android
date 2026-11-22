// SPDX-License-Identifier: MIT
package dev.nxnative.android;

/**
 * Pure-JDK state model, 3D radar projection, catalog formatters, and Ocarina
 * songbook metadata for the Switch 2 OoT second-screen &amp; drawer companion.
 * Zero {@code android.*} imports so host JVM tests verify it under JDK.
 */
final class OotCompanionModel {
    static final int DETAIL_INTS = 52;
    static final int DETAIL_FLOATS = 412;
    static final int TRAIL_CAP = 128;
    static final int PIN_CAP = 8;

    static final int TAB_RADAR = 0;
    static final int TAB_LOADOUT = 1;
    static final int TAB_QUEST = 2;
    static final int TAB_OCARINA = 3;
    static final int TAB_COUNT = 4;

    static final int BAND_BELOW = -1;
    static final int BAND_LEVEL = 0;
    static final int BAND_ABOVE = 1;

    static final float FLOOR_BAND_M = 3.0f;
    static final float[] ZOOM_RADII_M = {40.0f, 80.0f, 160.0f};
    static final String[] TAB_LABELS = {"RADAR", "GEAR", "ALTAR", "OCARINA"};

    private static String[] of(String... s) { return s; }

    static String[] ammoLabels() {   // the labels in the app's language (Words: English in the JVM tests)
        return of(
            Words.s("oot_ammo_sticks", "Sticks"),
            Words.s("oot_ammo_nuts", "Nuts"),
            Words.s("oot_ammo_seeds", "Seeds"),
            Words.s("oot_ammo_arrows", "Arrows"),
            Words.s("oot_ammo_bombs", "Bombs"),
            Words.s("oot_ammo_chus", "Chus")
        );
    }
    static String[] dungeonNames() {   // the labels in the app's language (Words: English in the JVM tests)
        return of(
            Words.s("oot_dungeon_well", "Bottom of Well"),
            Words.s("oot_dungeon_forest", "Forest Temple"),
            Words.s("oot_dungeon_fire", "Fire Temple"),
            Words.s("oot_dungeon_water", "Water Temple"),
            Words.s("oot_dungeon_shadow", "Shadow Temple"),
            Words.s("oot_dungeon_spirit", "Spirit Temple"),
            Words.s("oot_dungeon_gerudo_fortress", "Gerudo Fortress"),
            Words.s("oot_dungeon_training_ground", "Training Ground")
        );
    }
    static String[] stoneNames() {   // the labels in the app's language (Words: English in the JVM tests)
        return of(
            Words.s("oot_stone_kokiri", "Kokiri Emerald"),
            Words.s("oot_stone_goron", "Goron Ruby"),
            Words.s("oot_stone_zora", "Zora Sapphire")
        );
    }
    static String[] medallionNames() {   // the labels in the app's language (Words: English in the JVM tests)
        return of(
            Words.s("oot_medallion_light", "Light"),
            Words.s("oot_medallion_forest", "Forest"),
            Words.s("oot_medallion_fire", "Fire"),
            Words.s("oot_medallion_water", "Water"),
            Words.s("oot_medallion_shadow", "Shadow"),
            Words.s("oot_medallion_spirit", "Spirit")
        );
    }
    static final int[] MEDALLION_COLORS = {
        0xFFEAB308, 0xFF22C55E, 0xFFEF4444, 0xFF38BDF8, 0xFFA855F7, 0xFFF97316
    };
    static String[] bottleNames() {   // the labels in the app's language (Words: English in the JVM tests)
        return of(
            "---",
            Words.s("oot_bottle_empty", "Empty"),
            Words.s("oot_bottle_red_potion", "Red Pot."),
            Words.s("oot_bottle_green_potion", "Grn Pot."),
            Words.s("oot_bottle_blue_potion", "Blu Pot."),
            Words.s("oot_bottle_fairy", "Fairy"),
            Words.s("oot_bottle_milk", "Milk"),
            Words.s("oot_bottle_blue_fire", "Blu Fire"),
            Words.s("oot_bottle_fish", "Fish"),
            Words.s("oot_bottle_bugs", "Bugs"),
            Words.s("oot_bottle_poe", "Poe"),
            Words.s("oot_bottle_big_poe", "Big Poe"),
            Words.s("oot_bottle_letter", "Letter")
        );
    }
    static final int[] BOTTLE_COLORS = {
        0xFF3A4A44, 0xFF94A3B8, 0xFFEF4444, 0xFF22C55E, 0xFF3B82F6,
        0xFFF472B6, 0xFFF8FAFC, 0xFF38BDF8, 0xFF60A5FA, 0xFFA3E635,
        0xFFC084FC, 0xFFD946EF, 0xFFFDE047
    };

    static final int NOTE_D4 = 0, NOTE_F4 = 1, NOTE_A4 = 2, NOTE_B4 = 3, NOTE_D5 = 4;
    static final String[] NOTE_PITCH_NAMES = {"D4", "F4", "A4", "B4", "D5"};
    static String[] songNames() {   // the labels in the app's language (Words: English in the JVM tests)
        return of(
            Words.s("oot_song_zelda", "Zelda's Lullaby"),
            Words.s("oot_song_epona", "Epona's Song"),
            Words.s("oot_song_saria", "Saria's Song"),
            Words.s("oot_song_sun", "Sun's Song"),
            Words.s("oot_song_time", "Song of Time"),
            Words.s("oot_song_storms", "Song of Storms"),
            Words.s("oot_song_minuet", "Minuet of Forest"),
            Words.s("oot_song_bolero", "Bolero of Fire"),
            Words.s("oot_song_serenade", "Serenade of Water"),
            Words.s("oot_song_nocturne", "Nocturne of Shadow"),
            Words.s("oot_song_requiem", "Requiem of Spirit"),
            Words.s("oot_song_prelude", "Prelude of Light")
        );
    }
    static final int[][] SONG_NOTES = {
        {NOTE_B4, NOTE_D5, NOTE_A4, NOTE_B4, NOTE_D5, NOTE_A4},
        {NOTE_D5, NOTE_B4, NOTE_A4, NOTE_D5, NOTE_B4, NOTE_A4},
        {NOTE_F4, NOTE_A4, NOTE_B4, NOTE_F4, NOTE_A4, NOTE_B4},
        {NOTE_A4, NOTE_F4, NOTE_D5, NOTE_A4, NOTE_F4, NOTE_D5},
        {NOTE_A4, NOTE_D4, NOTE_F4, NOTE_A4, NOTE_D4, NOTE_F4},
        {NOTE_D4, NOTE_F4, NOTE_D5, NOTE_D4, NOTE_F4, NOTE_D5},
        {NOTE_D4, NOTE_D5, NOTE_B4, NOTE_A4, NOTE_B4, NOTE_A4},
        {NOTE_F4, NOTE_D4, NOTE_F4, NOTE_D4, NOTE_A4, NOTE_F4, NOTE_A4, NOTE_F4},
        {NOTE_D4, NOTE_F4, NOTE_A4, NOTE_A4, NOTE_B4},
        {NOTE_B4, NOTE_A4, NOTE_A4, NOTE_D4, NOTE_B4, NOTE_A4, NOTE_F4},
        {NOTE_D4, NOTE_F4, NOTE_D4, NOTE_A4, NOTE_F4, NOTE_D4},
        {NOTE_D5, NOTE_A4, NOTE_D5, NOTE_A4, NOTE_B4, NOTE_D5}
    };
    static final int[] SONG_ACCENT_COLORS = {
        0xFF60A5FA, 0xFFF59E0B, 0xFF4ADE80, 0xFFFACC15,
        0xFF38BDF8, 0xFFA78BFA, 0xFF22C55E, 0xFFEF4444,
        0xFF06B6D4, 0xFFC084FC, 0xFFFB923C, 0xFFFDE047
    };

    int status = CompanionState.NONE;
    boolean canPlayOcarina, writeSafe, isChild, playingOcarina, ridingHorse, menuOpen, dialogueOpen, poseValid;
    int life, maxLife, magic, maxMagic, magicUnlocked;
    int rupees, walletCap, tokens, beans;
    int stonesMask, medallionsMask, songsMask;
    final int[] ammo = new int[6];
    final int[] ammoCap = new int[6];
    final int[] smallKeys = new int[8];
    final int[] bottles = new int[5];
    int trailCount, seqTag = -1, seqStep = -1, pinMask, dungeonIdx = -1, hostPage = -1;
    float posX, posY, posZ, yawDeg;
    final float[] trailX = new float[TRAIL_CAP];
    final float[] trailY = new float[TRAIL_CAP];
    final float[] trailZ = new float[TRAIL_CAP];
    final float[] pinX = new float[PIN_CAP];
    final float[] pinY = new float[PIN_CAP];
    final float[] pinZ = new float[PIN_CAP];
    final String[] pinLabels = new String[PIN_CAP];
    String sceneTitle = "Hyrule", subArea = "", equipWeapon = "", equipShield = "", equipArmor = "", equipBoot = "", cSlot2 = "", cSlot3 = "";

    private int manualTab = TAB_RADAR;
    private int zoomIdx = 1;
    private boolean headingUp = true;
    private boolean classicOcarina;
    private int orbitDx, orbitDy;

    void clearData() {
        status = CompanionState.NONE;
        canPlayOcarina = writeSafe = isChild = playingOcarina = ridingHorse = menuOpen = dialogueOpen = poseValid = false;
        life = maxLife = magic = maxMagic = magicUnlocked = rupees = walletCap = tokens = beans = 0;
        stonesMask = medallionsMask = songsMask = trailCount = pinMask = 0;
        seqTag = -1; seqStep = -1; dungeonIdx = -1; hostPage = -1;
        posX = posY = posZ = yawDeg = 0.0f;
        sceneTitle = "Hyrule"; subArea = "";
        equipWeapon = ""; equipShield = ""; equipArmor = ""; equipBoot = ""; cSlot2 = ""; cSlot3 = "";
        for (int i = 0; i < PIN_CAP; i++) pinLabels[i] = "";
    }

    /** Unpack native detail snapshot and advance 60-second 1-pixel AMOLED orbit. */
    boolean update(int[] ints, float[] flts, String text, long nowMs) {
        updateOrbit(nowMs);
        if (ints == null || ints.length < DETAIL_INTS || flts == null || flts.length < DETAIL_FLOATS) {
            clearData();
            return false;
        }
        status = ints[0];
        canPlayOcarina = ints[1] != 0;
        writeSafe = ints[2] != 0;
        isChild = ints[3] != 0;
        playingOcarina = ints[4] != 0;
        ridingHorse = ints[5] != 0;
        menuOpen = ints[6] != 0;
        dialogueOpen = ints[7] != 0;
        poseValid = ints[8] != 0;
        life = ints[9]; maxLife = ints[10];
        magic = ints[11]; maxMagic = ints[12]; magicUnlocked = ints[13];
        rupees = ints[14]; walletCap = ints[15]; tokens = ints[16]; beans = ints[17];
        stonesMask = ints[18]; medallionsMask = ints[19]; songsMask = ints[20];
        System.arraycopy(ints, 21, ammo, 0, 6);
        System.arraycopy(ints, 27, ammoCap, 0, 6);
        System.arraycopy(ints, 33, smallKeys, 0, 8);
        System.arraycopy(ints, 41, bottles, 0, 5);
        trailCount = Math.max(0, Math.min(TRAIL_CAP, ints[46]));
        seqTag = ints[47]; seqStep = ints[48]; pinMask = ints[49];
        dungeonIdx = ints[50]; hostPage = ints[51];
        unpackFloats(flts);
        unpackLines(text);
        return true;
    }

    private void unpackFloats(float[] flts) {
        posX = flts[0]; posY = flts[1]; posZ = flts[2]; yawDeg = flts[3];
        for (int i = 0; i < trailCount; i++) {
            trailX[i] = flts[4 + i * 3];
            trailY[i] = flts[4 + i * 3 + 1];
            trailZ[i] = flts[4 + i * 3 + 2];
        }
        for (int i = 0; i < PIN_CAP; i++) {
            pinX[i] = flts[388 + i * 3];
            pinY[i] = flts[388 + i * 3 + 1];
            pinZ[i] = flts[388 + i * 3 + 2];
        }
    }

    private void unpackLines(String text) {
        String[] lines = text == null ? new String[0] : text.split("\n", -1);
        sceneTitle = lineAt(lines, 0, "Hyrule");
        subArea = lineAt(lines, 1, "");
        equipWeapon = lineAt(lines, 2, "");
        equipShield = lineAt(lines, 3, "");
        equipArmor = lineAt(lines, 4, "");
        equipBoot = lineAt(lines, 5, "");
        cSlot2 = lineAt(lines, 6, "");
        cSlot3 = lineAt(lines, 7, "");
        for (int i = 0; i < PIN_CAP; i++) pinLabels[i] = lineAt(lines, 8 + i, "");
    }

    private static String lineAt(String[] lines, int idx, String fallback) {
        if (idx < 0 || idx >= lines.length || lines[idx].isEmpty()) return fallback;
        return lines[idx];
    }

    void updateOrbit(long nowMs) {
        long step = Math.floorMod(nowMs / 60_000L, 8L);
        final int[] xs = {0, 1, 1, 1, 0, -1, -1, -1};
        final int[] ys = {-1, -1, 0, 1, 1, 1, 0, -1};
        orbitDx = xs[(int) step];
        orbitDy = ys[(int) step];
    }

    int orbitDx() { return orbitDx; }
    int orbitDy() { return orbitDy; }

    void selectTab(int tab) { if (tab >= 0 && tab < TAB_COUNT) manualTab = tab; }
    int manualTab() { return manualTab; }

    int activeTab() {
        if (playingOcarina || hostPage == CompanionState.PAGE_OCARINA) return TAB_OCARINA;
        return manualTab;
    }

    float radiusMeters() { return ZOOM_RADII_M[zoomIdx]; }
    int cycleZoom() { zoomIdx = (zoomIdx + 1) % ZOOM_RADII_M.length; return zoomIdx; }
    boolean isHeadingUp() { return headingUp; }
    void toggleHeadingUp() { headingUp = !headingUp; }
    boolean isClassicOcarina() { return classicOcarina; }
    void toggleClassicOcarina() { classicOcarina = !classicOcarina; }

    boolean hasPin(int slot) { return slot >= 0 && slot < PIN_CAP && (pinMask & (1 << slot)) != 0; }
    boolean hasStone(int idx) { return idx >= 0 && idx < 3 && (stonesMask & (1 << idx)) != 0; }
    boolean hasMedallion(int idx) { return idx >= 0 && idx < 6 && (medallionsMask & (1 << idx)) != 0; }
    boolean hasSong(int idx) { return idx >= 0 && idx < 12 && (songsMask & (1 << idx)) != 0; }

    int activeDungeonKeys() { return (dungeonIdx >= 0 && dungeonIdx < 8) ? smallKeys[dungeonIdx] : -1; }

    int floorBand(float ptY) {
        float dy = ptY - posY;
        if (dy > FLOOR_BAND_M) return BAND_ABOVE;
        if (dy < -FLOOR_BAND_M) return BAND_BELOW;
        return BAND_LEVEL;
    }

    /** Project (ptX, ptZ) to normalized [-1, 1]^2 in out[0..1], with out[2]=1 if rim-clamped. */
    boolean projectXZ(float ptX, float ptZ, float[] out) {
        if (out == null || out.length < 3 || !Float.isFinite(ptX) || !Float.isFinite(ptZ)) return false;
        float r = radiusMeters();
        float dx = ptX - posX;
        float dz = ptZ - posZ;
        float rx = dx;
        float rz = dz;
        if (headingUp) {
            double rad = Math.toRadians(Float.isFinite(yawDeg) ? yawDeg : 0.0f);
            float c = (float) Math.cos(rad);
            float s = (float) Math.sin(rad);
            rx = dx * c + dz * s;
            rz = -dx * s + dz * c;
        }
        float nx = rx / r;
        float ny = rz / r;
        float mag = (float) Math.hypot(nx, ny);
        if (mag > 1.0f) {
            out[0] = nx / mag;
            out[1] = ny / mag;
            out[2] = 1.0f;
        } else {
            out[0] = nx;
            out[1] = ny;
            out[2] = 0.0f;
        }
        return true;
    }

    String formatPinDelta(int slot) {
        if (!hasPin(slot)) return "";
        int dist = (int) Math.round(Math.hypot(pinX[slot] - posX, pinZ[slot] - posZ));
        int band = floorBand(pinY[slot]);
        String mark = band > 0 ? "▲" : (band < 0 ? "▼" : "●");
        return dist + "m " + mark;
    }

    String noteButtonLabel(int note) {
        final String[] normal = {"L", "R", "A", "Y", "X"};
        final String[] classic = {"R", "B", "A", "Y", "X"};
        if (note < 0 || note >= 5) return "?";
        return classicOcarina ? classic[note] : normal[note];
    }

    int bottleColor(int kind) {
        return (kind >= 0 && kind < BOTTLE_COLORS.length) ? BOTTLE_COLORS[kind] : BOTTLE_COLORS[0];
    }

    static String bottleLabel(int kind) {
        String[] names = bottleNames();
        return (kind >= 0 && kind < names.length) ? names[kind] : "---";
    }

    static String cleanItemName(String raw, String fallback) {
        if (raw == null || raw.isEmpty()) return fallback;
        String s = raw.replace("Weapon_", "").replace("Sword_", "").replace("Shield_", "")
                .replace("Armor_", "").replace("Boot_", "").replace("Item_", "").replace('_', ' ');
        return s.trim().isEmpty() ? fallback : s.trim();
    }

    static int nextSkulltulaMilestone(int tokens) {
        final int[] tiers = {10, 20, 30, 40, 50, 100};
        for (int t : tiers) if (tokens < t) return t;
        return 100;
    }
}
