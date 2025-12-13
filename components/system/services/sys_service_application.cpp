#include "sys_service_application.hpp"
#include "esp_log.h"

const char *TAG = "sys_app";

constexpr int AppTaskPrio = 5;
constexpr TickType_t CommandTimeoutMs = pdMS_TO_TICKS(100);

namespace sys::service {

AppThread::AppThread(app::itf::IApp &app, uint32_t stackSize)
  : app_(app)
  , taskHandle_(nullptr)
  , stackSize_(stackSize) {
    static StaticSemaphore_t mutexMem;
    commandMutex_ = xSemaphoreCreateMutexStatic(&mutexMem);
}

bool AppThread::start() {
    if (taskHandle_) {
        return true; //< Already started
    }

    if (xTaskCreate(stateMachineTask, "stateMachineTask", stackSize_, this, AppTaskPrio,
                    static_cast<TaskHandle_t *>(taskHandle_)) != pdPASS) {
        ESP_LOGE("%s", "task creation failed", __func__);
        return false;
    }

    return true;
}

bool AppThread::commandInit() {
    return setCommand(Command::INIT);
}

bool AppThread::commandWork() {
    return setCommand(Command::WORK);
}

bool AppThread::commandClose() {
    return setCommand(Command::CLOSE);
}


AppThread::Command AppThread::popCommand() {
    Command current = Command::NONE;
    if (xSemaphoreTake(commandMutex_, portMAX_DELAY) == pdTRUE) {
        current = command_;
        command_ = Command::NONE;
        xSemaphoreGive(commandMutex_);
        return current;
    }

    return current;
}

bool AppThread::setCommand(Command cmd) {
    if (xSemaphoreTake(commandMutex_, pdMS_TO_TICKS(CommandTimeoutMs)) == pdTRUE) {
        command_ = cmd;
        xSemaphoreGive(commandMutex_);
        return true;
    }

    return false;
}

void AppThread::stateMachineTask(void *arg) {
    AppThread *instance = static_cast<AppThread *>(arg);
    if (instance == nullptr) {
        vTaskDelete(NULL); //< Delete task if no app instance
    }

    app::itf::IApp &app = instance->app_;
    State &state = instance->state_;

    ESP_LOGI(TAG, "%s: State machine task started", __func__);

    while (1) {
        const Command command = instance->popCommand();

        switch (state) {
            case State::CLOSE:
                switch (command) {
                    case Command::INIT:
                        if (app.init()) {
                            state = State::INIT;
                        }
                        break;

                    default:
                        break;
                }
                break;

            case State::INIT:
                switch (command) {
                    case Command::CLOSE:
                        if (app.close()) {
                            state = State::CLOSE;
                        }
                        break;

                    case Command::WORK:
                        state = State::WORK;
                        break;

                    default:
                        break;
                }
                break;

            case State::WORK:
                app.service();

                switch (command) {
                    case Command::CLOSE:
                        if (app.close()) {
                            state = State::CLOSE;
                        }
                        break;

                    default:
                        break;
                }
                break;

            default:
                break;
        };

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

} // namespace sys::service