// SPDX-License-Identifier: MIT
package dev.nxnative.android;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.RectF;
import android.graphics.Typeface;
import android.view.MotionEvent;
import android.view.View;

/**
 * Procedural obsidian/emerald/brass Canvas companion HUD for Switch 2 OoT.
 * Renders the 4 zero-pause tabs (3D Breadcrumb Radar, Gear &amp; Bottles, Quest Altar,
 * and Interactive Ocarina Songbook + 5-Note Touch Pad) with zero Nintendo assets.
 */
final class OotCompanionView extends View {
    interface Actions {
        void onSelectTab(int tab);
        void onPlaySong(int songIdx);
        void onTapNote(int noteIdx);
        void onAbortSeq();
        void onPinAdd();
        void onPinClear();
        void onPinRemove(int slot);
        void onCloseDrawer();
    }

    private static final int ACT_TAB = 1, ACT_PLAY_SONG = 2, ACT_TAP_NOTE = 3, ACT_ABORT = 4;
    private static final int ACT_ZOOM = 5, ACT_HEADING = 6, ACT_PIN_ADD = 7, ACT_PIN_CLEAR = 8;
    private static final int ACT_PIN_DEL = 9, ACT_OCARINA_MODE = 10, ACT_CLOSE_DRAWER = 11;
    private static final int MAX_HITS = 40;

    private static final int BG_DARK = 0xFF0B1210;
    private static final int BG_PANEL = 0xFF121E1A;
    private static final int BG_CARD = 0xFF172822;
    private static final int EMERALD = 0xFF2DE2A6;
    private static final int EMERALD_DIM = 0xFF185E47;
    private static final int BRASS = 0xFFD4AF37;
    private static final int SKY_UP = 0xFF38BDF8;
    private static final int AMBER_DN = 0xFFF59E0B;
    private static final int CRIMSON = 0xFFEF4444;
    private static final int TXT_HI = 0xFFF1F5F9;
    private static final int TXT_DIM = 0xFF94A3B8;

    private final OotCompanionModel model;
    private final boolean drawerMode;
    private final Actions actions;
    private final Paint fillP = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final Paint strokeP = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final Paint textP = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final RectF tmpR = new RectF();
    private final Path arrowPath = new Path();
    private final float[] proj = new float[3];
    private final RectF[] hitRects = new RectF[MAX_HITS];
    private final int[] hitAct = new int[MAX_HITS];
    private final int[] hitArg = new int[MAX_HITS];
    private int hitCount;

    OotCompanionView(Context ctx, OotCompanionModel model, boolean drawerMode, Actions actions) {
        super(ctx);
        this.model = model;
        this.drawerMode = drawerMode;
        this.actions = actions;
        setFocusable(false);
        setFocusableInTouchMode(false);
        strokeP.setStyle(Paint.Style.STROKE);
        textP.setTypeface(Typeface.create(Typeface.SANS_SERIF, Typeface.NORMAL));
        for (int i = 0; i < MAX_HITS; i++) hitRects[i] = new RectF();
    }

    private void addHit(float l, float t, float r, float b, int act, int arg) {
        if (hitCount >= MAX_HITS) return;
        hitRects[hitCount].set(l, t, r, b);
        hitAct[hitCount] = act;
        hitArg[hitCount] = arg;
        hitCount++;
    }

    @Override protected void onDraw(Canvas c) {
        super.onDraw(c);
        hitCount = 0;
        float w = getWidth(), h = getHeight();
        if (w < 40 || h < 40) return;
        fillP.setColor(BG_DARK);
        c.drawRect(0, 0, w, h, fillP);
        c.save();
        c.translate(model.orbitDx(), model.orbitDy());
        float dp = Math.min(w, h) / 420f;
        if (dp < 0.75f) dp = 0.75f;
        float pad = 12f * dp;
        float bodyTop = drawHeader(c, w, pad, dp);
        if (model.status == CompanionState.NONE || model.status == CompanionState.STARTING) {
            drawCenteredBanner(c, w, bodyTop, h, "Waiting for Hyrule memory waist...");
        } else if (model.status == CompanionState.TITLE) {
            drawCenteredBanner(c, w, bodyTop, h, "Title Screen — Select a Quest File");
        } else if (model.status == CompanionState.LOADING) {
            drawCenteredBanner(c, w, bodyTop, h, "Area Transitioning...");
        } else {
            int tab = model.activeTab();
            if (tab == OotCompanionModel.TAB_RADAR) drawRadarTab(c, pad, bodyTop, w - 2f * pad, h - bodyTop - pad, dp);
            else if (tab == OotCompanionModel.TAB_LOADOUT) drawGearTab(c, pad, bodyTop, w - 2f * pad, h - bodyTop - pad, dp);
            else if (tab == OotCompanionModel.TAB_QUEST) drawAltarTab(c, pad, bodyTop, w - 2f * pad, h - bodyTop - pad, dp);
            else drawOcarinaTab(c, pad, bodyTop, w - 2f * pad, h - bodyTop - pad, dp);
        }
        c.restore();
    }

