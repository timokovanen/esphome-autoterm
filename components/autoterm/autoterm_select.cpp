#include "autoterm_select.h"
#include "esphome/core/log.h"

namespace esphome {
namespace autoterm {

const char *const ModeSelect::TAG = "autoterm.mode";

  void ModeSelect::control(const std::string &value){
    // Validate and apply
    if (value == "By Heater" || value == "By Panel" || value == "By External" || value == "By Power") {
      if (auto *p = this->get_parent()) {
        p->apply_operating_mode(value);
      }
      this->publish_state(value);
    } else {
      ESP_LOGW("autoterm.select", "Unsupported select value: %s", value.c_str());
    }
  }

}  // namespace autoterm
}  // namespace esphome
