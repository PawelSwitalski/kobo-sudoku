#include "core/session.h"

#include <algorithm>
#include <stdexcept>

#include "nlohmann/json.hpp"

namespace sudoku::core {

using nlohmann::json;

Session::Session(Puzzle p) : puzzle_(p) {
    for (int i = 0; i < 81; ++i) {
        if (puzzle_.givens[i]) {
            cells_[i].kind = CellKind::Given;
            cells_[i].value = puzzle_.givens[i];
        }
    }
}

namespace {
// Removes `digit` from the pencil marks of every peer of `cell` (FR-006a),
// recording each changed peer in the ChangeSet.
void cleanupPeerMarks(std::array<Cell, 81>& cells, int cell, int digit, ChangeSet& cs) {
    uint16_t b = static_cast<uint16_t>(1u << (digit - 1));
    for (int p : peersOf(cell)) {
        if (cells[p].value == 0 && (cells[p].marks & b)) {
            cells[p].marks &= static_cast<uint16_t>(~b);
            cs.cells.push_back(p);
        }
    }
}
}  // namespace

ChangeSet Session::commitDigit(int cell, int digit) {
    ChangeSet cs;
    if (cell < 0 || cell > 80 || digit < 1 || digit > 9) return cs;
    Cell& c = cells_[cell];
    if (c.kind == CellKind::Given) return cs;
    if (c.kind == CellKind::Entry && c.value == digit) return cs;  // nothing changes
    c.kind = CellKind::Entry;
    c.value = static_cast<uint8_t>(digit);
    c.marks = 0;      // FR-006
    c.fromHint = false;
    cs.cells.push_back(cell);
    cleanupPeerMarks(cells_, cell, digit, cs);
    return cs;
}

ChangeSet Session::clearCell(int cell) {
    ChangeSet cs;
    if (cell < 0 || cell > 80) return cs;
    Cell& c = cells_[cell];
    if (c.kind == CellKind::Given) return cs;
    if (c.kind == CellKind::Entry) {
        c = Cell{};
        cs.cells.push_back(cell);
    } else if (c.marks) {
        c.marks = 0;
        cs.cells.push_back(cell);
    }
    return cs;
}

ChangeSet Session::toggleMark(int cell, int digit) {
    ChangeSet cs;
    if (cell < 0 || cell > 80 || digit < 1 || digit > 9) return cs;
    Cell& c = cells_[cell];
    if (c.kind != CellKind::Empty) return cs;  // marks live only in empty cells
    c.marks ^= static_cast<uint16_t>(1u << (digit - 1));
    cs.cells.push_back(cell);
    return cs;
}

ChangeSet Session::applyHint() {
    ChangeSet cs;
    // Prefer fixing a wrong entry; otherwise fill the first empty cell.
    int target = -1;
    for (int i = 0; i < 81 && target < 0; ++i)
        if (isError(i)) target = i;
    for (int i = 0; i < 81 && target < 0; ++i)
        if (cells_[i].kind == CellKind::Empty) target = i;
    if (target < 0) return cs;  // board complete: nothing to do

    Cell& c = cells_[target];
    int digit = puzzle_.solution[target];
    c.kind = CellKind::Entry;
    c.value = static_cast<uint8_t>(digit);
    c.marks = 0;
    c.fromHint = true;
    ++hintsUsed_;
    cs.cells.push_back(target);
    cleanupPeerMarks(cells_, target, digit, cs);
    return cs;
}

bool Session::isError(int i) const {
    const Cell& c = cells_[i];
    return c.kind == CellKind::Entry && c.value != puzzle_.solution[i];
}

bool Session::isComplete() const {
    for (int i = 0; i < 81; ++i)
        if (cells_[i].value != puzzle_.solution[i]) return false;
    return true;
}

std::string Session::toJson(const std::string& savedAtIso) const {
    json j;
    j["schemaVersion"] = 1;
    if (!savedAtIso.empty()) j["savedAt"] = savedAtIso;

    std::string givens(81, '0'), solution(81, '0');
    for (int i = 0; i < 81; ++i) {
        givens[i] = static_cast<char>('0' + puzzle_.givens[i]);
        solution[i] = static_cast<char>('0' + puzzle_.solution[i]);
    }
    j["puzzle"] = {{"difficulty", difficultyName(puzzle_.difficulty)},
                   {"seed", puzzle_.seed},
                   {"givens", givens},
                   {"solution", solution}};
    j["elapsedSeconds"] = elapsedSeconds_;
    j["hintsUsed"] = hintsUsed_;

    json cells = json::array();
    for (int i = 0; i < 81; ++i) {
        const Cell& c = cells_[i];
        if (c.kind == CellKind::Entry) {
            json e = {{"i", i}, {"v", c.value}};
            if (c.fromHint) e["hint"] = true;
            cells.push_back(std::move(e));
        } else if (c.kind == CellKind::Empty && c.marks) {
            json m = json::array();
            for (int d = 1; d <= 9; ++d)
                if (c.hasMark(d)) m.push_back(d);
            cells.push_back(json{{"i", i}, {"m", std::move(m)}});
        }
    }
    j["cells"] = std::move(cells);
    return j.dump();
}

namespace {
[[noreturn]] void invalid(const char* why) {
    throw std::runtime_error(std::string("invalid save: ") + why);
}

Grid parseGridString(const json& v, const char* what, bool allowZero) {
    if (!v.is_string()) invalid(what);
    const std::string& s = v.get_ref<const std::string&>();
    if (s.size() != 81) invalid(what);
    Grid g{};
    for (int i = 0; i < 81; ++i) {
        char ch = s[i];
        if (ch < '0' || ch > '9' || (!allowZero && ch == '0')) invalid(what);
        g[i] = static_cast<uint8_t>(ch - '0');
    }
    return g;
}
}  // namespace

Session Session::fromJson(const std::string& text) {
    json j = json::parse(text);  // throws json::parse_error (a std::exception)
    if (!j.is_object()) invalid("not an object");
    if (j.value("schemaVersion", 0) != 1) invalid("schemaVersion");

    const json& jp = j.at("puzzle");
    Puzzle p;
    if (!difficultyFromString(jp.at("difficulty").get<std::string>(), p.difficulty))
        invalid("difficulty");
    p.seed = jp.at("seed").get<uint64_t>();
    p.givens = parseGridString(jp.at("givens"), "givens", true);
    p.solution = parseGridString(jp.at("solution"), "solution", false);

    if (!isSolvedGrid(p.solution)) invalid("solution not a valid grid");
    for (int i = 0; i < 81; ++i)
        if (p.givens[i] && p.givens[i] != p.solution[i]) invalid("givens contradict solution");

    Session s(p);

    const json& je = j.at("elapsedSeconds");
    if (!je.is_number_unsigned()) invalid("elapsedSeconds");
    s.elapsedSeconds_ = je.get<uint32_t>();
    const json& jh = j.at("hintsUsed");
    if (!jh.is_number_unsigned() || jh.get<uint64_t>() > 255) invalid("hintsUsed");
    s.hintsUsed_ = static_cast<uint8_t>(jh.get<uint32_t>());

    const json& jc = j.at("cells");
    if (!jc.is_array()) invalid("cells");
    for (const json& e : jc) {
        if (!e.is_object()) invalid("cell entry");
        int i = e.at("i").get<int>();
        if (i < 0 || i > 80) invalid("cell index");
        if (p.givens[i]) invalid("entry at a given index");
        bool hasV = e.contains("v"), hasM = e.contains("m");
        if (hasV == hasM) invalid("cell must have exactly one of v/m");
        Cell& c = s.cells_[i];
        if (c.kind != CellKind::Empty || c.marks) invalid("duplicate cell index");
        if (hasV) {
            int v = e.at("v").get<int>();
            if (v < 1 || v > 9) invalid("cell value");
            c.kind = CellKind::Entry;
            c.value = static_cast<uint8_t>(v);
            c.fromHint = e.value("hint", false);
        } else {
            for (const json& dm : e.at("m")) {
                int d = dm.get<int>();
                if (d < 1 || d > 9) invalid("mark digit");
                c.marks |= static_cast<uint16_t>(1u << (d - 1));
            }
            if (!c.marks) invalid("empty mark list");
        }
    }
    return s;
}

}  // namespace sudoku::core
