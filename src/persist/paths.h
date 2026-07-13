#pragma once
#include <string>

namespace sudoku::persist {

struct Paths {
    std::string dataDir;
    std::string save;      // dataDir/save.json
    std::string stats;     // dataDir/stats.json
    std::string settings;  // dataDir/settings.json
};

// Resolution order: cliOverride > $SUDOKU_DATA_DIR > /mnt/onboard/.adds/sudoku
// (when it exists, i.e. on a Kobo) > ./sudoku-data. Creates the directory.
Paths resolveDataDir(const char* cliOverride);

}  // namespace sudoku::persist
