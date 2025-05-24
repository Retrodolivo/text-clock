#pragma once

#include <string>

#include "itf_display.hpp"

// Board identification
std::string Board_getName(void);
std::string Board_getVersion(void);
std::string Board_getManufacturer(void);
std::string Board_getSerialNumber(void);

ILedMatrixDisplay *Board_getDisplay(void);

#ifdef CONFIG_NETWORK_USE
// Time Synchronization
esp_err_t Board_syncToNetworkTime(void);
#endif