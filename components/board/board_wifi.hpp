#pragma once

#include "sys_itf_wifi.hpp"
#include <memory>

namespace board {

class WifiEsp32: public sys::itf::IWifi {
public:
    WifiEsp32();
    ~WifiEsp32() override;

    bool connect(const std::string &ssid, const std::string &password) override;
    bool disconnect() override;
    bool reconnect() override;

    bool scan(ScanStartCallback callback = nullptr) override {
        return false; //< Not implemented yet
    }

    void setAutoReconnect(bool enable) override;

    State getState() const override;
    std::string_view getSSID() const override;

    void setConnectionCallback(ConnectionCallback callback) override;

    bool isConnected() const;

    class Impl;
    /* Ugly getter for internal use (allow implementation connects to esp32 internals)*/
    Impl *getImplForEventHandler() const {
        return pImpl_.get();
    }

private:
    std::unique_ptr<Impl> pImpl_; //< use of pimpl idiom 
};

} // namespace board