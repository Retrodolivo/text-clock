#include "app_system_init.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


void appSystemInitTask(void *arg) {

    /* Periodic system service*/
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