    private float drawHeader(Canvas c, float w, float pad, float dp) {
        float barH = 44f * dp;
        tmpR.set(pad, pad, w - pad, pad + barH);
        fillP.setColor(BG_PANEL);
        c.drawRoundRect(tmpR, 10f * dp, 10f * dp, fillP);
        textP.setColor(TXT_HI);
        textP.setTypeface(Typeface.DEFAULT_BOLD);
        textP.setTextSize(14f * dp);
        String title = model.sceneTitle + (model.isChild ? " [CHILD]" : " [ADULT]");
        c.drawText(title, pad + 10f * dp, pad + 18f * dp, textP);
        textP.setTypeface(Typeface.DEFAULT);
        textP.setColor(TXT_DIM);
        textP.setTextSize(11f * dp);
        int keys = model.activeDungeonKeys();
        String sub = model.subArea + "  |  " + model.rupees + "R  |  " + model.tokens + " GS"
                + (keys >= 0 ? "  |  " + keys + " Keys" : "");
        c.drawText(sub, pad + 10f * dp, pad + 35f * dp, textP);
        if (drawerMode) {
            float cr = w - pad - 6f * dp;
            drawPill(c, cr - 28f * dp, pad + 7f * dp, cr, pad + barH - 7f * dp, "×", CRIMSON, TXT_HI, dp);
            addHit(cr - 32f * dp, pad, w - pad, pad + barH, ACT_CLOSE_DRAWER, 0);
        }
        float tabTop = pad + barH + 8f * dp;
        float tabH = 32f * dp;
        float tabW = (w - 2f * pad - 3f * 6f * dp) / OotCompanionModel.TAB_COUNT;
        int active = model.activeTab();
        for (int i = 0; i < OotCompanionModel.TAB_COUNT; i++) {
            float l = pad + i * (tabW + 6f * dp);
            boolean sel = i == active;
            drawPill(c, l, tabTop, l + tabW, tabTop + tabH, OotCompanionModel.TAB_LABELS[i],
                    sel ? EMERALD : BG_CARD, sel ? BG_DARK : TXT_HI, dp);
            addHit(l, tabTop, l + tabW, tabTop + tabH, ACT_TAB, i);
        }
        return tabTop + tabH + 10f * dp;
    }

    private void drawRadarTab(Canvas c, float x, float y, float w, float h, float dp) {
        boolean wide = w >= h * 1.15f;
        float discSize = wide ? Math.min(h - 46f * dp, w * 0.56f) : Math.min(w * 0.72f, h * 0.54f);
        float cx = wide ? (x + discSize * 0.5f + 6f * dp) : (x + w * 0.5f);
        float cy = y + discSize * 0.5f + 4f * dp;
        float rad = Math.max(40f, discSize * 0.48f);
        fillP.setColor(BG_PANEL);
        c.drawCircle(cx, cy, rad, fillP);
        strokeP.setColor(EMERALD_DIM);
        strokeP.setStrokeWidth(1.5f * dp);
        c.drawCircle(cx, cy, rad, strokeP);
        c.drawCircle(cx, cy, rad * 0.5f, strokeP);
        drawCardinalN(c, cx, cy, rad, dp);
        drawTrailAndPins(c, cx, cy, rad, dp);
        drawLinkHeading(c, cx, cy, dp);
        float ctrlY = wide ? (y + h - 36f * dp) : (cy + rad + 12f * dp);
        float ctrlW = wide ? (discSize + 8f * dp) : w;
        drawRadarControls(c, x, ctrlY, ctrlW, dp);
        float listX = wide ? (x + discSize + 18f * dp) : x;
        float listY = wide ? y : (ctrlY + 42f * dp);
        float listW = wide ? (w - discSize - 18f * dp) : w;
        float listH = y + h - listY;
        drawPinList(c, listX, listY, listW, listH, dp);
    }

