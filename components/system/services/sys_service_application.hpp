#pragma once

#include "itf_app.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstdint>

namespace sys::service {

class AppThread {
public:
    explicit AppThread(app::itf::IApp &app, uint32_t stackSize);

    bool commandInit();
    bool commandClose();

    enum class State {
        CLOSE,
        WORK,
        BUSY,
    };

    State getState() const;
private:
    enum class Command {
        CLOSE,
        INIT,
        NONE,
    };

    bool start();

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
