#pragma once
#include "platform/input.h"

namespace sudoku {

// Mouse click -> Tap (T008). Window-close sets *quitFlag; window-exposed
// sets *redrawFlag so the app shell can repaint.
class MouseTouch : public TouchInput {
public:
    MouseTouch(bool* quitFlag, bool* redrawFlag) : quit_(quitFlag), redraw_(redrawFlag) {}

    std::optional<Tap> waitForTap(int timeoutMs) override;

private:
    bool* quit_;
    bool* redraw_;
};

}  // namespace sudoku
