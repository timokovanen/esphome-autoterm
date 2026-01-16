
#pragma once

#include "esphome/components/switch/switch.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace autoterm {
class AUTOTerm;

namespace switches {

class VentilationSwitch : public switch_::Switch, public Parented<AUTOTerm> {
 public:
  // VentilationSwitch() = default;
  void write_state(bool state) override;
 
 protected:
  static const char *const TAG;
};

} // namespace switches
} // namespace autoterm
} // namespace esphome
