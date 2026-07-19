#pragma once
#include <string>

#include "platform/canvas_renderer.h"

namespace sudoku {

// Device backend: the shared canvas is pushed to the e-ink framebuffer via
// FBInk (fbink_print_raw_data + fbink_refresh). Owns the waveform choice and
// the ghosting policy: after kGhostingPartials partial refreshes, the next
// flushPartial is promoted to a flashing full refresh (FR-016, T025/T052).
class FbinkRenderer : public CanvasRenderer {
public:
    ~FbinkRenderer() override;

    bool init(const std::string& assetsDir);

    DisplayInfo info() const override { return info_; }
    void flushPartial(Rect r) override;
    void flushFull() override;
    void setGhostingInterval(int n) override;

private:
    void pushRegion(Rect r, bool flash);

    int fbfd_ = -1;
    DisplayInfo info_{};
    int partialCount_ = 0;
    int ghostingPartials_ = 12;  // set from Settings at startup (T052)
};

}  // namespace sudoku
