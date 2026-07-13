#include "ui/screens/settings_screen.h"

namespace sudoku::ui {

SettingsScreen::SettingsScreen(App& app) : Screen(app) {
    const Theme& t = app_.theme();
    DisplayInfo d = app_.renderer().info();

    int y = t.mm(10.0) + t.titlePx * 2 + t.mm(10.0);
    int x = t.pad * 3;
    int bw = t.mm(28.0);
    timerLabelRect_ = {x, y, d.width - 2 * x - bw - t.gap, t.touchTargetPx};
    timerBtn_ = {{d.width - x - bw, y, bw, t.touchTargetPx}, ""};

    int backW = t.mm(50.0);
    backBtn_ = {{(d.width - backW) / 2, d.height - t.pad - t.touchTargetPx * 5 / 4, backW,
                 t.touchTargetPx * 5 / 4},
                "Back"};
}

void SettingsScreen::draw() {
    Renderer& r = app_.renderer();
    const Theme& t = app_.theme();
    DisplayInfo d = r.info();
    r.fillRect({0, 0, d.width, d.height}, Gray::White);

    TextStyle title;
    title.sizePx = t.titlePx;
    title.bold = true;
    r.drawText({0, t.mm(10.0), d.width, t.titlePx * 2}, "Settings", title);

    Label{timerLabelRect_, "Show timer while playing", 0, false, TextStyle::Align::Left}.draw(r,
                                                                                              t);
    timerBtn_.label = app_.settings().showTimer ? "On" : "Off";
    timerBtn_.toggled = app_.settings().showTimer;
    timerBtn_.draw(r, t);
    backBtn_.draw(r, t);
}

void SettingsScreen::onTap(Tap tap) {
    if (timerBtn_.hit(tap)) {
        app_.settings().showTimer = !app_.settings().showTimer;
        app_.saveSettings();
        timerBtn_.label = app_.settings().showTimer ? "On" : "Off";
        timerBtn_.toggled = app_.settings().showTimer;
        timerBtn_.draw(app_.renderer(), app_.theme());
        app_.renderer().flushPartial(timerBtn_.rect);
        return;
    }
    if (backBtn_.hit(tap)) app_.pop();
}

}  // namespace sudoku::ui
