
#pragma once

#include "esphome/components/select/select.h"
#include "esphome/core/helpers.h"

#include "autoterm.h" // for Parented<AUTOTerm>

namespace esphome {
namespace autoterm {
class AUTOTerm;


class ModeSelect : public select::Select, public Parented<AUTOTerm> {
 public:
 
  // Optionally allow your device driver to be attached if needed
  // void set_parent(AutotermDevice *parent) { this->parent_ = parent; }

//   void setup() override {
//     // If you have a stored/initial state, publish it here:
//     if (this->restore_state_) {
//       auto restored = this->restore_state_();
//       if (restored.has_value()) {
//         this->publish_state(restored.value());
//       } else {
//         this->publish_state("Off");
//       }
//     } else {
//       this->publish_state("Off");
//     }
//   }

 protected:

  void control(const std::string &value) override;
  static const char *const TAG;
};

}  // namespace autoterm
}  // namespace esphome
