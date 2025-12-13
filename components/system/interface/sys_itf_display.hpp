#pragma once

#include "esp_err.h"
#include "color.hpp"

namespace sys {

namespace display {

struct Resolution {
    std::size_t width;
    std::size_t height;

    constexpr std::size_t totalPixels() const {
        return width * height;
    }
};

struct Point {
    std::size_t x;
    std::size_t y;

    constexpr bool within(const Resolution &bounds) const {
        return x < bounds.width && y < bounds.height;
    }
};

} // namespace display

namespace itf {

class IDisplay {
public:
    virtual ~IDisplay() = default;

    virtual esp_err_t drawPixel(const display::Point &point, const color::CRGB &color) = 0;
    virtual esp_err_t clear() = 0;
    virtual esp_err_t setBrightness(uint8_t level) = 0;

    display::Resolution getResolution() const {
        return resolution_;
    }

protected:
    explicit IDisplay(display::Resolution resolution)
      : resolution_(resolution) {
    }

    display::Resolution resolution_;
};

} // namespace itf

} // namespace system