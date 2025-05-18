#pragma once

#include "itf_display.hpp"
#include "addressable_led.hpp"
#include "esp_err.h"

class TextClockDisplay : public ILedMatrixDisplay {
public:
    TextClockDisplay() = default;
    ~TextClockDisplay() { delete ledStrip_; }

    esp_err_t init(const ILedMatrixDisplay::resolution_t& resolution) override;
    
    resolution_t getResolution() const {
        return resolution_;
    }
    
    esp_err_t drawPixel(const point_t& point, const color::CRGB& color);

    esp_err_t clear(void);

    bool isSupportBrightnessControl(void) const override {
        return true;
    }

    esp_err_t setBrightness(const uint8_t level) {
        return ESP_ERR_NOT_SUPPORTED;
    }

private:
    bool isInited_ = false;
    ILedMatrixDisplay::resolution_t resolution_;
    AddresableLED<LedType::WS2812B> *ledStrip_;
};