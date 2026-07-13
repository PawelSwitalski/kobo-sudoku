#pragma once
#include "ui/app.h"
#include "ui/widgets.h"

namespace sudoku::ui {

// Settings: timer visibility toggle (FR-012).
class SettingsScreen : public Screen {
public:
    explicit SettingsScreen(App& app);

    void draw() override;
    void onTap(Tap tap) override;

private:
    Rect timerLabelRect_{};
    Button timerBtn_, backBtn_;
};

}  // namespace sudoku::ui
