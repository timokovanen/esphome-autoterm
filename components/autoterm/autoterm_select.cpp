#include "autoterm_select.h"
#include "esphome/core/log.h"

namespace esphome {
namespace autoterm {

const char *const ModeSelect::TAG = "autoterm.mode";

  // Handle selection from Home Assistant / API
  void ModeSelect::control(const std::string &value){
    // Validate and apply
    if (value == "By Heater" || value == "By Panel" || value == "By External" || value == "By Power") {
      // TODO: call into your device logic if needed, e.g.:
      // if (this->parent_) this->parent_->request_state(value);
      this->publish_state(value);
    } else {
      ESP_LOGW("autoterm.select", "Unsupported select value: %s", value.c_str());
      // Optionally reject or coerce
    }
  }

  // AutotermDevice *parent_{nullptr};


}  // namespace autoterm
}  // namespace esphome
