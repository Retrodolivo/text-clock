#pragma once

#include "sys_itf_display.hpp"

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

class TextClockApp : public app::itf::IApp {
public:
    TextClockApp(sys::itf::IDisplay &display);
    bool init() override;
    bool close() override;
    bool service() override;

private:
    sys::itf::IDisplay &display_;
};

} // namespace app
