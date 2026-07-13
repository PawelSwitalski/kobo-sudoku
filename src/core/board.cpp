#include "core/board.h"

namespace sudoku::core {

const char* difficultyName(Difficulty d) {
    switch (d) {
        case Difficulty::Easy: return "easy";
        case Difficulty::Medium: return "medium";
        case Difficulty::Hard: return "hard";
    }
    return "easy";
}

bool difficultyFromString(std::string_view s, Difficulty& out) {
    if (s == "easy") { out = Difficulty::Easy; return true; }
    if (s == "medium") { out = Difficulty::Medium; return true; }
    if (s == "hard") { out = Difficulty::Hard; return true; }
    return false;
}

namespace {
std::array<std::array<int, 20>, 81> buildPeers() {
    std::array<std::array<int, 20>, 81> table{};
    for (int i = 0; i < 81; ++i) {
        int n = 0;
        for (int j = 0; j < 81; ++j) {
            if (j == i) continue;
            if (rowOf(j) == rowOf(i) || colOf(j) == colOf(i) || boxOf(j) == boxOf(i))
                table[i][n++] = j;
        }
    }
    return table;
}
}  // namespace

const std::array<int, 20>& peersOf(int i) {
    static const auto table = buildPeers();
    return table[i];
}

bool isValidGrid(const Grid& g) {
    for (int i = 0; i < 81; ++i) {
        if (g[i] > 9) return false;
        if (g[i] == 0) continue;
        for (int p : peersOf(i))
            if (g[p] == g[i]) return false;
    }
    return true;
}

bool isSolvedGrid(const Grid& g) {
    for (int i = 0; i < 81; ++i)
        if (g[i] < 1 || g[i] > 9) return false;
    return isValidGrid(g);
}

}  // namespace sudoku::core
