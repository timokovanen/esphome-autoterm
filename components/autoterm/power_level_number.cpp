#include "power_level_number.h"
#include "esphome/core/log.h"

namespace esphome {
namespace autoterm {
namespace numbers {

const char *const PowerLevelNumber::TAG = "autoterm.power_level";

void PowerLevelNumber::control(float value) {
  // Ensure integer steps 0..9 (the traits already enforce, but clamp just in case)
  if (value < 0) value = 0;
  if (value > 9) value = 9;
  value = roundf(value);  // integerize

  ESP_LOGI(TAG, "Requested power_level -> %.0f", value);

  // Ask parent to apply it to hardware; parent can return the applied value
  if (auto *p = this->get_parent()) {
    p->apply_power_level(static_cast<uint8_t>(value));
  }

  // Publish final state
  this->publish_state(value);
}

}  // namespace numbers
}  // namespace autoterm
}  // namespace esphome
