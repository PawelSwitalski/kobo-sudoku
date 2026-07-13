#include "platform/kobo/fbink_renderer.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "fbink.h"

namespace sudoku {

namespace {
FBInkConfig baseConfig() {
    FBInkConfig cfg{};
    cfg.is_quiet = true;
    return cfg;
}
}  // namespace

FbinkRenderer::~FbinkRenderer() {
    if (fbfd_ >= 0) {
        // Leave the framebuffer as Nickel expects it (FR-017).
        fbink_close(fbfd_);
    }
}

bool FbinkRenderer::init(const std::string& assetsDir) {
    FBInkConfig cfg = baseConfig();
    fbfd_ = fbink_open();
    if (fbfd_ < 0) {
        std::fprintf(stderr, "fbink_open failed\n");
        return false;
    }
    if (fbink_init(fbfd_, &cfg) != EXIT_SUCCESS) {
        std::fprintf(stderr, "fbink_init failed\n");
        return false;
    }

    FBInkState state{};
    fbink_get_state(&cfg, &state);
    info_.width = static_cast<int>(state.view_width);
    info_.height = static_cast<int>(state.view_height);
    info_.dpi = static_cast<int>(state.screen_dpi);
    // Color panels (Kaleido: Libra/Clara Colour) run a 32bpp fb under Nickel;
    // mono devices run 8bpp. Verified on device in T030.
    info_.color = state.bpp >= 24;

    if (const char* n = getenv("SUDOKU_GHOSTING_N"); n && *n) {
        int v = std::atoi(n);
        if (v > 0) ghostingPartials_ = v;  // field tuning without rebuild (T052)
    }

    return canvas_.init(info_.width, info_.height, assetsDir + "/DejaVuSans.ttf",
                        assetsDir + "/DejaVuSans-Bold.ttf");
}

void FbinkRenderer::pushRegion(Rect r, bool flash) {
    // Clamp to the canvas.
    if (r.x < 0) { r.w += r.x; r.x = 0; }
    if (r.y < 0) { r.h += r.y; r.y = 0; }
    if (r.x + r.w > canvas_.width()) r.w = canvas_.width() - r.x;
    if (r.y + r.h > canvas_.height()) r.h = canvas_.height() - r.y;
    if (r.w <= 0 || r.h <= 0) return;

    // Sub-image copy of the dirty region (RGBA).
    std::vector<unsigned char> sub(static_cast<size_t>(r.w) * r.h * 4);
    const uint8_t* src = canvas_.pixels();
    for (int y = 0; y < r.h; ++y)
        std::memcpy(sub.data() + static_cast<size_t>(y) * r.w * 4,
                    src + ((static_cast<size_t>(r.y + y) * canvas_.width()) + r.x) * 4,
                    static_cast<size_t>(r.w) * 4);

    FBInkConfig cfg = baseConfig();
    cfg.no_refresh = true;  // draw first, refresh explicitly below
    cfg.ignore_alpha = true;
    if (fbink_print_raw_data(fbfd_, sub.data(), r.w, r.h, sub.size(),
                             static_cast<short>(r.x), static_cast<short>(r.y),
                             &cfg) != EXIT_SUCCESS) {
        std::fprintf(stderr, "fbink_print_raw_data failed\n");
        return;
    }

    cfg.no_refresh = false;
    cfg.is_flashing = flash;
    cfg.wfm_mode = flash ? WFM_GC16 : WFM_AUTO;
    fbink_refresh(fbfd_, static_cast<uint32_t>(r.y), static_cast<uint32_t>(r.x),
                  static_cast<uint32_t>(r.w), static_cast<uint32_t>(r.h), &cfg);
}

void FbinkRenderer::flushPartial(Rect r) {
    if (++partialCount_ >= ghostingPartials_) {
        // Ghosting policy: promote to a full flashing refresh (Constitution II).
        flushFull();
        return;
    }
    pushRegion(r, false);
}

void FbinkRenderer::flushFull() {
    partialCount_ = 0;
    pushRegion({0, 0, canvas_.width(), canvas_.height()}, true);
}

}  // namespace sudoku
