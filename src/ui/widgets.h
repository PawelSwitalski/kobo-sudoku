#pragma once
#include <string>
#include <vector>

#include "core/session.h"
#include "platform/input.h"
#include "platform/renderer.h"
#include "ui/theme.h"

namespace sudoku::ui {

// All widgets draw through Renderer only — no OS calls (Constitution I).

struct Button {
    Rect rect;
    std::string label;
    bool enabled = true;
    bool toggled = false;  // pressed-in look (e.g. pencil mode)
    int textPx = 0;        // 0 = theme.textPx

    void draw(Renderer& r, const Theme& t) const;
    bool hit(Tap tap) const { return enabled && rect.contains({tap.x, tap.y}); }
};

struct Label {
    Rect rect;
    std::string text;
    int sizePx = 0;  // 0 = theme.textPx
    bool bold = false;
    TextStyle::Align align = TextStyle::Align::Center;

    void draw(Renderer& r, const Theme& t) const;
};

// Modal confirm/info dialog. The owning screen draws it last and routes taps
// to hitButton() while active.
class Dialog {
public:
    static Dialog info(std::string title, std::string message, std::string okLabel);
    static Dialog confirm(std::string title, std::string message, std::string cancelLabel,
                          std::string confirmLabel);

    void layout(const Theme& t, const DisplayInfo& d);
    void draw(Renderer& r, const Theme& t) const;
    // -1 = no button hit (taps outside are swallowed: modal).
    int hitButton(Tap tap) const;
    int buttonCount() const { return static_cast<int>(buttons_.size()); }
    Rect box() const { return box_; }

private:
    std::string title_, message_;
    std::vector<Button> buttons_;
    Rect box_{};
};

// 9x9 board with thick 3x3 boundaries, bold givens, pencil marks at fixed
// slots (FR-005), grayscale-safe error shading (FR-007), hint markers, and
// same-digit highlight (spec "may" edge case, T020 stretch).
class BoardView {
public:
    void layout(Rect avail, const Theme& t);

    Rect rect() const { return board_; }
    Rect cellRect(int i) const;
    int cellAt(Tap tap) const;  // -1 when outside the grid

    // selectedDigit: value of the selected cell (0 = none) for same-digit
    // highlight. Draws the full board including grid lines.
    void drawAll(Renderer& r, const Theme& t, const core::Session& s, int selectedCell) const;
    // Redraws a single cell's interior (partial-refresh path).
    void drawCell(Renderer& r, const Theme& t, const core::Session& s, int i,
                  int selectedCell) const;

private:
    void drawGridLines(Renderer& r, const Theme& t) const;

    Rect board_{};
    int cellPx = 0;
    int origin[10] = {};  // cell edge positions including line offsets
};

// Formats seconds as "M:SS" (or "H:MM:SS" past an hour).
std::string formatTime(uint32_t seconds);

}  // namespace sudoku::ui
