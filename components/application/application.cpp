#include "itf_app.hpp"

namespace app {

using namespace sys::itf;

TextClockApp::TextClockApp(IClock &clock, IDisplay &display, IWifi &wifi)
  : clock_(clock)
  , display_(display)
  , wifi_(wifi) {
}

/**
 * - Display already good to go.
 * - As network clock is used Wifi should be in connected state
 *   before clock sync
 */
bool TextClockApp::init() {
    wifi_.setConnectionCallback([this](IWifi::State state, const std::string &ssid) {
        if (state == IWifi::State::ERROR) {
            display_.drawPixel({0, 0}, color::CRGB::Red);
        } else if (state == IWifi::State::DISCONNECTED) {
            display_.drawPixel({0, 0}, color::CRGB::White);
        } else if (state == IWifi::State::CONNECTING) {
            display_.drawPixel({0, 0}, {255, 255, 0}); //< yellow
        } else if (state == IWifi::State::CONNECTED) {
            display_.drawPixel({0, 0}, color::CRGB::Green);
        }
    });

    if (wifi_.connect() == false) {
        return false;
    }

    if (clock_.init() == false) {
        return false;
    }

    /* Do clock sync as network connected */
    clock_.sync();

    return true;
}

bool TextClockApp::close() {
    return true;
}

bool TextClockApp::service() {
    return true;
}

} // namespace app
