#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "render.hpp"
#include "esp_log.h"

static const char *TAG = "render_service";
static constexpr int TaskPrio = 5;
static constexpr uint32_t TaskStackSize = 3 * 1024;

namespace app {

bool Render::startRenderTo(sys::itf::IDisplay *display) {
    display_ = display;
    return start();
}

bool Render::start() {
    if (taskHandle_) {
        return true; //< Already started
    }

    if (xTaskCreate(serviceTask, "RenderServiceTask", TaskStackSize, this, TaskPrio,
                    static_cast<TaskHandle_t *>(taskHandle_)) != pdPASS) {
        ESP_LOGE("%s", "task creation failed", __func__);
        return false;
    }

    return true;
}

void Render::serviceTask(void *arg) {
    Render *instance = static_cast<Render *>(arg);
    if (instance == nullptr) {
        vTaskDelete(NULL);
    }

    /* Get display to work with */
    sys::itf::IDisplay *display = instance->display_;

    while (1) {
        display->drawPixel({2, 0}, color::CRGB::Blue);
        vTaskDelay(500);
        display->drawPixel({2, 0}, color::CRGB::Red);
        vTaskDelay(500);
    }
}

} // namespace app