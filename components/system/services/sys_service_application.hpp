#pragma once

#include "itf_app.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstdint>

namespace sys::service {

class AppThread {
public:
    explicit AppThread(app::itf::IApp &app, uint32_t stackSize);
    bool start();

    bool commandInit();
    bool commandWork();
    bool commandClose();

private:
    enum class State {
        INIT,
        WORK,
        CLOSE,
    };

    enum class Command {
        NONE,
        INIT,
        WORK,
        CLOSE,
    };

    static void stateMachineTask(void *arg);

    Command popCommand();
    bool setCommand(Command cmd);

    app::itf::IApp &app_;
    State state_ = State::CLOSE;
    Command command_ = Command::CLOSE;
    SemaphoreHandle_t commandMutex_ = nullptr;
    void *taskHandle_ = nullptr;
    uint32_t stackSize_ = 0;
};

} // namespace sys::service
