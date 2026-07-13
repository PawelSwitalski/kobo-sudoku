#include "ui/widgets.h"

#include <cstdio>

namespace sudoku::ui {

using core::CellKind;

namespace {
void drawFrame(Renderer& r, Rect box, int thickness, Gray shade) {
    r.fillRect({box.x, box.y, box.w, thickness}, shade);
    r.fillRect({box.x, box.y + box.h - thickness, box.w, thickness}, shade);
    r.fillRect({box.x, box.y, thickness, box.h}, shade);
    r.fillRect({box.x + box.w - thickness, box.y, thickness, box.h}, shade);
}

std::vector<std::string> splitLines(const std::string& s) {
    std::vector<std::string> out;
    size_t start = 0;
    while (true) {
        size_t nl = s.find('\n', start);
        if (nl == std::string::npos) {
            out.push_back(s.substr(start));
            return out;
        }
        out.push_back(s.substr(start, nl - start));
        start = nl + 1;
    }
}
}  // namespace

void Button::draw(Renderer& r, const Theme& t) const {
    Gray bg = toggled ? Gray::Light : Gray::White;
    r.fillRect(rect, bg);
    drawFrame(r, rect, toggled ? t.mm(0.8) : t.mm(0.35), enabled ? Gray::Black : Gray::Mid);
    TextStyle st;
    st.sizePx = textPx > 0 ? textPx : t.textPx;
    st.shade = enabled ? Gray::Black : Gray::Mid;
    st.bold = toggled;
    r.drawText(rect, label, st);
}

void Label::draw(Renderer& r, const Theme& t) const {
    TextStyle st;
    st.sizePx = sizePx > 0 ? sizePx : t.textPx;
    st.bold = bold;
    st.align = align;
    r.drawText(rect, text, st);
}

Dialog Dialog::info(std::string title, std::string message, std::string okLabel) {
    Dialog d;
    d.title_ = std::move(title);
    d.message_ = std::move(message);
    d.buttons_.push_back({{}, std::move(okLabel)});
    return d;
}

Dialog Dialog::confirm(std::string title, std::string message, std::string cancelLabel,
                       std::string confirmLabel) {
    Dialog d;
    d.title_ = std::move(title);
    d.message_ = std::move(message);
    d.buttons_.push_back({{}, std::move(cancelLabel)});
    d.buttons_.push_back({{}, std::move(confirmLabel)});
    return d;
}

void Dialog::layout(const Theme& t, const DisplayInfo& d) {
    int lines = static_cast<int>(splitLines(message_).size());
    int lineH = t.textPx + t.textPx / 2;
    int w = d.width - 4 * t.touchTargetPx / 2;
    int maxW = t.mm(95.0);
    if (w > maxW) w = maxW;
    int h = t.pad * 2 + t.touchTargetPx            // title band
            + lines * lineH + t.gap                // message
            + t.touchTargetPx + t.pad;             // buttons
    box_ = {(d.width - w) / 2, (d.height - h) / 2, w, h};

    int n = static_cast<int>(buttons_.size());
    int bw = (w - (n + 1) * t.pad) / n;
    int by = box_.y + h - t.pad - t.touchTargetPx;
    for (int i = 0; i < n; ++i)
        buttons_[i].rect = {box_.x + t.pad + i * (bw + t.pad), by, bw, t.touchTargetPx};
}

void Dialog::draw(Renderer& r, const Theme& t) const {
    r.fillRect(box_, Gray::White);
    drawFrame(r, box_, t.mm(0.6), Gray::Black);

    Rect title = {box_.x + t.pad, box_.y + t.pad, box_.w - 2 * t.pad, t.touchTargetPx};
    TextStyle st;
    st.sizePx = t.textPx;
    st.bold = true;
    r.drawText(title, title_, st);

    int lineH = t.textPx + t.textPx / 2;
    int y = title.y + title.h + t.gap / 2;
    st.bold = false;
    for (const std::string& line : splitLines(message_)) {
        r.drawText({box_.x + t.pad, y, box_.w - 2 * t.pad, lineH}, line, st);
        y += lineH;
    }
    for (const Button& b : buttons_)
        b.draw(r, t);
}

int Dialog::hitButton(Tap tap) const {
    for (size_t i = 0; i < buttons_.size(); ++i)
        if (buttons_[i].hit(tap)) return static_cast<int>(i);
    return -1;
}

void BoardView::layout(Rect avail, const Theme& t) {
    int side = avail.w < avail.h ? avail.w : avail.h;
    // 4 thick boundaries (0,3,6,9) + 6 thin inner lines + 9 cells
    int lineTotal = 4 * t.thickLine + 6 * t.thinLine;
    cellPx = (side - lineTotal) / 9;
    int total = cellPx * 9 + lineTotal;
    board_ = {avail.x + (avail.w - total) / 2, avail.y + (avail.h - total) / 2, total, total};

    int pos = 0;
    for (int k = 0; k < 9; ++k) {
        pos += (k % 3 == 0) ? t.thickLine : t.thinLine;
        origin[k] = pos;
        pos += cellPx;
    }
    origin[9] = pos + t.thickLine;  // == total
}

Rect BoardView::cellRect(int i) const {
    return {board_.x + origin[core::colOf(i)], board_.y + origin[core::rowOf(i)], cellPx, cellPx};
}

int BoardView::cellAt(Tap tap) const {
    if (!board_.contains({tap.x, tap.y})) return -1;
    int lx = tap.x - board_.x, ly = tap.y - board_.y;
    int col = -1, row = -1;
    for (int k = 0; k < 9; ++k) {
        // Include half the surrounding lines so taps on lines still land.
        if (lx >= origin[k] - 2 && lx < origin[k] + cellPx + 2) col = k;
        if (ly >= origin[k] - 2 && ly < origin[k] + cellPx + 2) row = k;
    }
    if (col < 0 || row < 0) return -1;
    return row * 9 + col;
}

void BoardView::drawGridLines(Renderer& r, const Theme& t) const {
    r.fillRect(board_, Gray::White);
    for (int k = 0; k <= 9; k += 1) {
        int lw = (k % 3 == 0) ? t.thickLine : t.thinLine;
        int off = (k == 9) ? origin[9] - lw : origin[k] - lw;
        Gray shade = (k % 3 == 0) ? Gray::Black : Gray::Dark;
        r.fillRect({board_.x + off, board_.y, lw, board_.h}, shade);
        r.fillRect({board_.x, board_.y + off, board_.w, lw}, shade);
    }
}

void BoardView::drawAll(Renderer& r, const Theme& t, const core::Session& s,
                        int selectedCell) const {
    drawGridLines(r, t);
    for (int i = 0; i < 81; ++i)
        drawCell(r, t, s, i, selectedCell);
}

void BoardView::drawCell(Renderer& r, const Theme& t, const core::Session& s, int i,
                         int selectedCell) const {
    Rect cr = cellRect(i);
    const core::Cell& c = s.cell(i);
    bool selected = (i == selectedCell);
    bool error = s.isError(i);
    int selectedDigit =
        (selectedCell >= 0 && selectedCell != i) ? s.cell(selectedCell).value : 0;
    bool sameDigit = selectedDigit != 0 && c.value == selectedDigit;

    // Grayscale-distinguishable states (SC-006): error is the darkest shading,
    // same-digit highlight is faint, selection is a heavy frame.
    if (error)
        r.fillRect(cr, Gray::Light, Color::Red);
    else if (sameDigit)
        r.fillRect(cr, Gray::Lighter);
    else
        r.fillRect(cr, Gray::White);
    if (selected)
        drawFrame(r, cr, t.mm(0.9), Gray::Black);

    if (c.value != 0) {
        TextStyle st;
        st.sizePx = cellPx * 62 / 100;
        st.bold = (c.kind == CellKind::Given);
        st.shade = Gray::Black;
        if (error) st.accent = Color::Red;
        r.drawText(cr, std::string(1, static_cast<char>('0' + c.value)), st);
        if (c.fromHint) {
            int m = t.mm(1.1);
            r.fillRect({cr.x + cr.w - 2 * m, cr.y + cr.h - 2 * m, m, m}, Gray::Mid);
        }
    } else if (c.marks) {
        // Fixed slots: 1-3 top, 4-6 middle, 7-9 bottom (FR-005).
        TextStyle st;
        st.sizePx = cellPx * 27 / 100;
        st.shade = Gray::Dark;
        int third = cellPx / 3;
        for (int d = 1; d <= 9; ++d) {
            if (!c.hasMark(d)) continue;
            Rect slot = {cr.x + core::markSlotCol(d) * third, cr.y + core::markSlotRow(d) * third,
                         third, third};
            r.drawText(slot, std::string(1, static_cast<char>('0' + d)), st);
        }
    }
}

std::string formatTime(uint32_t seconds) {
    char buf[16];
    uint32_t h = seconds / 3600, m = (seconds / 60) % 60, s = seconds % 60;
    if (h > 0)
        std::snprintf(buf, sizeof buf, "%u:%02u:%02u", h, m, s);
    else
        std::snprintf(buf, sizeof buf, "%u:%02u", m, s);
    return buf;
}

}  // namespace sudoku::ui
