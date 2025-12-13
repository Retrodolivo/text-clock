#include "board_display.hpp"
#include "addressable_led.hpp"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_check.h"


namespace board {

static const char *TAG = "board_display";

class LedMatrix::Impl {
public:
    Impl(const sys::display::Resolution &resolution, int gpioNum)
      : resolution_(resolution)
      , ledStrip_(std::make_unique<AddresableLED<LedType::WS2812B>>(resolution.totalPixels(),
                                                                    static_cast<gpio_num_t>(gpioNum))) {
        static StaticSemaphore_t mutexMem;
        accessMutex_ = xSemaphoreCreateMutexStatic(&mutexMem);
    }

    esp_err_t drawPixel(const sys::display::Point &point, const color::CRGB &color) {
        if (!point.within(resolution_)) {
            return ESP_ERR_INVALID_ARG;
        }

        const size_t LedsInRows = (point.y + 1) * resolution_.width;
        const size_t LedsCount = (point.y % 2) ? LedsInRows - point.x - 1 : point.x;

        if (lock()) {
            ESP_RETURN_ON_ERROR(ledStrip_->setColor(color, LedsCount), TAG, "%s: failed to set", __func__);
            ESP_RETURN_ON_ERROR(ledStrip_->update(), TAG, "%s: failed to update led strip buffer", __func__);
            ESP_LOGI(TAG, "%s: point{%d,%d} set up", __func__, point.x, point.y);            
            
            unlock();
            return ESP_OK;
        }

        return ESP_ERR_TIMEOUT;
    }

    esp_err_t clear() {
        if (lock()) {
            ledStrip_->clear();
            ESP_RETURN_ON_ERROR(ledStrip_->update(), TAG, "%s: failed to update led strip buffer", __func__);
            
            unlock();
            return ESP_OK;
        }

        return ESP_ERR_TIMEOUT;
    }

    esp_err_t setBrightness(uint8_t level) {
        if (lock()) {

            ledStrip_->setBrightness(level);

            unlock();
            return ESP_OK;
        }

        return ESP_ERR_TIMEOUT;
    }

private:
    sys::display::Resolution resolution_;
    std::unique_ptr<AddresableLED<LedType::WS2812B>> ledStrip_;
    SemaphoreHandle_t accessMutex_ = nullptr;

    bool lock() {
        return xSemaphoreTake(accessMutex_, portMAX_DELAY) == pdTRUE;
    }

    void unlock() {
        xSemaphoreGive(accessMutex_);
    }
};


/**
 * @brief Use of pImpl
 */

LedMatrix::LedMatrix(const sys::display::Resolution &resolution, int gpioNum)
  : IDisplay(resolution)
  , pImpl_(std::make_unique<Impl>(resolution_, gpioNum)) {
}

LedMatrix::~LedMatrix() = default;

esp_err_t LedMatrix::drawPixel(const sys::display::Point &point, const color::CRGB &color) {
    return pImpl_->drawPixel(point, color);
}

esp_err_t LedMatrix::clear() {
    return pImpl_->clear();
}

esp_err_t LedMatrix::setBrightness(uint8_t level) {
    return pImpl_->setBrightness(level);
}

} // namespace board