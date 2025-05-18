#include "app_system_init.hpp"
#include "itf_board.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "appSystemInitTask";

void appSystemInitTask(void *arg) {
    /* Get necessary objects from board interface*/
    ILedMatrixDisplay *display = Board_getDisplay();
    if (display == nullptr) {
        ESP_LOGE(TAG, "display create failed");
        while (1);
    }

    if (display->init({5, 5}) != ESP_OK) {
        ESP_LOGE(TAG, "display init failed");
        while (1);
    }

    /* Periodic system service*/
    while (1) {

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
