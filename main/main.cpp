#include "app_system_init.hpp"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#define APP_SYSTEM_INIT_TASK_STACK_SIZE  (1 * 1024)


extern "C" void app_main() {
    if (xTaskCreate(appSystemInitTask, "appSystemInitTask", APP_SYSTEM_INIT_TASK_STACK_SIZE, NULL, 5, NULL) != pdPASS) {
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