    private void drawCardinalN(Canvas c, float cx, float cy, float rad, float dp) {
        double a = model.isHeadingUp() ? Math.toRadians(-model.yawDeg) : 0.0;
        float nx = cx + (float) Math.sin(a) * (rad - 12f * dp);
        float ny = cy - (float) Math.cos(a) * (rad - 12f * dp);
        textP.setColor(BRASS);
        textP.setTypeface(Typeface.DEFAULT_BOLD);
        textP.setTextSize(11f * dp);
        c.drawText("N", nx - 4f * dp, ny + 4f * dp, textP);
    }

    private void drawTrailAndPins(Canvas c, float cx, float cy, float rad, float dp) {
        if (!model.poseValid) return;
        float lx = 0f, ly = 0f;
        boolean havePrev = false;
        strokeP.setStrokeWidth(2.5f * dp);
        for (int i = 0; i < model.trailCount; i++) {
            if (!model.projectXZ(model.trailX[i], model.trailZ[i], proj)) continue;
            float px = cx + proj[0] * rad, py = cy + proj[1] * rad;
            int band = model.floorBand(model.trailY[i]);
            int col = band > 0 ? SKY_UP : (band < 0 ? AMBER_DN : EMERALD);
            if (havePrev && proj[2] == 0f) {
                strokeP.setColor((col & 0x00FFFFFF) | 0xAA000000);
                c.drawLine(lx, ly, px, py, strokeP);
            }
            fillP.setColor(col);
            c.drawCircle(px, py, (proj[2] > 0f ? 2.0f : 3.2f) * dp, fillP);
            lx = px; ly = py; havePrev = proj[2] == 0f;
        }
        for (int s = 0; s < OotCompanionModel.PIN_CAP; s++) {
            if (!model.hasPin(s) || !model.projectXZ(model.pinX[s], model.pinZ[s], proj)) continue;
            float px = cx + proj[0] * rad, py = cy + proj[1] * rad;
            int band = model.floorBand(model.pinY[s]);
            fillP.setColor(band > 0 ? SKY_UP : (band < 0 ? AMBER_DN : BRASS));
            c.drawCircle(px, py, 7f * dp, fillP);
            textP.setColor(BG_DARK);
            textP.setTextSize(9f * dp);
            c.drawText(String.valueOf(s + 1), px - 2.5f * dp, py + 3.2f * dp, textP);
        }
    }

    private void drawLinkHeading(Canvas c, float cx, float cy, float dp) {
        float rotDeg = model.isHeadingUp() ? 0f : model.yawDeg;
        c.save();
        c.translate(cx, cy);
        c.rotate(rotDeg);
        arrowPath.reset();
        arrowPath.moveTo(0f, -9f * dp);
        arrowPath.lineTo(6.5f * dp, 7f * dp);
        arrowPath.lineTo(0f, 3.5f * dp);
        arrowPath.lineTo(-6.5f * dp, 7f * dp);
        arrowPath.close();
        fillP.setColor(TXT_HI);
        c.drawPath(arrowPath, fillP);
        c.restore();
    }

    private void drawRadarControls(Canvas c, float x, float y, float w, float dp) {
        String[] labels = {
            ((int) model.radiusMeters()) + "m",
            model.isHeadingUp() ? "H-Up" : "N-Up",
            "+ Pin",
            "Clear"
        };
        int[] acts = {ACT_ZOOM, ACT_HEADING, ACT_PIN_ADD, ACT_PIN_CLEAR};
        int[] cols = {BG_CARD, BG_CARD, EMERALD_DIM, BG_CARD};
        float bw = (w - 3f * 6f * dp) / 4f;
        for (int i = 0; i < 4; i++) {
            float l = x + i * (bw + 6f * dp);
            drawPill(c, l, y, l + bw, y + 32f * dp, labels[i], cols[i], TXT_HI, dp);
            addHit(l, y, l + bw, y + 32f * dp, acts[i], 0);
        }
    }

