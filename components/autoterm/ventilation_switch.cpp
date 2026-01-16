#include "ventilation_switch.h"
#include "esphome/core/log.h"

namespace esphome {
namespace autoterm {
namespace switches {

const char *const VentilationSwitch::TAG = "autoterm.ventilation_switch";

void VentilationSwitch::write_state(bool state) {
  this->publish_state(state);
}

}  // namespace switches
}  // namespace autoterm
}  // namespace esphome
