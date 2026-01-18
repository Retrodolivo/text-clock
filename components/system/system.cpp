#include "itf_app.hpp"
#include "board_clock.hpp"
#include "board_display.hpp"
#include "board_wifi.hpp"
#include "sys_service_application.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "systemTask";
static constexpr uint32_t APP_STACK_SIZE = 3 * 1024;

void systemTask(void *arg) {
    /* Create network clock instance with default configuration */
    board::NetworkClock clock("UTC3");
    /* Create led matrix display with default configuration */
    board::LedMatrix display;
    /* Create esp32 wifi instance */
    board::WifiEsp32 network;

    /* Create text clock application with specified board components */
    app::TextClockApp clockApplication(clock, display, network);

    using namespace sys::service;
    /* Create and run the app specific thread to control the flow of app */
    AppThread appThread(clockApplication, APP_STACK_SIZE);
    appThread.commandInit();

    /* Periodic system service*/
    while (1) {
        const AppThread::State appState = appThread.getState();
        if (appState != AppThread::State::WORK) {
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}