    private void drawPinList(Canvas c, float x, float y, float w, float h, float dp) {
        if (w < 60f || h < 40f) return;
        tmpR.set(x, y, x + w, y + h);
        fillP.setColor(BG_PANEL);
        c.drawRoundRect(tmpR, 8f * dp, 8f * dp, fillP);
        float rowH = 26f * dp;
        float cy = y + 20f * dp;
        int drawn = 0;
        for (int s = 0; s < OotCompanionModel.PIN_CAP && cy < y + h - 8f * dp; s++) {
            if (!model.hasPin(s)) continue;
            textP.setColor(BRASS);
            textP.setTextSize(11f * dp);
            String row = (s + 1) + ". " + model.pinLabels[s] + "   " + model.formatPinDelta(s);
            c.drawText(row, x + 10f * dp, cy, textP);
            drawPill(c, x + w - 42f * dp, cy - 13f * dp, x + w - 8f * dp, cy + 5f * dp,
                    "Del", BG_CARD, TXT_DIM, dp);
            addHit(x + w - 46f * dp, cy - 16f * dp, x + w - 6f * dp, cy + 8f * dp, ACT_PIN_DEL, s);
            cy += rowH;
            drawn++;
        }
        if (drawn == 0) {
            textP.setColor(TXT_DIM);
            textP.setTextSize(11f * dp);
            c.drawText("Tap '+ Pin' to mark chests or doors in 3D", x + 10f * dp, y + 24f * dp, textP);
        }
    }

    private void drawGearTab(Canvas c, float x, float y, float w, float h, float dp) {
        float rowH = 38f * dp;
        String[] gLabels = {"Sword", "Shield", "Tunic", "Boots"};
        String[] gVals = {
            OotCompanionModel.cleanItemName(model.equipWeapon, "---"),
            OotCompanionModel.cleanItemName(model.equipShield, "---"),
            OotCompanionModel.cleanItemName(model.equipArmor, "Kokiri"),
            OotCompanionModel.cleanItemName(model.equipBoot, "Kokiri")
        };
        float gw = (w - 3f * 6f * dp) / 4f;
        for (int i = 0; i < 4; i++) {
            float l = x + i * (gw + 6f * dp);
            boolean hot = (i >= 2) && !gVals[i].contains("Kokiri");
            drawCard(c, l, y, gw, rowH, gLabels[i], gVals[i], hot ? BRASS : TXT_HI, dp);
        }
        float botY = y + rowH + 10f * dp;
        float bw = (w - 4f * 6f * dp) / 5f;
        for (int i = 0; i < 5; i++) {
            float l = x + i * (bw + 6f * dp);
            int k = model.bottles[i];
            drawCard(c, l, botY, bw, rowH, "Bottle " + (i + 1),
                    OotCompanionModel.bottleLabel(k), model.bottleColor(k), dp);
        }
        float supY = botY + rowH + 12f * dp;
        float sw = (w - 2f * 8f * dp) / 3f;
        float sh = Math.min(34f * dp, Math.max(24f * dp, (y + h - supY - 24f * dp) / 2f));
        for (int i = 0; i < 6; i++) {
            float lx = x + (i % 3) * (sw + 8f * dp);
            float ly = supY + (i / 3) * (sh + 6f * dp);
            int cur = model.ammo[i], cap = Math.max(cur, model.ammoCap[i]);
            drawGauge(c, lx, ly, sw, sh, OotCompanionModel.AMMO_LABELS[i], cur, cap, dp);
        }
    }

    private void drawAltarTab(Canvas c, float x, float y, float w, float h, float dp) {
        float sw = (w - 2f * 6f * dp) / 3f;
        float sh = 34f * dp;
        for (int i = 0; i < 3; i++) {
            boolean own = model.hasStone(i);
            float l = x + i * (sw + 6f * dp);
            drawPill(c, l, y, l + sw, y + sh, OotCompanionModel.STONE_NAMES[i],
                    own ? EMERALD_DIM : BG_CARD, own ? TXT_HI : TXT_DIM, dp);
        }
        float my = y + sh + 8f * dp;
        float mw = (w - 2f * 6f * dp) / 3f;
        for (int i = 0; i < 6; i++) {
            boolean own = model.hasMedallion(i);
            float l = x + (i % 3) * (mw + 6f * dp);
            float t = my + (i / 3) * (30f * dp);
            int bg = own ? ((OotCompanionModel.MEDALLION_COLORS[i] & 0x00FFFFFF) | 0x44000000) : BG_CARD;
            int fg = own ? OotCompanionModel.MEDALLION_COLORS[i] : TXT_DIM;
            drawPill(c, l, t, l + mw, t + 25f * dp, OotCompanionModel.MEDALLION_NAMES[i], bg, fg, dp);
        }
        float ky = my + 66f * dp;
        float kw = (w - 6f * dp) / 2f;
        for (int i = 0; i < 8 && ky + (i / 2) * 20f * dp < y + h - 24f * dp; i++) {
            float l = x + (i % 2) * (kw + 6f * dp);
            float t = ky + (i / 2) * 20f * dp;
            textP.setColor(model.dungeonIdx == i ? BRASS : TXT_DIM);
            textP.setTextSize(11f * dp);
            c.drawText(OotCompanionModel.DUNGEON_NAMES[i] + ": " + model.smallKeys[i], l + 4f * dp, t + 14f * dp, textP);
        }
        int nextTier = OotCompanionModel.nextSkulltulaMilestone(model.tokens);
        drawGauge(c, x, y + h - 24f * dp, w, 24f * dp, "Gold Skulltulas", model.tokens, nextTier, dp);
    }

