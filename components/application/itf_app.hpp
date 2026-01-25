#pragma once

#include "sys_itf_display.hpp"
#include "sys_itf_clock.hpp"
#include "sys_itf_wifi.hpp"
#include "render.hpp"


namespace app {

namespace itf {

class IApp {
public:
    virtual ~IApp() = default;

    virtual bool init() = 0;
    virtual bool close() = 0;
    virtual bool service() = 0;
};

} // namespace itf

class TextClockApp: public app::itf::IApp {
public:
    TextClockApp(sys::itf::IClock &clock, sys::itf::IDisplay &display, sys::itf::IWifi &wifi);
    bool init() override;
    bool close() override;
    bool service() override;

private:
    sys::itf::IClock &clock_;
    sys::itf::IDisplay &display_;
    sys::itf::IWifi &wifi_;
    Render render_;
};

} // namespace app
