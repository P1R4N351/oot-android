// SPDX-License-Identifier: MIT
package dev.nxnative.android;

/** Host JVM verification harness for {@link OotCompanionModel} (zero Android SDK needed). */
public final class OotCompanionJvmTest {
    private static void check(boolean cond, String msg) {
        if (!cond) throw new AssertionError(msg);
    }

    private static void testUnpackAutoTabAndOrbit() {
        OotCompanionModel m = new OotCompanionModel();
        int[] ints = new int[OotCompanionModel.DETAIL_INTS];
        float[] flts = new float[OotCompanionModel.DETAIL_FLOATS];

        ints[0] = CompanionState.READY;
        ints[1] = 0; // not holding Ocarina yet
        ints[2] = 1; // writeSafe
        ints[3] = 0; // Adult
        ints[4] = 0; // playingOcarina
        ints[8] = 1; // poseValid
        ints[14] = 245; ints[15] = 500; ints[16] = 23;
        ints[18] = 0x7; ints[19] = 0x2A; ints[20] = 0x25; // Stones, Medallions, Songs 0/2/5
        ints[24] = 18; ints[30] = 40; // Arrows
        ints[36] = 4; // Water Temple keys
        ints[41] = 5; // Bottle 0 -> Fairy
        ints[46] = 2; // 2 trail points
        ints[47] = -1; ints[48] = -1;
        ints[49] = 1; // pin slot 0 active
        ints[50] = 3; // Water Temple index
        ints[51] = -1;

        flts[0] = 10f; flts[1] = 20f; flts[2] = 30f; flts[3] = 90f;
        flts[4] = 5f; flts[5] = 25f; flts[6] = 30f;
        flts[7] = 10f; flts[8] = 20f; flts[9] = 30f;
        flts[388] = 16f; flts[389] = 25f; flts[390] = 38f;

        String txt = "Water Temple\nCentral Pillar\nWeapon_Sword_Master\nArmor_Zora\nArmor_Zora\nBoot_Iron\nBow\nHookshot\nChest Pin\n";
        check(m.update(ints, flts, txt, 0L), "valid update returns true");
        check("Water Temple".equals(m.sceneTitle) && m.activeDungeonKeys() == 4, "dungeon keys unpacked");
        check("Fairy".equals(OotCompanionModel.bottleLabel(m.bottles[0])), "bottle decoded");
        check(m.ammo[3] == 18 && m.ammoCap[3] == 40, "ammo unpacked");
        check("10m ▲".equals(m.formatPinDelta(0)), "pin delta formats 3D floor band");

        m.selectTab(OotCompanionModel.TAB_LOADOUT);
        check(m.activeTab() == OotCompanionModel.TAB_LOADOUT, "manual tab active");
        ints[1] = 1; ints[4] = 1; ints[51] = CompanionState.PAGE_OCARINA;
        m.update(ints, flts, txt, 60_000L);
        check(m.activeTab() == OotCompanionModel.TAB_OCARINA, "auto switches to Ocarina tab");

        ints[1] = 0; ints[4] = 0; ints[51] = OotCompanionModel.TAB_LOADOUT;
        m.update(ints, flts, txt, 120_000L);
        check(m.activeTab() == OotCompanionModel.TAB_LOADOUT, "restores manual tab when Ocarina exits");

        for (int step = 0; step < 8; step++) {
            m.updateOrbit(step * 60_000L);
            int x0 = m.orbitDx(), y0 = m.orbitDy();
            m.updateOrbit((step + 1) * 60_000L);
            int x1 = m.orbitDx(), y1 = m.orbitDy();
            check(Math.abs(x1 - x0) + Math.abs(y1 - y0) <= 1, "AMOLED orbit steps by <= 1 px");
        }
    }

    private static void testRadarProjectionAndOcarinaLabels() {
        OotCompanionModel m = new OotCompanionModel();
        int[] ints = new int[OotCompanionModel.DETAIL_INTS];
        float[] flts = new float[OotCompanionModel.DETAIL_FLOATS];
        ints[0] = CompanionState.READY;
        ints[8] = 1;
        flts[0] = 0f; flts[1] = 10f; flts[2] = 0f; flts[3] = 90f; // facing East, zoom default 80m
        m.update(ints, flts, "Hyrule Field\n\n\n\n\n\n\n\n", 0L);

        float[] out = new float[3];
        check(m.projectXZ(40f, 0f, out), "project east point");
        check(Math.abs(out[0]) < 1e-4f && Math.abs(out[1] + 0.5f) < 1e-4f && out[2] == 0f,
                "heading-up rotates East to straight ahead");
        m.toggleHeadingUp();
        check(m.projectXZ(40f, 0f, out) && Math.abs(out[0] - 0.5f) < 1e-4f && Math.abs(out[1]) < 1e-4f,
                "north-up maps +X East to right");
        check(m.cycleZoom() == 2 && m.radiusMeters() == 160f, "zoom cycles 80 -> 160");

        check("L".equals(m.noteButtonLabel(OotCompanionModel.NOTE_D4)), "Normal D4 = L");
        check("R".equals(m.noteButtonLabel(OotCompanionModel.NOTE_F4)), "Normal F4 = R");
        m.toggleClassicOcarina();
        check("R".equals(m.noteButtonLabel(OotCompanionModel.NOTE_D4)), "Classic D4 = R");
        check("B".equals(m.noteButtonLabel(OotCompanionModel.NOTE_F4)), "Classic F4 = B");
        check(!m.update(null, flts, "", 0L) && m.status == CompanionState.NONE, "null ints clears model");
    }

    public static void main(String[] args) {
        testUnpackAutoTabAndOrbit();
        testRadarProjectionAndOcarinaLabels();
        System.out.println("OotCompanionJvmTest: PASS");
    }
}