    private void drawOcarinaTab(Canvas c, float x, float y, float w, float h, float dp) {
        float topH = 28f * dp;
        boolean ready = model.canPlayOcarina;
        drawPill(c, x, y, x + w * 0.56f, y + topH,
                ready ? "READY: TAP SONG TO PLAY" : "HOLD OCARINA IN GAME",
                ready ? EMERALD_DIM : BG_CARD, ready ? EMERALD : AMBER_DN, dp);
        float mL = x + w * 0.58f;
        float stopW = (model.seqTag >= 0) ? 54f * dp : 0f;
        drawPill(c, mL, y, x + w - stopW - (stopW > 0 ? 4f * dp : 0f), y + topH,
                model.isClassicOcarina() ? "Classic Pad" : "Normal Pad", BG_CARD, BRASS, dp);
        addHit(mL, y, x + w - stopW, y + topH, ACT_OCARINA_MODE, 0);
        if (stopW > 0f) {
            drawPill(c, x + w - stopW, y, x + w, y + topH, "STOP", CRIMSON, TXT_HI, dp);
            addHit(x + w - stopW, y, x + w, y + topH, ACT_ABORT, 0);
        }
        float padH = 38f * dp;
        float gridTop = y + topH + 8f * dp;
        float gridH = Math.max(60f * dp, h - topH - padH - 16f * dp);
        int cols = w >= 420f ? 3 : 2;
        int rows = (12 + cols - 1) / cols;
        float cw = (w - (cols - 1) * 6f * dp) / cols;
        float ch = (gridH - (rows - 1) * 5f * dp) / rows;
        for (int i = 0; i < 12; i++) {
            float cl = x + (i % cols) * (cw + 6f * dp);
            float ct = gridTop + (i / cols) * (ch + 5f * dp);
            drawSongTile(c, i, cl, ct, cw, ch, dp);
        }
        drawNotePad(c, x, y + h - padH, w, padH, dp);
    }

    private void drawSongTile(Canvas c, int idx, float x, float y, float w, float h, float dp) {
        boolean owned = model.hasSong(idx);
        boolean playing = model.seqTag == idx;
        tmpR.set(x, y, x + w, y + h);
        fillP.setColor(playing ? EMERALD_DIM : (owned ? BG_PANEL : BG_CARD));
        c.drawRoundRect(tmpR, 7f * dp, 7f * dp, fillP);
        if (playing) {
            strokeP.setColor(EMERALD);
            strokeP.setStrokeWidth(2f * dp);
            c.drawRoundRect(tmpR, 7f * dp, 7f * dp, strokeP);
        }
        textP.setColor(owned ? TXT_HI : TXT_DIM);
        textP.setTextSize(10f * dp);
        c.drawText(OotCompanionModel.SONG_NAMES[idx], x + 6f * dp, y + 13f * dp, textP);
        int[] notes = OotCompanionModel.SONG_NOTES[idx];
        float staffY = y + h * 0.56f;
        float stepX = Math.min(16f * dp, (w - 14f * dp) / Math.max(1, notes.length));
        for (int n = 0; n < notes.length; n++) {
            float nx = x + 8f * dp + n * stepX;
            float ny = staffY + (2 - notes[n]) * 2.8f * dp;
            boolean lit = playing && model.seqStep == n;
            fillP.setColor(lit ? BRASS : (owned ? OotCompanionModel.SONG_ACCENT_COLORS[idx] : TXT_DIM));
            c.drawCircle(nx, ny, (lit ? 3.5f : 2.2f) * dp, fillP);
        }
        if (owned) addHit(x, y, x + w, y + h, ACT_PLAY_SONG, idx);
    }

