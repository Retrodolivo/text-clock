#pragma once

#include "time.h"
#include "esp_err.h"
#include <string_view>
#include <functional>

namespace sys::itf {

class IClock {
public:
    virtual ~IClock() = default;

    virtual bool init() = 0;

    /* Time management */
    virtual bool setTime(uint8_t hours, uint8_t minutes, uint8_t seconds) = 0;
    virtual bool setDate(uint8_t day, uint8_t month, uint16_t year) = 0;
    virtual bool getTime(uint8_t &hours, uint8_t &minutes, uint8_t &seconds) = 0;
    virtual bool getDate(uint8_t &day, uint8_t &month, uint16_t &year) = 0;

    virtual bool getUnixTime(time_t &timestamp) = 0;
    virtual bool setUnixTime(time_t timestamp) = 0;

    virtual std::string_view getName() const = 0;

    /* For network time */
    virtual esp_err_t sync() = 0;
    virtual bool isSync() const = 0;
    /* Callback for sync completion */
    using SyncCallback = std::function<void(bool success)>;
    virtual void setSyncCallback(SyncCallback callback) = 0;
};

} // namespace sys::itf