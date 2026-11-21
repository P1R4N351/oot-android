// SPDX-License-Identifier: MIT
package dev.nxnative.android;

import android.app.Presentation;
import android.content.Context;
import android.os.Bundle;
import android.view.Display;
import android.view.Window;
import android.view.WindowManager;

/**
 * Secondary-display {@link Presentation} (AYN Thor Display 4) configured with
 * {@code FLAG_NOT_FOCUSABLE | FLAG_KEEP_SCREEN_ON} so touch interactions on
 * the lower screen never steal gamepad input focus from Display 0.
 */
final class OotCompanionPresentation extends Presentation {
    private final OotCompanionView content;

    OotCompanionPresentation(Context ctx, Display display, OotCompanionView view) {
        super(ctx, display);
        this.content = view;
    }

    @Override protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Window w = getWindow();
        if (w != null) {
            w.addFlags(WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                    | WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        }
        setContentView(content);
    }
}
