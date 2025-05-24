#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "itf_display.hpp"
#include "itf_board.hpp"

#define APPLICATION_TASK_STACK_SIZE     (3 * 1024)

static const char *TAG = "application";

void ApplicationTask(void *arg);

esp_err_t ApplicationInit(void) {
    if (xTaskCreate(ApplicationTask, "applicationTask", APPLICATION_TASK_STACK_SIZE, NULL, 5, NULL) != pdPASS) {
        ESP_LOGE(TAG, "application task creation failed (insufficient heap?)");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "inited");
    return ESP_OK;
}


void ApplicationTask(void *arg) {
    /**
     *  At this point system components must be initialized
     */

    ILedMatrixDisplay *display = Board_getDisplay();

    

    while (1) {
        vTaskDelay(portMAX_DELAY);
    }
}