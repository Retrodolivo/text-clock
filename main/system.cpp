#include "system.hpp"
#include "itf_board.hpp"
#include "itf_wifi.hpp"
#include "application.hpp"
#include "nettime.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"

static const char *TAG = "systemTask";

#define WIFI_SSID       "Retrolink2"
#define WIFI_PASSWORD   "Thunder_Bolt1"

static void systemWifiFail_Callback(WifiFailEvents event);

const itf_wifi_config_t wifiConfig = {
    .ssid = WIFI_SSID,
    .password = WIFI_PASSWORD,
    .failCallBack = systemWifiFail_Callback,
};

void systemTask(void *arg) {
    /* Initialize flash for storing credentials*/
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ILedMatrixDisplay *display = Board_getDisplay();
    if (display == nullptr) {
        ret = ESP_FAIL;
        ESP_ERROR_CHECK(ret);
    }

    ret = display->init({16, 16});
    ESP_ERROR_CHECK(ret);

    ret = Board_wifiInit();
    ESP_ERROR_CHECK(ret);

    if (Board_wifiConnect(wifiConfig, pdMS_TO_TICKS(5000)) == ESP_OK) {
        NetTime::init(); //< trying to connect to NTP server
        
    }

    ret = ApplicationInit();
    ESP_ERROR_CHECK(ret);

    /* Periodic system service*/
    while (1) {
        if (NetTime::isInited() && NetTime::isSynced()) {
            const auto timeStr = NetTime::getLocalTimeString("%Y-%m-%d %H:%M:%S");
            ESP_LOGI(TAG, "%s", timeStr.c_str());
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void systemWifiFail_Callback(WifiFailEvents event) {
    return;
}