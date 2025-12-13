#pragma once

#include "sys_itf_clock.hpp"
#include "time.h"
#include "esp_err.h"
#include <string>

namespace board {

class NetworkClock: public sys::itf::IClock {
public:
    NetworkClock(const std::string &timezone = "UTC0", const std::string &ntpServer = "pool.ntp.org");

    bool init() override;

    /* Time management */
    bool getTime(uint8_t &hours, uint8_t &minutes, uint8_t &seconds) override;
    bool getDate(uint8_t &day, uint8_t &month, uint16_t &year) override;
    bool getUnixTime(time_t &timestamp) override;

    /* Cannot be implemented. Empty to remove abstract property */
    bool setTime(uint8_t hours, uint8_t minutes, uint8_t seconds) override {
        return false;
    }
    bool setDate(uint8_t day, uint8_t month, uint16_t year) override {
        return false;
    }
    bool setUnixTime(time_t timestamp) override {
        return false;
    }

    void setSyncCallback(SyncCallback callback) override;
    bool isSync() const override;
    esp_err_t sync() override;

    std::string_view getName() const {
        return "Network NTP Clock";
    }

private:
    std::string timezone_;
    std::string ntpServer_;
    SyncCallback syncCallback_;
};

} // namespace board