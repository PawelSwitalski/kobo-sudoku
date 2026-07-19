#pragma once
#include "ui/app.h"
#include "ui/widgets.h"

namespace sudoku::ui {

// Settings: timer visibility toggle, e-ink full-refresh interval (FR-012).
class SettingsScreen : public Screen {
public:
    explicit SettingsScreen(App& app);

    void draw() override;
    void onTap(Tap tap) override;

private:
    static std::string refreshLabel(int n);

    Rect timerLabelRect_{}, refreshLabelRect_{};
    Button timerBtn_, refreshBtn_, backBtn_;
};

}  // namespace sudoku::ui
