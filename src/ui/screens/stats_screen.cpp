#include "ui/screens/stats_screen.h"

namespace sudoku::ui {

using core::Difficulty;

StatsScreen::StatsScreen(App& app) : Screen(app) {
    const Theme& t = app_.theme();
    DisplayInfo d = app_.renderer().info();
    int bw = t.mm(50.0);
    backBtn_ = {{(d.width - bw) / 2, d.height - t.pad - t.touchTargetPx * 5 / 4, bw,
                 t.touchTargetPx * 5 / 4},
                "Back"};
}

void StatsScreen::draw() {
    Renderer& r = app_.renderer();
    const Theme& t = app_.theme();
    DisplayInfo d = r.info();
    r.fillRect({0, 0, d.width, d.height}, Gray::White);

    TextStyle title;
    title.sizePx = t.titlePx;
    title.bold = true;
    r.drawText({0, t.mm(10.0), d.width, t.titlePx * 2}, "Statistics", title);

    // Compact table so it fits 6" screens too (FR-015): one row per difficulty.
    int x = t.pad * 2;
    int w = d.width - 2 * x;
    int y = t.mm(10.0) + t.titlePx * 2 + t.mm(6.0);
    int rowH = t.textPx * 2;
    int colW[4] = {w * 30 / 100, w * 18 / 100, w * 26 / 100, w * 26 / 100};

    const char* headers[4] = {"", "Done", "Best", "Average"};
    int cx = x;
    for (int c = 0; c < 4; ++c) {
        Label{{cx, y, colW[c], rowH}, headers[c], t.smallPx, true, TextStyle::Align::Left}.draw(r,
                                                                                               t);
        cx += colW[c];
    }
    y += rowH;

    for (int di = 0; di < 3; ++di) {
        const core::DiffStats& s = app_.stats().at(static_cast<Difficulty>(di));
        std::string name = core::difficultyName(static_cast<Difficulty>(di));
        name[0] = static_cast<char>(name[0] - 'a' + 'A');
        std::string cells[4] = {
            name, std::to_string(s.completed),
            s.completed ? formatTime(s.bestSeconds) : "-",
            s.completed ? formatTime(static_cast<uint32_t>(s.totalSeconds / s.completed)) : "-"};
        cx = x;
        for (int c = 0; c < 4; ++c) {
            Label{{cx, y, colW[c], rowH}, cells[c], 0, c == 0, TextStyle::Align::Left}.draw(r, t);
            cx += colW[c];
        }
        y += rowH;
    }
    backBtn_.draw(r, t);
}

void StatsScreen::onTap(Tap tap) {
    if (backBtn_.hit(tap)) app_.pop();
}

}  // namespace sudoku::ui
