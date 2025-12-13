#include "sys_service_application.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "systemTask";

void systemTask(void *arg) {

    /* Periodic system service*/
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}