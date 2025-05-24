#include "esp_err.h"

enum class WifiFailEvents {
    FAIL_TO_CONNECT,
    BEACON_TIMEOUT,
    FAIL_UNKNOWN,
};

typedef struct {
    uint8_t ssid[32];
    uint8_t password[64];
    void (*failCallBack)(WifiFailEvents event);
} itf_wifi_config_t;

esp_err_t Board_wifiInit(void);
esp_err_t Board_wifiDeinit(void);

esp_err_t Board_wifiConnect(const itf_wifi_config_t& config, uint32_t timeoutMs);
esp_err_t Board_wifiDisconnect(void);

bool Board_wifiIsInited(void);
bool Board_wifiIsConnected(void);