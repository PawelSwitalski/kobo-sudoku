#include "persist/paths.h"

#include <cstdlib>
#include <filesystem>

namespace sudoku::persist {

namespace fs = std::filesystem;

Paths resolveDataDir(const char* cliOverride) {
    std::string dir;
    if (cliOverride && *cliOverride) {
        dir = cliOverride;
    } else if (const char* env = std::getenv("SUDOKU_DATA_DIR"); env && *env) {
        dir = env;
    } else {
        std::error_code ec;
        const char* kobo = "/mnt/onboard/.adds/sudoku";
        dir = fs::is_directory(kobo, ec) ? kobo : "sudoku-data";
    }
    std::error_code ec;
    fs::create_directories(dir, ec);  // best effort; saves will fail loudly if unusable

    Paths p;
    p.dataDir = dir;
    p.save = (fs::path(dir) / "save.json").string();
    p.stats = (fs::path(dir) / "stats.json").string();
    p.settings = (fs::path(dir) / "settings.json").string();
    return p;
}

}  // namespace sudoku::persist
