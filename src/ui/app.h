#pragma once
#include <memory>

#include "core/session.h"
#include "core/stats.h"
#include "platform/renderer.h"
#include "ui/screens/screen.h"
#include "ui/theme.h"

namespace sudoku::ui {

// Semantic surface the screens talk to. Implemented by the app shell in
// main.cpp, which owns backends, persistence wiring and the screen stack.
class App {
public:
    virtual ~App() = default;

    virtual Renderer& renderer() = 0;
    virtual const Theme& theme() const = 0;

    virtual core::Session* session() = 0;  // null when no game is in progress
    virtual core::Settings& settings() = 0;
    virtual const core::Stats& stats() const = 0;

    virtual bool hasSavedGame() const = 0;
    // Generates a new puzzle (discards any existing save) and makes it current.
    virtual void startNewGame(core::Difficulty d) = 0;
    // Loads save.json into the current session; false if missing/corrupt (FR-018).
    virtual bool continueSavedGame() = 0;
    virtual void autosave() = 0;      // save.json after every action (SC-005)
    virtual void completeGame() = 0;  // record stats, delete save.json
    virtual void saveSettings() = 0;

    // Navigation. Transitions trigger a full redraw + flushFull (Constitution II).
    virtual void push(std::unique_ptr<Screen> s) = 0;
    virtual void pop() = 0;
    virtual void requestExit() = 0;
};

}  // namespace sudoku::ui
