#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "core/generator.h"
#include "core/session.h"
#include "core/stats.h"
#include "persist/paths.h"
#include "persist/store.h"
#include "ui/app.h"
#include "ui/screens/menu_screen.h"
#include "ui/theme.h"

#if defined(SUDOKU_BACKEND_SDL)
#include "platform/sdl/mouse_touch.h"
#include "platform/sdl/sdl_renderer.h"
#elif defined(SUDOKU_BACKEND_FBINK)
#include "platform/kobo/evdev_touch.h"
#include "platform/kobo/fbink_renderer.h"
#endif

namespace {

volatile std::sig_atomic_t g_signalled = 0;
void onSignal(int) { g_signalled = 1; }  // persist-and-exit (device sleep/power)

struct Options {
    int width = 1264, height = 1680, dpi = 300;  // Kobo Libra Colour geometry
    bool color = true;
    const char* dataDir = nullptr;
    const char* assetsDir = nullptr;
    bool reveal = false;  // dev cheat: pre-fill all but a few cells (US1 test)
};

Options parseArgs(int argc, char** argv) {
    Options o;
    for (int i = 1; i < argc; ++i) {
        auto next = [&](int& out) { if (i + 1 < argc) out = std::atoi(argv[++i]); };
        if (!std::strcmp(argv[i], "--width")) next(o.width);
        else if (!std::strcmp(argv[i], "--height")) next(o.height);
        else if (!std::strcmp(argv[i], "--dpi")) next(o.dpi);
        else if (!std::strcmp(argv[i], "--gray")) o.color = false;
        else if (!std::strcmp(argv[i], "--reveal")) o.reveal = true;
        else if (!std::strcmp(argv[i], "--data-dir") && i + 1 < argc) o.dataDir = argv[++i];
        else if (!std::strcmp(argv[i], "--assets") && i + 1 < argc) o.assetsDir = argv[++i];
    }
    return o;
}

std::string isoNowUtc() {
    std::time_t now = std::time(nullptr);
    char buf[32];
    std::tm tm{};
#ifdef _WIN32
    gmtime_s(&tm, &now);
#else
    gmtime_r(&now, &tm);
#endif
    std::strftime(buf, sizeof buf, "%Y-%m-%dT%H:%M:%SZ", &tm);
    return buf;
}

std::string resolveAssetsDir(const Options& o) {
    if (o.assetsDir) return o.assetsDir;
    if (const char* env = std::getenv("SUDOKU_ASSETS_DIR"); env && *env) return env;
#if defined(SUDOKU_BACKEND_FBINK)
    return "/mnt/onboard/.adds/sudoku/assets";
#else
    return "dist/.adds/sudoku/assets";
#endif
}

class AppImpl : public sudoku::ui::App {
public:
    AppImpl(sudoku::Renderer& r, const sudoku::ui::Theme& theme, sudoku::persist::Paths paths,
            bool reveal)
        : renderer_(r), theme_(theme), paths_(std::move(paths)), reveal_(reveal) {
        if (auto text = sudoku::persist::loadFile(paths_.stats)) {
            try { stats_ = sudoku::core::Stats::fromJson(*text); } catch (...) {}
        }
        if (auto text = sudoku::persist::loadFile(paths_.settings)) {
            try { settings_ = sudoku::core::Settings::fromJson(*text); } catch (...) {}
        }
    }

    sudoku::Renderer& renderer() override { return renderer_; }
    const sudoku::ui::Theme& theme() const override { return theme_; }
    sudoku::core::Session* session() override { return session_.get(); }
    sudoku::core::Settings& settings() override { return settings_; }
    const sudoku::core::Stats& stats() const override { return stats_; }

    bool hasSavedGame() const override {
        return sudoku::persist::loadFile(paths_.save).has_value();
    }

    void startNewGame(sudoku::core::Difficulty d) override {
        std::random_device rd;
        uint64_t seed = (static_cast<uint64_t>(rd()) << 32) ^ rd() ^
                        static_cast<uint64_t>(
                            std::chrono::system_clock::now().time_since_epoch().count());
        sudoku::core::Puzzle p = sudoku::core::generate(d, seed);
        session_ = std::make_unique<sudoku::core::Session>(p);
        if (reveal_) {
            // Leave only three cells to fill: quick manual completion testing.
            std::vector<int> empties;
            for (int i = 0; i < 81; ++i)
                if (session_->cell(i).kind == sudoku::core::CellKind::Empty)
                    empties.push_back(i);
            for (size_t k = 0; k + 3 < empties.size(); ++k)
                session_->commitDigit(empties[k], p.solution[empties[k]]);
        }
        autosave();
    }

    bool continueSavedGame() override {
        auto text = sudoku::persist::loadFile(paths_.save);
        if (!text) return false;
        try {
            session_ = std::make_unique<sudoku::core::Session>(
                sudoku::core::Session::fromJson(*text));
            return true;
        } catch (const std::exception& e) {
            // Corrupt save: log, drop the file, degrade to "no save" (FR-018).
            std::fprintf(stderr, "save.json rejected: %s\n", e.what());
            sudoku::persist::removeFile(paths_.save);
            return false;
        }
    }

    void autosave() override {
        if (session_)
            sudoku::persist::saveFileAtomic(paths_.save, session_->toJson(isoNowUtc()));
    }

    void completeGame() override {
        if (!session_) return;
        stats_.recordCompletion(session_->puzzle().difficulty, session_->elapsedSeconds());
        sudoku::persist::saveFileAtomic(paths_.stats, stats_.toJson());
        sudoku::persist::removeFile(paths_.save);  // completed: no game to resume
    }

