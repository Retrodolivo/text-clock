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

static SemaphoreHandle_t mutex;
static uint32_t mutexTimeoutMs = 1000;

#define MUTEX_LOCK(m) \
    do { \
        if (xSemaphoreTake(m, pdMS_TO_TICKS(mutexTimeoutMs)) != pdTRUE) { \
            ESP_LOGI(TAG, "mutex timeout"); \
            ESP_ERROR_CHECK(ESP_ERR_TIMEOUT); \
        } \
    } while (0)

#define MUTEX_UNLOCK(m) xSemaphoreGive(m)

esp_err_t NetTime::init(const std::string& tz, const std::string& ntpServer, NetTime::SyncCallback syncCb) {
    assert(!isInited_);

    mutex = xSemaphoreCreateRecursiveMutex();
    assert(mutex);

    ntpServer_ = ntpServer;
    /* Define user after time sync callback*/
    syncCallback_ = syncCb;
    timezone_ = tz;

    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG(ntpServer_.c_str());
    config.sync_cb = sntpCallback; //< would call usert time sync callback
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

    isInited_ = true;
    ESP_LOGI(TAG, "init: initialized with NTP server: %s", ntpServer_.c_str());
    return ESP_OK;
}

esp_err_t NetTime::sync(void) {
    assert(isInited_);
    assert(mutex);

    isSynced_ = false;

    MUTEX_LOCK(mutex);
    bool restartSuccess = esp_sntp_restart();
    MUTEX_UNLOCK(mutex);

    if (!restartSuccess) {
        ESP_LOGE(TAG, "sync: failed to restart sntp");
        MUTEX_UNLOCK(mutex);
        return ESP_FAIL;
    }

    int retry = 0;
    const int retryMaxCount = 15;
    while (retry < retryMaxCount) {
        MUTEX_LOCK(mutex);
        bool synced = (sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED);
        MUTEX_UNLOCK(mutex);
        
        if (synced) {
            isSynced_ = true;
            return ESP_OK;
        }
        
        ESP_LOGI(TAG, "sync: waiting for time sync... (%d/%d)", ++retry, retryMaxCount);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    ESP_LOGE(TAG, "sync: time sync timeout");
    return ESP_ERR_TIMEOUT;
}

bool NetTime::isInited(void) {
    return isInited_;
}

bool NetTime::isSynced(void) {
    assert(isInited_);

    return isSynced_;
}

void NetTime::sntpCallback(struct timeval *tv) {
    assert(isInited_);

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
    assert(isInited_);
    
    const time_t now = getUnixTime();
    char buf[64];
    strftime(buf, sizeof(buf), format, localtime(&now));

    return std::string(buf);
}

void NetTime::setTimezone(const std::string& tz) {
    assert(isInited_);
    assert(mutex);

    MUTEX_LOCK(mutex);

    timezone_ = tz;
    setenv("TZ", timezone_.c_str(), 1);
    tzset();
    
    MUTEX_UNLOCK(mutex);
}

std::string NetTime::getTimezone(void) {
    assert(isInited_);

    return timezone_;
}

std::string NetTime::getNtpServer(void) {
    assert(isInited_);

    return ntpServer_;
}

void NetTime::setNtpServer(const std::string& server) {

}