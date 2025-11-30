#include "itf_app.hpp"

namespace app {

TextClockApp::TextClockApp(sys::itf::IDisplay &display)
  : display_(display) {
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
