#pragma once

#include "sys_itf_display.hpp"

namespace app {

class Render {
public:
    Render() = default;

    bool startRenderTo(sys::itf::IDisplay *display);

private:
    bool start();
    static void serviceTask(void *arg);

    sys::itf::IDisplay *display_ = nullptr;
    void *taskHandle_ = nullptr;
};

} // namespace app