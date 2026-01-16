#include "switches.h"
#include "esphome/core/log.h"

namespace esphome {
namespace autoterm {
namespace switches {

const char *const VentilationSwitch::TAG = "autoterm.ventilation";

void VentilationSwitch::write_state(bool state) {
  this->publish_state(state);
}

}  // namespace switches
}  // namespace autoterm
}  // namespace esphome
