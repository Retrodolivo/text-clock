#pragma once

#include <string>

#include "itf_display.hpp"

// @file Clock board core interface

typedef struct {
    uint8_t hour; 
    uint8_t min;
    uint8_t sec;
} time_st;

// Board identification
std::string Board_getName(void);
std::string Board_getVersion(void);
std::string Board_getManufacturer(void);
std::string Board_getSerialNumber(void);

// Time basic operation
esp_err_t Board_setTimeUTC(const time_st& time);
time_st Board_getTimeUTC(void);
esp_err_t Board_showTime(void);
esp_err_t Board_hideTime(void);

ILedMatrixDisplay *Board_getDisplay(void);

#ifdef CONFIG_NETWORK_USE
// Time Synchronization
esp_err_t Board_syncToNetworkTime(void);
#endif