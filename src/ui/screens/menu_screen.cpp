#include "ui/screens/menu_screen.h"

#include "ui/screens/game_screen.h"
#include "ui/screens/settings_screen.h"
#include "ui/screens/stats_screen.h"

namespace sudoku::ui {

using core::Difficulty;

MenuScreen::MenuScreen(App& app) : Screen(app) {
    layoutButtons();
}

void MenuScreen::layoutButtons() {
    const Theme& t = app_.theme();
    DisplayInfo d = app_.renderer().info();

    std::vector<std::string> labels;
    if (state_ == State::Main) {
        if (app_.hasSavedGame()) labels.push_back("Continue");
        labels.insert(labels.end(), {"New Game", "Statistics", "Settings", "Exit"});
    } else {
        labels = {"Easy", "Medium", "Hard", "Back"};
    }

    int bw = t.mm(70.0);
    if (bw > d.width - 2 * t.pad) bw = d.width - 2 * t.pad;
    int bh = t.touchTargetPx * 5 / 4;
    int n = static_cast<int>(labels.size());
    int totalH = n * bh + (n - 1) * t.gap * 2;
    int y = (d.height - totalH) / 2 + t.mm(6.0);  // slightly below center; title above

    buttons_.clear();
    for (int i = 0; i < n; ++i)
        buttons_.push_back({{(d.width - bw) / 2, y + i * (bh + 2 * t.gap), bw, bh}, labels[i]});
}

void MenuScreen::draw() {
    // Re-layout every draw: Continue availability changes when a game starts,
    // finishes, or its save turns out to be corrupt.
    layoutButtons();

    Renderer& r = app_.renderer();
    const Theme& t = app_.theme();
    DisplayInfo d = r.info();
    r.fillRect({0, 0, d.width, d.height}, Gray::White);

    Rect titleRect = {0, t.mm(12.0), d.width, t.titlePx * 2};
    TextStyle title;
    title.sizePx = t.titlePx;
    title.bold = true;
    r.drawText(titleRect, state_ == State::Main ? "Kobo Sudoku" : "Choose difficulty", title);

    for (const Button& b : buttons_)
        b.draw(r, t);
    if (confirmDiscard_) confirmDiscard_->draw(r, t);
}

void MenuScreen::startGame(Difficulty d) {
    Renderer& r = app_.renderer();
    const Theme& t = app_.theme();
    DisplayInfo di = r.info();
    // Busy indicator: generation is usually instant but may take a moment (SC-001).
    r.fillRect({0, 0, di.width, di.height}, Gray::White);
    TextStyle st;
    st.sizePx = t.titlePx;
    r.drawText({0, 0, di.width, di.height}, "Generating puzzle...", st);
    r.flushFull();

    app_.startNewGame(d);
    state_ = State::Main;
    layoutButtons();
    app_.push(std::make_unique<GameScreen>(app_));
}

void MenuScreen::onTap(Tap tap) {
    if (confirmDiscard_) {
        int b = confirmDiscard_->hitButton(tap);
        if (b < 0) return;
        confirmDiscard_.reset();
        if (b == 1) {  // confirmed discard
            state_ = State::PickDifficulty;
            layoutButtons();
        }
        draw();
        app_.renderer().flushFull();
        return;
    }

    for (const Button& btn : buttons_) {
        if (!btn.hit(tap)) continue;
        const std::string& l = btn.label;
        if (state_ == State::Main) {
            if (l == "Continue") {
                if (app_.continueSavedGame())
                    app_.push(std::make_unique<GameScreen>(app_));
                else {
                    // Corrupt save: degrade gracefully to a clean menu (FR-018).
                    layoutButtons();
                    draw();
                    app_.renderer().flushFull();
                }
            } else if (l == "New Game") {
                if (app_.hasSavedGame()) {
                    confirmDiscard_ = Dialog::confirm(
                        "Start a new game?", "Your saved game will be discarded.", "Cancel",
                        "New Game");
                    confirmDiscard_->layout(app_.theme(), app_.renderer().info());
                    confirmDiscard_->draw(app_.renderer(), app_.theme());
                    app_.renderer().flushFull();
                } else {
                    state_ = State::PickDifficulty;
                    layoutButtons();
                    draw();
                    app_.renderer().flushFull();
                }
            } else if (l == "Statistics") {
                app_.push(std::make_unique<StatsScreen>(app_));
            } else if (l == "Settings") {
                app_.push(std::make_unique<SettingsScreen>(app_));
            } else if (l == "Exit") {
                app_.requestExit();
            }
        } else {
            if (l == "Easy") startGame(Difficulty::Easy);
            else if (l == "Medium") startGame(Difficulty::Medium);
            else if (l == "Hard") startGame(Difficulty::Hard);
            else {
                state_ = State::Main;
                layoutButtons();
                draw();
                app_.renderer().flushFull();
            }
        }
        return;
    }
}

}  // namespace sudoku::ui
