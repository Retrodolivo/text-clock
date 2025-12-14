#include "itf_app.hpp"

namespace app {

using namespace sys::itf;

TextClockApp::TextClockApp(IClock &clock, IDisplay &display, IWifi &wifi)
  : clock_(clock)
  , display_(display)
  , wifi_(wifi) {
}

bool TextClockApp::init() {
    return true;
}

bool TextClockApp::close() {
    return true;
}

bool TextClockApp::service() {
    return true;
}

} // namespace app
