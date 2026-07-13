#pragma once
#include "platform/input.h"

namespace sudoku::ui {

class App;

// A full-screen view. draw() renders everything into the canvas; the app shell
// full-flushes after screen transitions. Handlers do their own partial flushes.
class Screen {
public:
    explicit Screen(App& app) : app_(app) {}
    virtual ~Screen() = default;

    virtual void draw() = 0;
    virtual void onTap(Tap tap) = 0;
    // Called on input timeout with the active seconds since the last call.
    virtual void onTick(uint32_t /*activeSeconds*/) {}
    // Only time spent on screens that return true counts as solving time (US5;
    // device sleep and menu time are excluded per spec edge case).
    virtual bool countsPlayTime() const { return false; }

protected:
    App& app_;
};

}  // namespace sudoku::ui
