#include "platform/kobo/evdev_touch.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fcntl.h>
#include <linux/input.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace sudoku {

namespace {
bool hasAbsAxis(int fd, unsigned axis) {
    unsigned char bits[(ABS_MAX + 7) / 8] = {};
    if (ioctl(fd, EVIOCGBIT(EV_ABS, sizeof bits), bits) < 0) return false;
    return bits[axis / 8] & (1u << (axis % 8));
}

bool envFlag(const char* name) {
    const char* v = getenv(name);
    return v && *v && *v != '0';
}
}  // namespace

EvdevTouch::~EvdevTouch() {
    if (fd_ >= 0) close(fd_);
}

bool EvdevTouch::init(const DisplayInfo& display) {
    viewW_ = display.width;
    viewH_ = display.height;
    swapXY_ = envFlag("SUDOKU_TOUCH_SWAP_XY");
    mirrorX_ = envFlag("SUDOKU_TOUCH_MIRROR_X");
    mirrorY_ = envFlag("SUDOKU_TOUCH_MIRROR_Y");
    debug_ = envFlag("SUDOKU_TOUCH_DEBUG");

    for (int i = 0; i < 12; ++i) {
        char path[32];
        std::snprintf(path, sizeof path, "/dev/input/event%d", i);
        int fd = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
        if (fd < 0) continue;
        bool mt = hasAbsAxis(fd, ABS_MT_POSITION_X);
        bool st = hasAbsAxis(fd, ABS_X);
        if (!mt && !st) {
            close(fd);
            continue;
        }
        unsigned xAxis = mt ? ABS_MT_POSITION_X : ABS_X;
        unsigned yAxis = mt ? ABS_MT_POSITION_Y : ABS_Y;
        input_absinfo ax{}, ay{};
        if (ioctl(fd, EVIOCGABS(xAxis), &ax) < 0 || ioctl(fd, EVIOCGABS(yAxis), &ay) < 0) {
            close(fd);
            continue;
        }
        fd_ = fd;
        rawMinX_ = ax.minimum; rawMaxX_ = ax.maximum;
        rawMinY_ = ay.minimum; rawMaxY_ = ay.maximum;
        if (debug_)
            std::fprintf(stderr, "touch: %s mt=%d x[%d..%d] y[%d..%d]\n", path, mt, rawMinX_,
                         rawMaxX_, rawMinY_, rawMaxY_);
        return true;
    }
    std::fprintf(stderr, "touch: no touchscreen found under /dev/input\n");
    return false;
}

std::optional<Tap> EvdevTouch::waitForTap(int timeoutMs) {
    int rawX = -1, rawY = -1;
    bool sawContact = false, lifted = false;

    while (true) {
        pollfd pfd{fd_, POLLIN, 0};
        int rc = poll(&pfd, 1, timeoutMs);
        if (rc <= 0) return std::nullopt;  // timeout, or EINTR (SIGTERM path)

        input_event ev[32];
        ssize_t n = read(fd_, ev, sizeof ev);
        if (n <= 0) return std::nullopt;

        for (size_t k = 0; k < static_cast<size_t>(n) / sizeof(input_event); ++k) {
            const input_event& e = ev[k];
            if (e.type == EV_ABS) {
                switch (e.code) {
                    case ABS_MT_POSITION_X: case ABS_X: rawX = e.value; sawContact = true; break;
                    case ABS_MT_POSITION_Y: case ABS_Y: rawY = e.value; sawContact = true; break;
                    case ABS_MT_TRACKING_ID:
                        if (e.value < 0) lifted = true;
                        else sawContact = true;
                        break;
                    default: break;
                }
            } else if (e.type == EV_KEY && e.code == BTN_TOUCH) {
                if (e.value == 0) lifted = true;
                else sawContact = true;
            } else if (e.type == EV_SYN && e.code == SYN_REPORT) {
                if (lifted && sawContact && rawX >= 0 && rawY >= 0) {
                    int x = rawX, y = rawY;
                    int maxX = rawMaxX_, minX = rawMinX_, maxY = rawMaxY_, minY = rawMinY_;
                    if (swapXY_) {
                        int t = x; x = y; y = t;
                        t = maxX; maxX = maxY; maxY = t;
                        t = minX; minX = minY; minY = t;
                    }
                    int px = (maxX > minX)
                                 ? static_cast<int>(static_cast<long long>(x - minX) *
                                                    (viewW_ - 1) / (maxX - minX))
                                 : x;
                    int py = (maxY > minY)
                                 ? static_cast<int>(static_cast<long long>(y - minY) *
                                                    (viewH_ - 1) / (maxY - minY))
                                 : y;
                    if (mirrorX_) px = viewW_ - 1 - px;
                    if (mirrorY_) py = viewH_ - 1 - py;
                    if (debug_)
                        std::fprintf(stderr, "tap raw=(%d,%d) -> (%d,%d)\n", rawX, rawY, px, py);
                    return Tap{px, py};
                }
                lifted = false;
            }
        }
    }
}

}  // namespace sudoku
