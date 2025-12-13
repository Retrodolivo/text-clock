#include "board_clock.hpp"
#include "nettime.hpp"
#include "esp_err.h"


namespace board {

NetworkClock::NetworkClock(const std::string &timezone, const std::string &ntpServer)
  : timezone_(timezone)
  , ntpServer_(ntpServer) {
}

bool NetworkClock::init() {
   return NetTime::init(timezone_, ntpServer_) == ESP_OK; 
}

bool NetworkClock::getTime(uint8_t &hours, uint8_t &minutes, uint8_t &seconds) {
    if (NetTime::isInited()) {
        auto time = NetTime::getLocalTime();
        hours = time.tm_hour;
        minutes = time.tm_min;
        seconds = time.tm_sec;
        return true;
    }

    return false;
}

bool NetworkClock::getDate(uint8_t &day, uint8_t &month, uint16_t &year) {
    if (NetTime::isInited()) {
        auto time = NetTime::getLocalTime();
        day = time.tm_mday;
        month = time.tm_mon + 1; //< convert from zero index
        year = time.tm_year + 1900; 
        return true;
    }

    return false;    
}

bool NetworkClock::getUnixTime(time_t &timestamp) {
    if (NetTime::isInited()) {
        timestamp = NetTime::getUnixTime();
        return true;
    }

    return false;
}

esp_err_t NetworkClock::sync() {
    return NetTime::sync();
}

bool NetworkClock::isSync() const {
    return NetTime::isSynced();
}

void NetworkClock::setSyncCallback(SyncCallback callback) {
    syncCallback_ = callback;
}

} // namespace board