#pragma once

#include <string_view>

namespace board {

struct info {
    std::string_view name;
    std::string_view hwVersion;
    std::string_view manufacturer;
    std::string_view serialNumber;
};

const info &getInfo();
constexpr std::string_view getName();
constexpr std::string_view getHwVersion();
constexpr std::string_view getManufacturer();
constexpr std::string_view getSerialNumber();

} // namespace board