    private void drawNotePad(Canvas c, float x, float y, float w, float h, float dp) {
        float bw = (w - 4f * 6f * dp) / 5f;
        for (int n = 0; n < 5; n++) {
            float l = x + n * (bw + 6f * dp);
            String txt = model.noteButtonLabel(n) + " (" + OotCompanionModel.NOTE_PITCH_NAMES[n] + ")";
            boolean active = model.seqTag == 100 + n;
            drawPill(c, l, y, l + bw, y + h, txt, active ? EMERALD : BG_CARD, active ? BG_DARK : TXT_HI, dp);
            addHit(l, y, l + bw, y + h, ACT_TAP_NOTE, n);
        }
    }

    private void drawPill(Canvas c, float l, float t, float r, float b, String txt, int bg, int fg, float dp) {
        tmpR.set(l, t, r, b);
        fillP.setColor(bg);
        c.drawRoundRect(tmpR, 7f * dp, 7f * dp, fillP);
        textP.setColor(fg);
        textP.setTextSize(10.5f * dp);
        textP.setTypeface(Typeface.DEFAULT_BOLD);
        float tw = textP.measureText(txt);
        c.drawText(txt, (l + r - tw) * 0.5f, (t + b) * 0.5f + 3.8f * dp, textP);
        textP.setTypeface(Typeface.DEFAULT);
    }

    private void drawCard(Canvas c, float x, float y, float w, float h, String k, String v, int vc, float dp) {
        tmpR.set(x, y, x + w, y + h);
        fillP.setColor(BG_PANEL);
        c.drawRoundRect(tmpR, 7f * dp, 7f * dp, fillP);
        textP.setColor(TXT_DIM);
        textP.setTextSize(9f * dp);
        c.drawText(k, x + 6f * dp, y + 12f * dp, textP);
        textP.setColor(vc);
        textP.setTextSize(11f * dp);
        c.drawText(v, x + 6f * dp, y + 29f * dp, textP);
    }

    private void drawGauge(Canvas c, float x, float y, float w, float h, String label, int cur, int cap, float dp) {
        tmpR.set(x, y, x + w, y + h);
        fillP.setColor(BG_CARD);
        c.drawRoundRect(tmpR, 6f * dp, 6f * dp, fillP);
        float frac = cap > 0 ? Math.max(0f, Math.min(1f, (float) cur / cap)) : 0f;
        tmpR.set(x, y, x + w * frac, y + h);
        fillP.setColor(EMERALD_DIM);
        c.drawRoundRect(tmpR, 6f * dp, 6f * dp, fillP);
        textP.setColor(TXT_HI);
        textP.setTextSize(10.5f * dp);
        c.drawText(label + ": " + cur + "/" + cap, x + 8f * dp, y + h * 0.5f + 3.8f * dp, textP);
    }

    private void drawCenteredBanner(Canvas c, float w, float top, float h, String msg) {
        textP.setColor(TXT_DIM);
        textP.setTextSize(14f);
        float tw = textP.measureText(msg);
        c.drawText(msg, Math.max(12f, (w - tw) * 0.5f), (top + h) * 0.5f, textP);
    }

    @Override public boolean onTouchEvent(MotionEvent ev) {
        if (ev.getActionMasked() != MotionEvent.ACTION_UP) return ev.getActionMasked() == MotionEvent.ACTION_DOWN;
        float x = ev.getX(), y = ev.getY();
        for (int i = hitCount - 1; i >= 0; i--) {
            if (hitRects[i].contains(x, y)) { dispatchHit(hitAct[i], hitArg[i]); return true; }
        }
        return true;
    }

    private void dispatchHit(int act, int arg) {
        if (act == ACT_TAB) { model.selectTab(arg); actions.onSelectTab(arg); }
        else if (act == ACT_PLAY_SONG) actions.onPlaySong(arg);
        else if (act == ACT_TAP_NOTE) actions.onTapNote(arg);
        else if (act == ACT_ABORT) actions.onAbortSeq();
        else if (act == ACT_ZOOM) model.cycleZoom();
        else if (act == ACT_HEADING) model.toggleHeadingUp();
        else if (act == ACT_OCARINA_MODE) model.toggleClassicOcarina();
        else if (act == ACT_PIN_ADD) actions.onPinAdd();
        else if (act == ACT_PIN_CLEAR) actions.onPinClear();
        else if (act == ACT_PIN_DEL) actions.onPinRemove(arg);
        else if (act == ACT_CLOSE_DRAWER) actions.onCloseDrawer();
        invalidate();
    }
}
