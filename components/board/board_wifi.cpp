#include "board_wifi.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <vector>
#include <string>
#include <functional>

static const char *TAG = "WifiEsp32";

/* Map ESP32 authmode to our Security enum */
sys::itf::IWifi::Security espAuthToSecurity(wifi_auth_mode_t auth) {
    switch (auth) {
        case WIFI_AUTH_OPEN:
            return sys::itf::IWifi::Security::OPEN;
        case WIFI_AUTH_WEP:
            return sys::itf::IWifi::Security::WEP;
        case WIFI_AUTH_WPA_PSK:
            return sys::itf::IWifi::Security::WPA_PSK;
        case WIFI_AUTH_WPA2_PSK:
            return sys::itf::IWifi::Security::WPA2_PSK;
        case WIFI_AUTH_WPA_WPA2_PSK:
            return sys::itf::IWifi::Security::WPA_WPA2_PSK;
        case WIFI_AUTH_WPA3_PSK:
            return sys::itf::IWifi::Security::WPA3_PSK;
        default:
            return sys::itf::IWifi::Security::OPEN;
    }
}

namespace board {

/**
 * Implementation of WifiEsp32
 */
class WifiEsp32::Impl {
public:
    mutable SemaphoreHandle_t stateMutex = nullptr;
    sys::itf::IWifi::State currentState = sys::itf::IWifi::State::DISCONNECTED;
    std::string connectedSSID;
    bool autoReconnect = true;

    /* Callbacks */
    sys::itf::IWifi::ConnectionCallback connectionCallback = nullptr;

    /* Stored for reconnection */
    struct {
        std::string ssid;
        std::string password;
    } credentials;

    std::vector<sys::itf::IWifi::NetworkInfo> scannedNetworks;

    /* Esp32 specific event handlers */
    esp_event_handler_instance_t wifiEventHandler = nullptr;
    esp_event_handler_instance_t ipEventHandler = nullptr;

    Impl() {
        stateMutex = xSemaphoreCreateRecursiveMutex();
        if (!stateMutex) {
            ESP_LOGE(TAG, "Failed to create recursive mutex");
        }
    }

    ~Impl() {
        if (stateMutex) {
            vSemaphoreDelete(stateMutex);
        }
    }

    void lock() const {
        if (stateMutex) {
            xSemaphoreTakeRecursive(stateMutex, portMAX_DELAY);
        }
    }

    void unlock() const {
        if (stateMutex) {
            xSemaphoreGiveRecursive(stateMutex);
        }
    }

    void updateState(sys::itf::IWifi::State newState, const std::string &ssid = "") {
        lock();
        currentState = newState;
        if (!ssid.empty()) {
            connectedSSID = ssid;
        }
        unlock();

        /* Notify user if it have to */
        if (connectionCallback) {
            connectionCallback(newState, connectedSSID);
        }
    }
};

static void wifiEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    WifiEsp32 *wifi = static_cast<WifiEsp32 *>(arg);
    if (wifi == nullptr) {
        return;
    }

    WifiEsp32::Impl *pImpl = wifi->getImplForEventHandler();
    if (pImpl == nullptr) {
        return;
    }

    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "WiFi STA started");
                break;

            case WIFI_EVENT_STA_CONNECTED:
                {
                    wifi_event_sta_connected_t *event = static_cast<wifi_event_sta_connected_t *>(event_data);
                    std::string ssid(reinterpret_cast<const char *>(event->ssid));
                    pImpl->updateState(sys::itf::IWifi::State::CONNECTED, ssid);

                    ESP_LOGI(TAG, "Connected to %s", ssid.c_str());
                    break;
                }

            case WIFI_EVENT_STA_DISCONNECTED:
                {
                    wifi_event_sta_disconnected_t *event = static_cast<wifi_event_sta_disconnected_t *>(event_data);
                    std::string ssid(reinterpret_cast<const char *>(event->ssid));
                    pImpl->updateState(sys::itf::IWifi::State::DISCONNECTED, ssid);

                    ESP_LOGW(TAG, "Disconnected from %s, reason: %d", ssid.c_str(), event->reason);
                    /* Do reconnection if it have to */
                    if (pImpl->autoReconnect && !pImpl->credentials.ssid.empty()) {
                        ESP_LOGI(TAG, "Attempting auto-reconnect...");
                        esp_wifi_connect();
                    }
                    break;
                }

            default:
                break;
        }
    } else if (event_base == IP_EVENT) {
        if (event_id == IP_EVENT_STA_GOT_IP) {
            ip_event_got_ip_t *event = static_cast<ip_event_got_ip_t *>(event_data);
            ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        }
    }
}

