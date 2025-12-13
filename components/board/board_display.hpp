#pragma once

#include "sys_itf_display.hpp"
#include "esp_err.h"
#include <memory>

namespace board {

constexpr int DISPLAY_CONN_GPIO = 23;

constexpr sys::display::Resolution RESOLUTION = {
    .width = 16,
    .height = 16,
};

class LedMatrix: public sys::itf::IDisplay {
public:
    LedMatrix(const sys::display::Resolution &resolution = RESOLUTION, int gpioNum = DISPLAY_CONN_GPIO);
    ~LedMatrix() override;

    esp_err_t drawPixel(const sys::display::Point &point, const color::CRGB &color) override;
    esp_err_t clear() override;

    esp_err_t setBrightness(uint8_t level) override;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl_; //< pointer to implementation idiom
};

} // namespace board