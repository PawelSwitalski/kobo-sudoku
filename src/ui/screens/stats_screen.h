#pragma once
#include "ui/app.h"
#include "ui/widgets.h"

namespace sudoku::ui {

// Per-difficulty completed / best / average times (FR-011).
class StatsScreen : public Screen {
public:
    explicit StatsScreen(App& app);

    void draw() override;
    void onTap(Tap tap) override;

private:
    Button backBtn_;
};

}  // namespace sudoku::ui