    void saveSettings() override {
        sudoku::persist::saveFileAtomic(paths_.settings, settings_.toJson());
    }

    void push(std::unique_ptr<sudoku::ui::Screen> s) override {
        stack_.push_back(std::move(s));
        navDirty_ = true;
    }
    void pop() override {
        if (!stack_.empty()) stack_.pop_back();
        navDirty_ = true;
    }
    void requestExit() override { exitRequested_ = true; }

    // --- app-shell surface (not part of the Screen-facing interface) ---
    sudoku::ui::Screen* top() { return stack_.empty() ? nullptr : stack_.back().get(); }
    bool exitRequested() const { return exitRequested_; }
    bool consumeNavDirty() { bool v = navDirty_; navDirty_ = false; return v; }

private:
    sudoku::Renderer& renderer_;
    const sudoku::ui::Theme& theme_;
    sudoku::persist::Paths paths_;
    bool reveal_;

    std::unique_ptr<sudoku::core::Session> session_;
    sudoku::core::Stats stats_;
    sudoku::core::Settings settings_;

    std::vector<std::unique_ptr<sudoku::ui::Screen>> stack_;
    bool navDirty_ = false;
    bool exitRequested_ = false;
};

}  // namespace

int main(int argc, char** argv) {
    Options opt = parseArgs(argc, argv);
    std::signal(SIGINT, onSignal);
    std::signal(SIGTERM, onSignal);

    std::string assets = resolveAssetsDir(opt);
    sudoku::persist::Paths paths = sudoku::persist::resolveDataDir(opt.dataDir);

    bool sdlQuit = false, sdlRedraw = false;

#if defined(SUDOKU_BACKEND_SDL)
    sudoku::SdlRenderer renderer;
    sudoku::ui::Theme theme = sudoku::ui::makeTheme({opt.width, opt.height, opt.dpi, opt.color},
                                                    assets);
    if (!renderer.init(opt.width, opt.height, opt.dpi, opt.color, theme.fontPath,
                       theme.fontBoldPath)) {
        std::fprintf(stderr, "renderer init failed (fonts at %s?)\n", assets.c_str());
        return 1;
    }
    sudoku::MouseTouch touch(&sdlQuit, &sdlRedraw);
#elif defined(SUDOKU_BACKEND_FBINK)
    sudoku::FbinkRenderer renderer;
    if (!renderer.init(assets)) {
        std::fprintf(stderr, "FBInk init failed\n");
        return 1;
    }
    sudoku::ui::Theme theme = sudoku::ui::makeTheme(renderer.info(), assets);
    sudoku::EvdevTouch touch;
    if (!touch.init(renderer.info())) {
        std::fprintf(stderr, "touch input init failed\n");
        return 1;
    }
#else
    (void)sdlQuit; (void)sdlRedraw;
    std::fprintf(stderr, "built without a backend (SUDOKU_BACKEND=none)\n");
    return 1;
#endif

#if defined(SUDOKU_BACKEND_SDL) || defined(SUDOKU_BACKEND_FBINK)
    AppImpl app(renderer, theme, paths, opt.reveal);
    app.push(std::make_unique<sudoku::ui::MenuScreen>(app));
    app.consumeNavDirty();
    app.top()->draw();
    renderer.flushFull();

    const int kTimeoutMs = 20000;       // wakes the loop for minute-level timer updates
    const int kSleepGapMs = 45000;      // wall-clock gap => device slept (FR-016)
    auto lastSteady = std::chrono::steady_clock::now();
    auto lastWall = std::chrono::system_clock::now();
    uint32_t activeMsAccum = 0;

    while (!app.exitRequested() && !g_signalled && !sdlQuit && app.top()) {
        std::optional<sudoku::Tap> tap = touch.waitForTap(kTimeoutMs);

        auto nowSteady = std::chrono::steady_clock::now();
        auto nowWall = std::chrono::system_clock::now();
        int64_t steadyMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(nowSteady - lastSteady).count();
        int64_t wallMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(nowWall - lastWall).count();
        lastSteady = nowSteady;
        lastWall = nowWall;

        // Sleep exclusion (T040): only count time the loop was demonstrably
        // active; anything beyond the wait timeout (suspend, clock jumps) is
        // capped away.
        int64_t activeMs = steadyMs;
        if (activeMs < 0) activeMs = 0;
        if (activeMs > kTimeoutMs + 5000) activeMs = kTimeoutMs + 5000;
        bool slept = wallMs > kTimeoutMs + kSleepGapMs || wallMs < 0;

        sudoku::ui::Screen* screen = app.top();
        if (screen->countsPlayTime() && app.session()) {
            activeMsAccum += static_cast<uint32_t>(activeMs);
            if (activeMsAccum >= 1000) {
                app.session()->tick(activeMsAccum / 1000);
                activeMsAccum %= 1000;
            }
        }

        if (slept || sdlRedraw) {
            // Wake-from-sleep: the sleep screen may cover us; repaint fully.
            sdlRedraw = false;
            screen->draw();
            renderer.flushFull();
        }

        if (tap)
            screen->onTap(*tap);
        else
            screen->onTick(0);

        if (app.consumeNavDirty()) {
            if (!app.top()) break;
            app.top()->draw();
            renderer.flushFull();  // screen transition: clean full refresh
        }
    }

    // Never lose progress: persist on every exit path (Constitution V).
    app.autosave();
    return 0;
#endif
}
