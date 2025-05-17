#pragma once

#include <cstddef>

#include "color.hpp"
#include "esp_err.h"

class ILedMatrixDisplay {
public:
    virtual ~ILedMatrixDisplay() = default;

    typedef struct {
        std::size_t x;
        std::size_t y;
    } resolution_t;

    // Should initialize periphery interface if required
    virtual esp_err_t init(const resolution_t& resolution) = 0;
 
    virtual resolution_t getResolution(void) const = 0;

    typedef struct {
        std::size_t x;
        std::size_t y;
    } point_t;

    virtual esp_err_t drawPixel(const point_t& point, const color::CRGB& color) = 0;
    virtual esp_err_t clear(void) = 0;

    virtual bool isSupportBrightnessControl() const = 0;
    virtual esp_err_t setBrightness(const uint8_t level) = 0;
};