/**
 * WifiEsp32 implementation
 */
WifiEsp32::WifiEsp32()
  : pImpl_(std::make_unique<Impl>()) {
    // Initialize ESP32 WiFi
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* Register event handlers */
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifiEventHandler, this,
                                                        &pImpl_->wifiEventHandler));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifiEventHandler, this,
                                                        &pImpl_->ipEventHandler));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi initialized");
}

WifiEsp32::~WifiEsp32() {
    disconnect();

    /* Cleanup ESP32 handlers */
    if (pImpl_->wifiEventHandler) {
        esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, pImpl_->wifiEventHandler);
    }
    if (pImpl_->ipEventHandler) {
        esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, pImpl_->ipEventHandler);
    }

    esp_wifi_stop();
    esp_wifi_deinit();

    ESP_LOGI(TAG, "WiFi deinitialized");
}

bool WifiEsp32::connect(const std::string &ssid, const std::string &password) {
    pImpl_->lock();

    if (pImpl_->currentState == State::CONNECTING || pImpl_->currentState == State::CONNECTED) {
        pImpl_->unlock();
        ESP_LOGW(TAG, "Already connecting or connected");
        return false;
    }

    /* save credentials */
    pImpl_->credentials.ssid = ssid;
    pImpl_->credentials.password = password;

    pImpl_->updateState(State::CONNECTING, ssid);
    pImpl_->unlock();

    /* Configure WiFi */
    wifi_config_t wifi_config = {};
    strncpy((char *)wifi_config.sta.ssid, ssid.c_str(), sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, password.c_str(), sizeof(wifi_config.sta.password) - 1);

    esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set config: %s", esp_err_to_name(err));
        pImpl_->updateState(State::ERROR, ssid);
        return false;
    }

    err = esp_wifi_connect();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to connect: %s", esp_err_to_name(err));
        pImpl_->updateState(State::ERROR, ssid);
        return false;
    }

    ESP_LOGI(TAG, "Connecting to %s...", ssid.c_str());
    return true;
}

bool WifiEsp32::disconnect() {
    esp_err_t err = esp_wifi_disconnect();
    if (err == ESP_OK) {
        pImpl_->updateState(State::DISCONNECTED);
        ESP_LOGI(TAG, "Disconnected");
        return true;
    }

    ESP_LOGE(TAG, "Failed to disconnect: %s", esp_err_to_name(err));
    return false;
}

bool WifiEsp32::reconnect() {
    pImpl_->lock();
    if (pImpl_->credentials.ssid.empty()) {
        pImpl_->unlock();
        ESP_LOGW(TAG, "No stored credentials for reconnection");
        return false;
    }
    pImpl_->unlock();

    return connect(pImpl_->credentials.ssid, pImpl_->credentials.password);
}

void WifiEsp32::setAutoReconnect(bool enable) {
    pImpl_->lock();
    pImpl_->autoReconnect = enable;
    pImpl_->unlock();
    ESP_LOGI(TAG, "Auto reconnect: %s", enable ? "enabled" : "disabled");
}

sys::itf::IWifi::State WifiEsp32::getState() const {
    pImpl_->lock();
    State state = pImpl_->currentState;
    pImpl_->unlock();
    return state;
}

std::string_view WifiEsp32::getSSID() const {
    pImpl_->lock();
    std::string_view ssid = pImpl_->connectedSSID;
    pImpl_->unlock();
    return ssid;
}

void WifiEsp32::setConnectionCallback(ConnectionCallback callback) {
    pImpl_->lock();
    pImpl_->connectionCallback = callback;
    pImpl_->unlock();
}

bool WifiEsp32::isConnected() const {
    return getState() == State::CONNECTED;
}

} // namespace board