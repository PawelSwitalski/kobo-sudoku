#include "ui/screens/game_screen.h"

#include <string>

namespace sudoku::ui {

using core::ChangeSet;
using core::Difficulty;

GameScreen::GameScreen(App& app) : Screen(app) {
    layoutUi();
}

void GameScreen::layoutUi() {
    const Theme& t = app_.theme();
    DisplayInfo d = app_.renderer().info();
    int w = d.width, h = d.height;

    int btnH = t.touchTargetPx;
    topBar_ = {t.pad, t.pad, w - 2 * t.pad, btnH};
    menuBtn_ = {{topBar_.x, topBar_.y, t.mm(24.0), btnH}, "Menu"};
    timerRect_ = {topBar_.x + topBar_.w - t.mm(24.0), topBar_.y, t.mm(24.0), btnH};
    diffRect_ = {menuBtn_.rect.x + menuBtn_.rect.w, topBar_.y,
                 timerRect_.x - menuBtn_.rect.x - menuBtn_.rect.w, btnH};

    int row2Y = h - t.pad - btnH;
    int row1Y = row2Y - t.gap - btnH;

    int dw = (w - 2 * t.pad - 8 * t.gap) / 9;
    for (int i = 0; i < 9; ++i) {
        digitBtns_[i] = {{t.pad + i * (dw + t.gap), row1Y, dw, btnH},
                         std::string(1, static_cast<char>('1' + i))};
        digitBtns_[i].textPx = t.textPx * 5 / 4;
    }
    int cw = (w - 2 * t.pad - 2 * t.gap) / 3;
    pencilBtn_ = {{t.pad, row2Y, cw, btnH}, "Pencil"};
    clearBtn_ = {{t.pad + cw + t.gap, row2Y, cw, btnH}, "Clear"};
    hintBtn_ = {{t.pad + 2 * (cw + t.gap), row2Y, cw, btnH}, "Hint"};

    Rect avail = {t.pad, topBar_.y + topBar_.h + t.gap, w - 2 * t.pad,
                  row1Y - topBar_.y - topBar_.h - 2 * t.gap};
    board_.layout(avail, t);
}

std::string GameScreen::timerText() const {
    const core::Session* s = app_.session();
    if (!s || !app_.settings().showTimer) return "";
    // Minute granularity only — no per-second e-ink redraws (Constitution II).
    return std::to_string(s->elapsedSeconds() / 60) + " min";
}

void GameScreen::drawTopBar() {
    Renderer& r = app_.renderer();
    const Theme& t = app_.theme();
    r.fillRect(topBar_, Gray::White);
    menuBtn_.draw(r, t);
    const core::Session* s = app_.session();
    if (s) {
        std::string diff = core::difficultyName(s->puzzle().difficulty);
        diff[0] = static_cast<char>(diff[0] - 'a' + 'A');
        Label{diffRect_, diff, t.textPx, true}.draw(r, t);
    }
    Label{timerRect_, timerText(), t.smallPx, false, TextStyle::Align::Right}.draw(r, t);
}

void GameScreen::drawControls() {
    Renderer& r = app_.renderer();
    const Theme& t = app_.theme();
    for (const Button& b : digitBtns_)
        b.draw(r, t);
    pencilBtn_.toggled = pencilMode_;
    pencilBtn_.draw(r, t);
    clearBtn_.draw(r, t);
    hintBtn_.draw(r, t);
}

void GameScreen::draw() {
    Renderer& r = app_.renderer();
    DisplayInfo d = r.info();
    r.fillRect({0, 0, d.width, d.height}, Gray::White);
    drawTopBar();
    if (app_.session()) {
        board_.drawAll(r, app_.theme(), *app_.session(), selected_);
        shownMinutes_ = app_.session()->elapsedSeconds() / 60;
    }
    drawControls();
    if (dialog_) dialog_->draw(r, app_.theme());
}

void GameScreen::appendCellsWithValue(std::vector<int>& out, int digit) const {
    if (digit <= 0) return;
    const core::Session* s = app_.session();
    for (int i = 0; i < 81; ++i)
        if (s->cell(i).value == digit) out.push_back(i);
}

void GameScreen::redrawCells(std::vector<int> cells) {
    if (cells.empty() || !app_.session()) return;
    Renderer& r = app_.renderer();
    const Theme& t = app_.theme();
    Rect dirty{};
    for (int i : cells) {
        board_.drawCell(r, t, *app_.session(), i, selected_);
        dirty = dirty.unite(board_.cellRect(i));
    }
    r.flushPartial(dirty);
}

void GameScreen::handleMutation(const ChangeSet& cs, int digitTouched) {
    if (cs.empty()) return;
    app_.autosave();
    std::vector<int> dirty = cs.cells;
    // Same-digit highlight may change when a value appears/disappears.
    appendCellsWithValue(dirty, digitTouched);
    redrawCells(std::move(dirty));
    checkCompletion();
}

void GameScreen::checkCompletion() {
    core::Session* s = app_.session();
    if (!s || !s->isComplete()) return;
    app_.completeGame();

    std::string msg = "Difficulty: ";
    std::string diff = core::difficultyName(s->puzzle().difficulty);
    diff[0] = static_cast<char>(diff[0] - 'a' + 'A');
    msg += diff;
    if (app_.settings().showTimer)
        msg += "\nTime: " + formatTime(s->elapsedSeconds());
    msg += "\nHints used: " + std::to_string(s->hintsUsed());
    openDialog(DialogKind::Completion, Dialog::info("Puzzle solved!", msg, "OK"));
}

void GameScreen::openDialog(DialogKind kind, Dialog d) {
    dialogKind_ = kind;
    d.layout(app_.theme(), app_.renderer().info());
    dialog_ = std::move(d);
    dialog_->draw(app_.renderer(), app_.theme());
    // Full flush: a modal is a major state change and deserves a clean screen.
    app_.renderer().flushFull();
}

void GameScreen::closeDialog() {
    dialog_.reset();
    dialogKind_ = DialogKind::None;
    draw();
    app_.renderer().flushFull();
}

void GameScreen::onTap(Tap tap) {
    Renderer& r = app_.renderer();
    const Theme& t = app_.theme();
    core::Session* s = app_.session();

    if (dialog_) {
        int b = dialog_->hitButton(tap);
        if (b < 0) return;  // modal: swallow outside taps
        if (dialogKind_ == DialogKind::Completion) {
            app_.pop();
        } else {
            closeDialog();
        }
        return;
    }
    if (!s) return;

    if (menuBtn_.hit(tap)) {
        app_.autosave();
        app_.pop();
        return;
    }
    if (pencilBtn_.hit(tap)) {
        pencilMode_ = !pencilMode_;
        pencilBtn_.toggled = pencilMode_;
        pencilBtn_.draw(r, t);
        r.flushPartial(pencilBtn_.rect);
        return;
    }
    if (clearBtn_.hit(tap)) {
        if (selected_ < 0) return;
        int oldValue = s->cell(selected_).value;
        handleMutation(s->clearCell(selected_), oldValue);
        return;
    }
    if (hintBtn_.hit(tap)) {
        ChangeSet cs = s->applyHint();
        if (cs.empty()) {
            openDialog(DialogKind::Info,
                       Dialog::info("Hint", "Nothing to fill in —\nthe board is already correct.",
                                    "OK"));
            return;
        }
        int filled = cs.cells.empty() ? 0 : s->cell(cs.cells[0]).value;
        handleMutation(cs, filled);
        return;
    }
    for (int d = 0; d < 9; ++d) {
        if (!digitBtns_[d].hit(tap)) continue;
        if (selected_ < 0) return;
        if (pencilMode_) {
            handleMutation(s->toggleMark(selected_, d + 1), 0);
        } else {
            int oldValue = s->cell(selected_).value;
            ChangeSet cs = s->commitDigit(selected_, d + 1);
            if (!cs.empty() && oldValue) appendCellsWithValue(cs.cells, oldValue);
            handleMutation(cs, d + 1);
        }
        return;
    }
    int cell = board_.cellAt(tap);
    if (cell >= 0 && cell != selected_) {
        int old = selected_;
        int oldDigit = old >= 0 ? s->cell(old).value : 0;
        selected_ = cell;
        std::vector<int> dirty;
        if (old >= 0) dirty.push_back(old);
        dirty.push_back(cell);
        appendCellsWithValue(dirty, oldDigit);
        appendCellsWithValue(dirty, s->cell(cell).value);
        redrawCells(std::move(dirty));
    }
}

void GameScreen::onTick(uint32_t /*activeSeconds*/) {
    // Time accounting lives in the app shell (it ticks the session whenever the
    // active screen countsPlayTime()); here we only refresh the display.
    core::Session* s = app_.session();
    if (!s) return;
    uint32_t minutes = s->elapsedSeconds() / 60;
    if (app_.settings().showTimer && minutes != shownMinutes_) {
        shownMinutes_ = minutes;
        Renderer& r = app_.renderer();
        r.fillRect(timerRect_, Gray::White);
        Label{timerRect_, timerText(), app_.theme().smallPx, false, TextStyle::Align::Right}.draw(
            r, app_.theme());
        r.flushPartial(timerRect_);
    }
}

}  // namespace sudoku::ui
