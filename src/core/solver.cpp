#include "core/solver.h"
#include "core/board.h"

namespace sudoku::core {

uint64_t nextRandom(uint64_t& state) {
    state += 0x9E3779B97F4A7C15ULL;
    uint64_t z = state;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

namespace {

constexpr uint16_t kAll = 0x1FF;

inline uint16_t bit(int d) { return static_cast<uint16_t>(1u << (d - 1)); }

inline int popcount9(uint16_t v) {
    int n = 0;
    while (v) { v &= static_cast<uint16_t>(v - 1); ++n; }
    return n;
}

// Used-digit masks per row/col/box for fast candidate lookup.
struct Masks {
    uint16_t row[9]{}, col[9]{}, box[9]{};

    bool init(const Grid& g) {
        for (int i = 0; i < 81; ++i) {
            if (!g[i]) continue;
            uint16_t b = bit(g[i]);
            if ((row[rowOf(i)] | col[colOf(i)] | box[boxOf(i)]) & b) return false;  // contradiction
            place(i, g[i]);
        }
        return true;
    }
    void place(int i, int d) {
        row[rowOf(i)] |= bit(d); col[colOf(i)] |= bit(d); box[boxOf(i)] |= bit(d);
    }
    void remove(int i, int d) {
        row[rowOf(i)] &= ~bit(d); col[colOf(i)] &= ~bit(d); box[boxOf(i)] &= ~bit(d);
    }
    uint16_t candidates(int i) const {
        return kAll & static_cast<uint16_t>(~(row[rowOf(i)] | col[colOf(i)] | box[boxOf(i)]));
    }
};

int countRec(Grid& g, Masks& m, int limit) {
    // Most-constrained-cell heuristic keeps worst-case search shallow.
    int best = -1, bestCount = 10;
    uint16_t bestCand = 0;
    for (int i = 0; i < 81; ++i) {
        if (g[i]) continue;
        uint16_t c = m.candidates(i);
        int n = popcount9(c);
        if (n == 0) return 0;
        if (n < bestCount) {
            bestCount = n; best = i; bestCand = c;
            if (n == 1) break;
        }
    }
    if (best == -1) return 1;  // no empties: solved

    int total = 0;
    for (int d = 1; d <= 9; ++d) {
        if (!(bestCand & bit(d))) continue;
        g[best] = static_cast<uint8_t>(d);
        m.place(best, d);
        total += countRec(g, m, limit - total);
        m.remove(best, d);
        g[best] = 0;
        if (total >= limit) return total;
    }
    return total;
}

bool fillRec(Grid& g, Masks& m, int pos, uint64_t& rng) {
    while (pos < 81 && g[pos]) ++pos;
    if (pos == 81) return true;

    uint16_t cand = m.candidates(pos);
    int digits[9], n = 0;
    for (int d = 1; d <= 9; ++d)
        if (cand & bit(d)) digits[n++] = d;
    // Deterministic Fisher-Yates (std::shuffle is implementation-defined).
    for (int k = n - 1; k > 0; --k) {
        int j = static_cast<int>(nextRandom(rng) % static_cast<uint64_t>(k + 1));
        int t = digits[k]; digits[k] = digits[j]; digits[j] = t;
    }
    for (int k = 0; k < n; ++k) {
        int d = digits[k];
        g[pos] = static_cast<uint8_t>(d);
        m.place(pos, d);
        if (fillRec(g, m, pos + 1, rng)) return true;
        m.remove(pos, d);
        g[pos] = 0;
    }
    return false;
}

}  // namespace

bool fillGrid(Grid& g, uint64_t& rngState) {
    Masks m;
    if (!m.init(g)) return false;
    return fillRec(g, m, 0, rngState);
}

int countSolutions(const Grid& g, int limit) {
    Grid work = g;
    Masks m;
    if (!m.init(work)) return 0;
    return countRec(work, m, limit);
}

}  // namespace sudoku::core
