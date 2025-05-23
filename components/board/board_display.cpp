#include "board_display.hpp"
#include "esp_log.h"
#include "esp_check.h"

#define DISPLAY_CONN_PIN  GPIO_NUM_23

static const char *TAG = "board_display";

esp_err_t TextClockDisplay::init(const ILedMatrixDisplay::resolution_t& resolution) {
    ledStrip_ = new AddresableLED<LedType::WS2812B>(resolution.x * resolution.y, DISPLAY_CONN_PIN);
    ESP_RETURN_ON_FALSE(ledStrip_, ESP_FAIL, TAG, "failed to create ledstrip");
    
    ESP_RETURN_ON_ERROR(ledStrip_->update(), TAG, "failed to update ledstrip buffer");

    resolution_ = resolution;
    isInited_ = true;
    ESP_LOGI(TAG, "init: inited with %dx%d resolution", resolution_.x, resolution_.y);

    return ESP_OK;
}

esp_err_t TextClockDisplay::drawPixel(const point_t& point, const color::CRGB& color) {
    ESP_RETURN_ON_FALSE(isInited_, ESP_FAIL, TAG, "drawPixel: not inited");

    if (point.x > resolution_.x || point.y > resolution_.y) {
        ESP_LOGE(TAG, "drawPixel: x:%d,y:%d - no such point", point.x, point.y);
        return ESP_ERR_INVALID_ARG;
    }

    const size_t LedsInRows = (point.y + 1) * resolution_.x;
    const size_t LedsCount = (point.y % 2) ? LedsInRows - point.x - 1 : point.x;

    ESP_RETURN_ON_ERROR(ledStrip_->setColor(color, LedsCount), TAG, "drawPixel: failed to set");
    ESP_RETURN_ON_ERROR(ledStrip_->update(), TAG, "drawPixel: failed to update led strip buffer");

    ESP_LOGI(TAG, "drawPixel: point{%d,%d} set up", point.x, point.y);
    return ESP_OK;
}

esp_err_t TextClockDisplay::clear(void) {
    ESP_RETURN_ON_FALSE(isInited_, ESP_FAIL, TAG, "clear: not inited");

    ledStrip_->clear();
    ESP_RETURN_ON_ERROR(ledStrip_->update(), TAG, "clear: failed to update led strip buffer");

    return ESP_OK;
}

ILedMatrixDisplay::resolution_t TextClockDisplay::getResolution(void) const {
    if (!isInited_) {
        ESP_LOGE(TAG, "getResolution: not inited");
    }

    return resolution_;
}

esp_err_t TextClockDisplay::setBrightness(const uint8_t level) {
    ESP_RETURN_ON_FALSE(isSupportBrightnessControl(), ESP_FAIL, TAG, "setBrightness: not supported");

    ledStrip_->setBrightness(level);

    return ESP_OK;
}