#include "nettime.hpp"
#include "esp_netif_sntp.h"
#include "esp_sntp.h"
#include "esp_log.h"
#include "assert.h"

static const char *TAG = "nettime";

const std::string NetTime::DefaultNtpServer = "pool.ntp.org";
bool NetTime::isInited_ = false;
bool NetTime::isSynced_ = false;
std::string NetTime::ntpServer_ = NetTime::DefaultNtpServer;
std::string NetTime::timezone_{"UTC0"};
NetTime::SyncCallback NetTime::syncCallback_ = nullptr;


esp_err_t NetTime::init(const std::string& ntpServer) {
    assert(!isInited_);

    ntpServer_ = ntpServer.empty() ? DefaultNtpServer : ntpServer;

    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG(ntpServer_.c_str());
    config.sync_cb = sntpCallback;
    config.start = true;
    config.server_from_dhcp = false;
    config.renew_servers_after_new_IP = true;
    config.ip_event_to_renew = IP_EVENT_STA_GOT_IP;

    esp_err_t ret = esp_netif_sntp_init(&config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "init: failed: %s", esp_err_to_name(ret));
        return ret;
    }    
    setenv("TZ", timezone_.c_str(), 1);
    tzset();

    if (!config.start) {
        esp_netif_sntp_start();
    }

    isInited_ = true;
    ESP_LOGI(TAG, "init: initialized with NTP server: %s", ntpServer_.c_str());
    return ESP_OK;
}
    
esp_err_t NetTime::sync(NetTime::SyncCallback cb) {
    assert(isInited_);

    syncCallback_ = cb;
    isSynced_ = false;

    int retry = 0;
    const int retryMaxCount = 15;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retryMaxCount) {
        ESP_LOGI(TAG, "Waiting for time sync... (%d/%d)", retry, retryMaxCount);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    if (retry >= retryMaxCount) {
        ESP_LOGE(TAG, "Time sync timeout");
        return ESP_ERR_TIMEOUT;
    }

    return ESP_OK;
}

bool NetTime::isInited(void) {
    return isInited_;
}

bool NetTime::isSynced(void) {
    return isSynced_;
}

void NetTime::sntpCallback(struct timeval *tv) {
    isSynced_ = true;
    ESP_LOGI(TAG, "sntpCallback: time synchronized");
    
    if (syncCallback_) {
        syncCallback_(true);
        syncCallback_ = nullptr;
    }
}

time_t NetTime::getUnixTime(void) {
    assert(isInited_);

    time_t now;
    time(&now);
    return now;
}

tm NetTime::getLocalTime(void) {
    assert(isInited_);

    const time_t now = getUnixTime();
    tm timeinfo;
    
    localtime_r(&now, &timeinfo);
    return timeinfo;
}

std::string NetTime::getLocalTimeString(const char* format) {
    const time_t now = getUnixTime();
    char buf[64];
    strftime(buf, sizeof(buf), format, localtime(&now));
    return std::string(buf);
}


void NetTime::setTimezone(const std::string& tz) {

}

std::string NetTime::getTimezone(void) {
    return {};
}

std::string NetTime::getNtpServer(void) {
    return {};
}

void NetTime::setNtpServer(const std::string& server) {

}