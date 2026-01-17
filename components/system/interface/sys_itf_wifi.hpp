#pragma once

#include <string>
#include <cstdint>
#include <functional>

namespace sys::itf {

class IWifi {
public:
    enum class State {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        ERROR
    };

    enum class Security {
        OPEN,
        WEP,
        WPA_PSK,
        WPA2_PSK,
        WPA_WPA2_PSK,
        WPA3_PSK
    };

    struct NetworkInfo {
        std::string ssid;
        int32_t rssi;
        Security security;
        uint8_t channel;
    };

    using ScanStartCallback = std::function<void(void)>;
    using ConnectionCallback = std::function<void(State state, const std::string &ssid)>;

    virtual ~IWifi() = default;

    /* Connection management */
    virtual bool connect(const std::string &ssid = MY_WIFI_SSID, const std::string &password = MY_WIFI_PASSWD) = 0;
    virtual bool disconnect() = 0;
    virtual bool reconnect() = 0;

    /* Network scan */
    virtual bool scan(ScanStartCallback callback = nullptr) = 0;

    /* Configuration */
    virtual void setAutoReconnect(bool enable) = 0;

    /* Get info */
    virtual State getState() const = 0;
    virtual std::string_view getSSID() const = 0;

    virtual void setConnectionCallback(ConnectionCallback callback) = 0;
};


} // namespace sys::itf