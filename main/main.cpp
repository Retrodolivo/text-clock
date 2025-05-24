#include "system.hpp"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#define SYSTEM_TASK_STACK_SIZE  (5 * 1024)


extern "C" void app_main() {
    if (xTaskCreate(systemTask, "systemTask", SYSTEM_TASK_STACK_SIZE, NULL, 5, NULL) != pdPASS) {
        ESP_LOGE("app_main", "app system init task creation failed (insufficient heap?)");
    }

    ESP_LOGI("app_main", "app system init task created");
    /**
     * Do not return from the 'app_main' to make sure 
     * other's created tasks objects won't get freed
     */
    while (1) {
        vTaskDelay(portMAX_DELAY);
    }
}
