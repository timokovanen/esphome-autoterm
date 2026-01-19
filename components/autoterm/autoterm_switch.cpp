#include "autoterm_switch.h"
#include "esphome/core/log.h"

namespace esphome {
namespace autoterm {

const char *const VentilationSwitch::TAG = "autoterm.ventilation";

void VentilationSwitch::write_state(bool state) {
  if (auto *p = this->get_parent()) {
    p->apply_ventilation(state);
  }

  this->publish_state(state);
}

void PowerSwitch::write_state(bool state) {
  if (auto *p = this->get_parent()) {
    p->apply_power(state);
  }

  this->publish_state(state);
}

}  // namespace autoterm
}  // namespace esphome
