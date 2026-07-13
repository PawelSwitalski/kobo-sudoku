#pragma once
#include <optional>
#include <vector>

#include "ui/app.h"
#include "ui/widgets.h"

namespace sudoku::ui {

// Board + always-visible 1-9 pad, cell-first input (FR-004), pencil-mode
// toggle (US2), hint/error UX (US3), minute-granularity timer (US5).
class GameScreen : public Screen {
public:
    explicit GameScreen(App& app);

    void draw() override;
    void onTap(Tap tap) override;
    void onTick(uint32_t activeSeconds) override;
    bool countsPlayTime() const override { return dialogKind_ == DialogKind::None; }

private:
    enum class DialogKind { None, Completion, Info };

    void layoutUi();
    void drawTopBar();
    void drawControls();
    std::string timerText() const;
    void redrawCells(std::vector<int> cells);
    void appendCellsWithValue(std::vector<int>& out, int digit) const;
    void handleMutation(const core::ChangeSet& cs, int digitTouched);
    void checkCompletion();
    void openDialog(DialogKind kind, Dialog d);
    void closeDialog();

    BoardView board_;
    Button menuBtn_, pencilBtn_, clearBtn_, hintBtn_;
    Button digitBtns_[9];
    Rect topBar_{}, diffRect_{}, timerRect_{};

    int selected_ = -1;
    bool pencilMode_ = false;
    uint32_t shownMinutes_ = ~0u;

    DialogKind dialogKind_ = DialogKind::None;
    std::optional<Dialog> dialog_;
};

}  // namespace sudoku::ui
