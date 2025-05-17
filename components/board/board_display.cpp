#include "board_display.hpp"
#include "esp_log.h"
#include "esp_check.h"

#define DISPLAY_CONN_PIN  GPIO_NUM_23

static const char *TAG = "board_display";

esp_err_t TextClockDisplay::init(const ILedMatrixDisplay::resolution_t& resolution) {
    ledStrip_ = new AddresableLED<LedType::WS2812B>(resolution.x * resolution.y, DISPLAY_CONN_PIN);
    ESP_RETURN_ON_FALSE(ledStrip_, ESP_FAIL, TAG, "Failed to create ledstrip");
    
    ESP_RETURN_ON_ERROR(ledStrip_->update(), TAG, "Failed to update ledstrip buffer");

    resolution_ = resolution;
    ESP_LOGI(TAG, "inited with %dx%d resolution", resolution_.x, resolution_.y);

    return ESP_OK;
}

esp_err_t TextClockDisplay::drawPixel(const point_t& point, const color::CRGB& color) {
    return ESP_OK;
}

esp_err_t TextClockDisplay::clear(void) {
    return ESP_OK;
}