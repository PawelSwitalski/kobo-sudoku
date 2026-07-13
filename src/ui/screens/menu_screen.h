#pragma once
#include <optional>
#include <vector>

#include "ui/app.h"
#include "ui/widgets.h"

namespace sudoku::ui {

// Main menu: Continue (when a save exists), New Game -> difficulty picker
// (with discard confirmation, FR-019), Statistics, Settings, Exit.
class MenuScreen : public Screen {
public:
    explicit MenuScreen(App& app);

    void draw() override;
    void onTap(Tap tap) override;

private:
    enum class State { Main, PickDifficulty };

    void layoutButtons();
    void startGame(core::Difficulty d);

    State state_ = State::Main;
    std::vector<Button> buttons_;
    std::optional<Dialog> confirmDiscard_;
};

}  // namespace sudoku::ui
