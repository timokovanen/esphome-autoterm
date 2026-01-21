#include "autoterm_select.h"
#include "esphome/core/log.h"

namespace esphome {
namespace autoterm {

const char *const ModeSelect::TAG = "autoterm.mode";

void ModeSelect::control(const std::string &value) {

  if (auto *p = this->get_parent()) {
    if (p->apply_operating_mode(value)) {
      this->publish_state(value);
    } else {
      const char *previous = this->current_option();
      this->publish_state(previous ? previous : "");
    }
  }
}

}  // namespace autoterm
}  // namespace esphome
