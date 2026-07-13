#pragma once
#include <string>

#include "platform/renderer.h"

namespace sudoku::ui {

// Grayscale-first styles and DPI-relative metrics (Constitution II, FR-015).
// All geometry derives from DisplayInfo; nothing is hardcoded in pixels.
struct Theme {
    int dpi = 300;
    bool color = false;

    int touchTargetPx = 0;  // >= 9 mm (T009): ~106 px @300 dpi, ~75 px @212 dpi
    int pad = 0;            // outer screen margin
    int gap = 0;            // spacing between controls

    int titlePx = 0;   // screen titles
    int textPx = 0;    // buttons, labels
    int smallPx = 0;   // secondary text, timer
    int digitPx = 0;   // board digits (relative to cell size, set by BoardView)

    int thinLine = 0;   // board minor grid line
    int thickLine = 0;  // board 3x3 boundary

    std::string fontPath, fontBoldPath;

    int mm(double millimetres) const {
        int px = static_cast<int>(millimetres * dpi / 25.4 + 0.5);
        return px < 1 ? 1 : px;
    }
};

Theme makeTheme(const DisplayInfo& d, const std::string& assetsDir);

}  // namespace sudoku::ui
