#include "autoterm_number.h"
#include "esphome/core/log.h"

namespace esphome {
namespace autoterm {

const char *const PowerLevelNumber::TAG = "autoterm.power_level";
const char *const TemperatureSetpointNumber::TAG = "autoterm.temperature_setpoint";

void PowerLevelNumber::control(float value) {

  value = roundf(value); // integerize

  if (auto *p = this->get_parent()) {
    if (p->apply_power_level(static_cast<uint8_t>(value))) {
      this->publish_state(value);
    } else {
      this->publish_state(this->state);
    }
  }
}

void TemperatureSetpointNumber::control(float value) {

  value = roundf(value);  // integerize

  if (auto *p = this->get_parent()) {
    if (p->apply_temperature_setpoint(static_cast<uint8_t>(value))) {
      this->publish_state(value);
    } else {
      this->publish_state(this->state);
    }
  }
}

}  // namespace autoterm
}  // namespace esphome
