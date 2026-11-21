// SPDX-License-Identifier: MIT
package dev.nxnative.android;

import android.content.Context;
import android.hardware.display.DisplayManager;
import android.os.Handler;
import android.os.SystemClock;
import android.view.Display;
import android.view.Gravity;
import android.view.View;
import android.widget.FrameLayout;

/**
 * Manages AYN Thor Display 4 {@link OotCompanionPresentation}, optional single-screen
 * companion drawer overlay, DisplayListener attach/detach, and 20 Hz UI refresh.
 */
final class OotCompanionController implements DisplayManager.DisplayListener {
    private static final String OOT_TITLE_ID = "0400486024DEC000";
    private static final long POLL_MS = 50L;

    private final GameActivity activity;
    private final FrameLayout rootView;
    private final Handler ui;
    private final DisplayManager dm;
    private final OotCompanionModel model = new OotCompanionModel();
    private final int[] ints = new int[OotCompanionModel.DETAIL_INTS];
    private final float[] flts = new float[OotCompanionModel.DETAIL_FLOATS];

    private OotCompanionView presView;
    private OotCompanionPresentation presentation;
    private OotCompanionView drawerView;
    private boolean drawerOpen;
    private boolean started, resumed, closed;
    private final Runnable pollTask = new Runnable() {
        @Override public void run() {
            if (closed || !resumed || !anyVisible()) return;
            refreshNow();
            ui.postDelayed(this, POLL_MS);
        }
    };

    static OotCompanionController create(GameActivity act, FrameLayout root, Handler ui, String titleId) {
        if (!OOT_TITLE_ID.equals(titleId)) return null;
        return new OotCompanionController(act, root, ui);
    }

    private OotCompanionController(GameActivity act, FrameLayout root, Handler ui) {
        this.activity = act;
        this.rootView = root;
        this.ui = ui;
        this.dm = (DisplayManager) act.getSystemService(Context.DISPLAY_SERVICE);
        this.drawerOpen = Prefs.companionDrawer(act, OOT_TITLE_ID);
        if (dm != null) dm.registerDisplayListener(this, ui);
        ensurePresentation();
        applyDrawerVisibility();
    }

    private OotCompanionView.Actions makeActions() {
        return new OotCompanionView.Actions() {
            @Override public void onSelectTab(int tab) { Native.companionSetPage(tab); }
            @Override public void onPlaySong(int idx) { Native.companionPlaySong(idx, model.isClassicOcarina()); }
            @Override public void onTapNote(int note) { Native.companionTapNote(note, model.isClassicOcarina()); }
            @Override public void onAbortSeq() { Native.companionAbortSeq(); }
            @Override public void onPinAdd() { Native.companionPin(0, -1, null); refreshNow(); }
            @Override public void onPinClear() { Native.companionPin(2, -1, null); refreshNow(); }
            @Override public void onPinRemove(int slot) { Native.companionPin(1, slot, null); refreshNow(); }
            @Override public void onCloseDrawer() { setDrawerOpen(false); }
        };
    }

    private Display pickSecondaryDisplay() {
        if (dm == null) return null;
        Display[] all = dm.getDisplays();
        Display fallback = null;
        for (Display d : all) {
            if (d == null || d.getDisplayId() == Display.DEFAULT_DISPLAY) continue;
            if ((d.getFlags() & Display.FLAG_PRIVATE) != 0) continue;
            if (d.getDisplayId() == 4) return d;
            if ((d.getFlags() & Display.FLAG_PRESENTATION) != 0) return d;
            if (fallback == null) fallback = d;
        }
        return fallback;
    }

    private void ensurePresentation() {
        if (closed) return;
        Display target = pickSecondaryDisplay();
        if (target == null) {
            dismissPresentation();
            return;
        }
        if (presentation != null && presentation.getDisplay() != null
                && presentation.getDisplay().getDisplayId() == target.getDisplayId()) {
            return;
        }
        dismissPresentation();
        presView = new OotCompanionView(activity, model, false, makeActions());
        presentation = new OotCompanionPresentation(activity, target, presView);
        if (started) presentation.show();
    }

    private void dismissPresentation() {
        if (presentation != null) {
            try { presentation.dismiss(); } catch (Throwable ignored) {}
            presentation = null;
            presView = null;
        }
    }

    private void applyDrawerVisibility() {
        if (drawerOpen) {
            if (drawerView == null && rootView != null) {
                drawerView = new OotCompanionView(activity, model, true, makeActions());
                int rootW = rootView.getWidth() > 0 ? rootView.getWidth()
                        : activity.getResources().getDisplayMetrics().widthPixels;
                int dw = Math.max(320, Math.round(rootW * 0.40f));
                FrameLayout.LayoutParams lp = new FrameLayout.LayoutParams(
                        dw, FrameLayout.LayoutParams.MATCH_PARENT, Gravity.START);
                View panel = activity.panelView();
                int idx = panel != null ? Math.max(0, rootView.indexOfChild(panel)) : rootView.getChildCount();
                rootView.addView(drawerView, idx, lp);
            }
            if (drawerView != null) drawerView.setVisibility(View.VISIBLE);
        } else if (drawerView != null) {
            drawerView.setVisibility(View.GONE);
        }
        schedulePoll();
    }

    void setDrawerOpen(boolean on) {
        drawerOpen = on;
        Prefs.companionDrawer(activity, OOT_TITLE_ID, on);
        applyDrawerVisibility();
        if (on) refreshNow();
    }

    boolean isDrawerOpen() { return drawerOpen; }

    private boolean anyVisible() {
        return (presentation != null && started) || drawerOpen;
    }

    private void schedulePoll() {
        ui.removeCallbacks(pollTask);
        if (!closed && resumed && anyVisible()) ui.post(pollTask);
    }

    private void refreshNow() {
        String text = Native.companionDetail(ints, flts);
        if (text != null) model.update(ints, flts, text, SystemClock.elapsedRealtime());
        else model.updateOrbit(SystemClock.elapsedRealtime());
        if (presView != null) presView.invalidate();
        if (drawerView != null && drawerOpen) drawerView.invalidate();
    }

    void onStart() {
        started = true;
        ensurePresentation();
        if (presentation != null) presentation.show();
        schedulePoll();
    }

    void onResume() {
        resumed = true;
        schedulePoll();
    }

    void onPause() {
        resumed = false;
        ui.removeCallbacks(pollTask);
        Native.companionAbortSeq();
    }

    void onStop() {
        started = false;
        if (presentation != null) presentation.hide();
        schedulePoll();
    }

    void close() {
        if (closed) return;
        closed = true;
        resumed = false;
        started = false;
        ui.removeCallbacks(pollTask);
        Native.companionAbortSeq();
        dismissPresentation();
        if (drawerView != null && rootView != null) {
            rootView.removeView(drawerView);
            drawerView = null;
        }
        if (dm != null) dm.unregisterDisplayListener(this);
    }

    @Override public void onDisplayAdded(int id) { ensurePresentation(); schedulePoll(); }
    @Override public void onDisplayRemoved(int id) { ensurePresentation(); schedulePoll(); }
    @Override public void onDisplayChanged(int id) { ensurePresentation(); }
}
