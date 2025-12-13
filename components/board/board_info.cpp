#include "board_info.hpp"

namespace board {

using namespace std::literals;

constexpr info Info {
    .name = "TextClockBoard",
    .hwVersion = "0.1",
    .manufacturer = "Retroboyy Inc.",
    .serialNumber = "0001",
};


const info &getInfo() {
    return Info;
}

constexpr std::string_view getName() {
    return Info.name;
}

constexpr std::string_view getHwVersion() {
    return Info.hwVersion;
}

constexpr std::string_view getManufacturer() {
    return Info.manufacturer;
}

constexpr std::string_view getSerialNumber() {
    return Info.serialNumber;
}

} //namespace board