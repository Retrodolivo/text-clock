#include <string>
#include "itf_board.hpp"
#include "board_display.hpp"
#include "esp_err.h"

// Board identification
std::string Board_getName(void) {
    return {"TextClockBoard"};
}

std::string Board_getVersion(void) {
    return {"v1.0"};
}

std::string Board_getManufacturer(void) {
    return {"Retroboyy Inc."};
}

std::string Board_getSerialNumber(void) {
    return {"0001"};
}

ILedMatrixDisplay *Board_getDisplay(void) {
    static TextClockDisplay display;
    return &display;
}

// Time basic operation
esp_err_t Board_setTimeUTC(const time_st& time) {
    // Hardware specific time setting code
    

    return ESP_OK;
}

time_st Board_getTimeUTC(void) {
    return {0, 0, 0};
}

esp_err_t Board_showTime(void) {
    return ESP_OK;
}

esp_err_t Board_hideTime(void) {
    return ESP_OK;
}