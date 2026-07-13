#include "core/difficulty.h"
#include "core/board.h"

namespace sudoku::core {

namespace {

constexpr uint16_t kAll = 0x1FF;

inline uint16_t bit(int d) { return static_cast<uint16_t>(1u << (d - 1)); }

inline int popcount9(uint16_t v) {
    int n = 0;
    while (v) { v &= static_cast<uint16_t>(v - 1); ++n; }
    return n;
}

// 27 units: rows 0-8, columns 9-17, boxes 18-26.
const std::array<std::array<int, 9>, 27>& units() {
    static const auto table = [] {
        std::array<std::array<int, 9>, 27> t{};
        for (int r = 0; r < 9; ++r)
            for (int c = 0; c < 9; ++c) {
                t[r][c] = r * 9 + c;
                t[9 + c][r] = r * 9 + c;
            }
        for (int b = 0; b < 9; ++b) {
            int base = (b / 3) * 27 + (b % 3) * 3;
            for (int k = 0; k < 9; ++k)
                t[18 + b][k] = base + (k / 3) * 9 + (k % 3);
        }
        return t;
    }();
    return table;
}

struct SolveState {
    Grid g{};
    std::array<uint16_t, 81> cand{};
    bool contradiction = false;

    void init(const Grid& givens) {
        g = givens;
        for (int i = 0; i < 81; ++i) {
            if (g[i]) { cand[i] = 0; continue; }
            uint16_t used = 0;
            for (int p : peersOf(i))
                if (g[p]) used |= bit(g[p]);
            cand[i] = kAll & static_cast<uint16_t>(~used);
            if (!cand[i]) contradiction = true;
        }
    }

    void place(int i, int d) {
        g[i] = static_cast<uint8_t>(d);
        cand[i] = 0;
        for (int p : peersOf(i)) {
            if (g[p]) continue;
            cand[p] &= static_cast<uint16_t>(~bit(d));
            if (!cand[p]) contradiction = true;
        }
    }

    bool solved() const {
        for (int i = 0; i < 81; ++i)
            if (!g[i]) return false;
        return true;
    }

    // Tier 0: naked singles + hidden singles. Returns true on any placement.
    bool singlesPass() {
        bool progress = false;
        for (int i = 0; i < 81 && !contradiction; ++i) {
            if (g[i] || popcount9(cand[i]) != 1) continue;
            for (int d = 1; d <= 9; ++d)
                if (cand[i] & bit(d)) { place(i, d); break; }
            progress = true;
        }
        for (const auto& u : units()) {
            if (contradiction) break;
            for (int d = 1; d <= 9; ++d) {
                int pos = -1, count = 0;
                bool present = false;
                for (int i : u) {
                    if (g[i] == d) { present = true; break; }
                    if (!g[i] && (cand[i] & bit(d))) { pos = i; ++count; }
                }
                if (present || count != 1) continue;
                place(pos, d);
                progress = true;
            }
        }
        return progress;
    }

    // Tier 1: locked candidates (pointing + claiming) and naked pairs.
    // Returns true on any candidate elimination.
    bool tier1Pass() {
        bool progress = false;

        // Locked candidates, pointing: digit confined to one row/col of a box.
        for (int b = 0; b < 9 && !contradiction; ++b) {
            const auto& u = units()[18 + b];
            for (int d = 1; d <= 9; ++d) {
                int rows = 0, cols = 0, n = 0;
                for (int i : u)
                    if (!g[i] && (cand[i] & bit(d))) { rows |= 1 << rowOf(i); cols |= 1 << colOf(i); ++n; }
                if (n < 2) continue;
                if (popcount9(static_cast<uint16_t>(rows)) == 1) {
                    int r = 0; while (!((rows >> r) & 1)) ++r;
                    for (int c = 0; c < 9; ++c) {
                        int i = r * 9 + c;
                        if (boxOf(i) == b || g[i] || !(cand[i] & bit(d))) continue;
                        cand[i] &= static_cast<uint16_t>(~bit(d));
                        if (!cand[i]) contradiction = true;
                        progress = true;
                    }
                }
                if (popcount9(static_cast<uint16_t>(cols)) == 1) {
                    int c = 0; while (!((cols >> c) & 1)) ++c;
                    for (int r = 0; r < 9; ++r) {
                        int i = r * 9 + c;
                        if (boxOf(i) == b || g[i] || !(cand[i] & bit(d))) continue;
                        cand[i] &= static_cast<uint16_t>(~bit(d));
                        if (!cand[i]) contradiction = true;
                        progress = true;
                    }
                }
            }
        }

        // Locked candidates, claiming: digit in a row/col confined to one box.
        for (int line = 0; line < 18 && !contradiction; ++line) {
            const auto& u = units()[line];
            for (int d = 1; d <= 9; ++d) {
                int boxes = 0, n = 0;
                for (int i : u)
                    if (!g[i] && (cand[i] & bit(d))) { boxes |= 1 << boxOf(i); ++n; }
                if (n < 2 || popcount9(static_cast<uint16_t>(boxes)) != 1) continue;
                int b = 0; while (!((boxes >> b) & 1)) ++b;
                for (int i : units()[18 + b]) {
                    bool inLine = (line < 9) ? rowOf(i) == line : colOf(i) == line - 9;
                    if (inLine || g[i] || !(cand[i] & bit(d))) continue;
                    cand[i] &= static_cast<uint16_t>(~bit(d));
                    if (!cand[i]) contradiction = true;
                    progress = true;
                }
            }
        }

        // Naked pairs: two cells of a unit sharing the same 2-candidate set.
        for (const auto& u : units()) {
            if (contradiction) break;
            for (int a = 0; a < 9; ++a) {
                int ia = u[a];
                if (g[ia] || popcount9(cand[ia]) != 2) continue;
                for (int bIdx = a + 1; bIdx < 9; ++bIdx) {
                    int ib = u[bIdx];
                    if (g[ib] || cand[ib] != cand[ia]) continue;
                    for (int i : u) {
                        if (i == ia || i == ib || g[i] || !(cand[i] & cand[ia])) continue;
                        cand[i] &= static_cast<uint16_t>(~cand[ia]);
                        if (!cand[i]) contradiction = true;
                        progress = true;
                    }
                }
            }
        }
        return progress;
    }
};

}  // namespace

bool logicalSolve(Grid g, int maxTier, bool* usedTier1) {
    SolveState s;
    s.init(g);
    if (usedTier1) *usedTier1 = false;
    while (!s.contradiction && !s.solved()) {
        if (s.singlesPass()) continue;
        if (maxTier >= 1 && s.tier1Pass()) {
            if (usedTier1) *usedTier1 = true;
            continue;
        }
        break;
    }
    return !s.contradiction && s.solved();
}

Difficulty grade(const Grid& givens) {
    bool usedTier1 = false;
    if (logicalSolve(givens, 1, &usedTier1))
        return usedTier1 ? Difficulty::Medium : Difficulty::Easy;
    return Difficulty::Hard;
}

}  // namespace sudoku::core
