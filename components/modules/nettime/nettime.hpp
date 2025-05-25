#pragma once

#include "time.h"
#include "esp_err.h"
#include <functional>
#include <string>

class NetTime {
public:
    using SyncCallback = std::function<void(bool success)>;
    static const std::string DefaultNtpServer;

    static esp_err_t init(const std::string& tz = "UTC0", const std::string& ntpServer = DefaultNtpServer, NetTime::SyncCallback syncCb = nullptr);
    static bool isInited(void);
    
    static void setTimezone(const std::string& tz);
    static std::string getTimezone(void);
    
    static time_t getUnixTime(void); //< UTC time
    static tm getLocalTime(void);    //< Timezone offset
    static std::string getLocalTimeString(const char* format);

    static std::string getNtpServer(void);
    static void setNtpServer(const std::string& server);

    static esp_err_t sync(void);
    static bool isSynced(void);
private:
    static void sntpCallback(struct timeval *tv);
    static bool isInited_;
    static bool isSynced_;
    static std::string ntpServer_;
    static std::string timezone_;
    static SyncCallback syncCallback_;
};
