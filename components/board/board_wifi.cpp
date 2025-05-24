#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include <cstring> // for memcpy
#include <inttypes.h>
#include <string>

#include "itf_wifi.hpp"
#include "esp_check.h"
#include "esp_bit_defs.h"
#include "esp_wifi.h"

#define WIFI_CONNECTED_FLAG BIT0
#define WIFI_FAILED_FLAG    BIT1

typedef struct {
    esp_netif_t *netif;
    bool isUserRequest;
    bool isInited;
    void (*failCallBack)(WifiFailEvents event);
} wifi_context_t;

static const char *TAG = "board_wifi";
static wifi_context_t gContext;
static EventGroupHandle_t gWifiEventGroup;

static void eventHandler(void *arg, esp_event_base_t eventBase, int32_t eventId, void *eventData) {
    if (eventBase == WIFI_EVENT) {
        switch (eventId) {
            case WIFI_EVENT_STA_START:
                esp_wifi_connect();
                break;

            case WIFI_EVENT_STA_DISCONNECTED: {
                wifi_event_sta_disconnected_t* disconEvent = static_cast<wifi_event_sta_disconnected_t*>(eventData);
                std::string reasonStr;
                
                /* Fail bit rise in case or NON user initiated disconnects*/
                if (gContext.isUserRequest) {
                    reasonStr = "User initiated";
                    gContext.isUserRequest = false;
                } else {
                    switch (disconEvent->reason) {
                        case WIFI_REASON_NO_AP_FOUND:
                            reasonStr = "SSID not found";
                            gContext.failCallBack(WifiFailEvents::FAIL_TO_CONNECT);
                            break;
                        case WIFI_REASON_BEACON_TIMEOUT:
                            reasonStr = "Beacon timeout";
                            gContext.failCallBack(WifiFailEvents::BEACON_TIMEOUT);
                            break;
                        default:
                            reasonStr = "Unknown reason";
                            gContext.failCallBack(WifiFailEvents::FAIL_UNKNOWN);
                            break;
                    }
                    xEventGroupSetBits(gWifiEventGroup, WIFI_FAILED_FLAG);
                }

                ESP_LOGW(TAG, "wifi event handler: disconnected from AP (reason: #%d - %s)", disconEvent->reason, reasonStr.c_str());
                xEventGroupClearBits(gWifiEventGroup, WIFI_CONNECTED_FLAG);
                break;
            }

            case WIFI_EVENT_STA_CONNECTED: {
                wifi_event_sta_connected_t* connEvent = static_cast<wifi_event_sta_connected_t*>(eventData);
                ESP_LOGI(TAG, "wifi event handler: connected to AP: %s , channel: %d)", connEvent->ssid, connEvent->channel);
                break;
            }
            
            default:
                break;
        }
    } else if (eventBase == IP_EVENT) {
        switch (eventId) {
            case IP_EVENT_STA_GOT_IP: {
                ip_event_got_ip_t* gotIpEvent = static_cast<ip_event_got_ip_t*>(eventData);
                ESP_LOGI(TAG, "wifi event handler: got IP: " IPSTR ", gateway: " IPSTR ", netmask: " IPSTR,
                       IP2STR(&gotIpEvent->ip_info.ip),
                       IP2STR(&gotIpEvent->ip_info.gw),
                       IP2STR(&gotIpEvent->ip_info.netmask));
                xEventGroupSetBits(gWifiEventGroup, WIFI_CONNECTED_FLAG);
                break;
            }
            
            default:
                break;
        }
    }
}

esp_err_t Board_wifiInit(void) {
    if (Board_wifiIsInited()) {
        ESP_LOGW(TAG, "init: already initialized");
        return ESP_FAIL;
    }

    gWifiEventGroup = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    gContext.netif = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &eventHandler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &eventHandler,
                                                        NULL,
                                                        &instance_got_ip));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    ESP_LOGI(TAG, "init: wifi successfully inited");

    gContext.isInited = true;

    return ESP_OK;
}

esp_err_t Board_wifiDeinit(void) {
    if (!Board_wifiIsInited()) {
        ESP_LOGW(TAG, "deinit: wifi not even initialized");
        return ESP_OK;
    }

    esp_err_t result = Board_wifiDisconnect();
    if (result != ESP_OK) {
        return result;
    }

    result = esp_wifi_deinit();
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "deinit: failed to deinit");
        return result;
    }

    ESP_LOGI(TAG, "deinit: successufully deinited");
    return ESP_OK;
}

esp_err_t Board_wifiConnect(const itf_wifi_config_t& config, uint32_t timeoutMs) {
    if (Board_wifiIsConnected()) {
        ESP_LOGW(TAG, "connect: already connected. Disconnect first");
        return ESP_ERR_INVALID_STATE;
    }
    
    /* Register callback. Use it in event handler*/
    gContext.failCallBack = config.failCallBack;

    wifi_config_t wifiConfig {};
    wifiConfig.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifiConfig.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;
    std::memcpy(wifiConfig.sta.ssid, config.ssid, sizeof(config.ssid));
    std::memcpy(wifiConfig.sta.password, config.password, sizeof(config.password));

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiConfig));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "connect: wifi started");

    /* Waiting until either the connection is established (WIFI_CONNECTED_FLAG) or connection failed for the maximum
     * number of retries (WIFI_FAILED_FLAG). The bits are set by eventHandler() */
    const EventBits_t flags = xEventGroupWaitBits(gWifiEventGroup, WIFI_CONNECTED_FLAG | WIFI_FAILED_FLAG, pdFALSE, pdFALSE, pdMS_TO_TICKS(timeoutMs));

    if ((flags & (WIFI_CONNECTED_FLAG | WIFI_FAILED_FLAG)) == 0) {
        /* No flags - timeout occurs*/
        ESP_LOGE(TAG, "connect: connection timeout after %" PRIu32 " ms", timeoutMs);
        return ESP_ERR_TIMEOUT;
    }
    if (flags & WIFI_CONNECTED_FLAG) {
        ESP_LOGI(TAG, "connect: connected to AP SSID:%s", config.ssid);
        return ESP_OK;
    } else if (flags & WIFI_FAILED_FLAG) {
        ESP_LOGE(TAG, "connect: failed to connect to AP SSID:%s", config.ssid);
        return ESP_FAIL;
    } else {
        ESP_LOGE(TAG, "connect: unexpected event");
        return ESP_FAIL;        
    }
}

esp_err_t Board_wifiDisconnect(void) {
    if (!Board_wifiIsConnected()) {
        ESP_LOGI(TAG, "disconnect: not connected yet");
        return ESP_ERR_INVALID_STATE;
    }

    gContext.isUserRequest = true;

    const esp_err_t err = esp_wifi_disconnect();
    if (err == ESP_ERR_WIFI_NOT_INIT) {
        ESP_LOGW(TAG, "disconnect: wifi not initialized when trying to disconnect");
    } else if (err != ESP_OK) {
        ESP_LOGE(TAG, "disconnect: failed to disconnect wifi");
    } else {
        ESP_LOGI(TAG, "disconnect: wifi disconnection initiated");
    }

    ESP_ERROR_CHECK(esp_wifi_stop());


    return err;
}


bool Board_wifiIsInited(void) {
    return gContext.isInited;
}

bool Board_wifiIsConnected(void) {
    if (!gContext.isInited) {
        return false;
    }
    return esp_netif_is_netif_up(gContext.netif);   
}
