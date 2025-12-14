#include "itf_app.hpp"
#include "board_clock.hpp"
#include "board_display.hpp"
#include "board_wifi.hpp"
#include "sys_service_application.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "systemTask";

void systemTask(void *arg) {
    /* Create network clock instance with default configuration */
    static board::NetworkClock clock;

    /* Create led matrix display with default configuration */
    static board::LedMatrix display;

    /* Create esp32 wifi instance */
    static board::WifiEsp32 network;


    /* Create text clock application with specified board components */
    static app::TextClockApp clockApplication(clock, display, network);


    /* Periodic system service*/
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}