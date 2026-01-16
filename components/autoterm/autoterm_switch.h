
#pragma once

#include "esphome/components/switch/switch.h"
#include "esphome/core/helpers.h"

#include "autoterm.h" // for Parented<AUTOTerm>

namespace esphome {
namespace autoterm {
class AUTOTerm;

class VentilationSwitch : public switch_::Switch, public Parented<AUTOTerm> {
 public:
  // VentilationSwitch() = default;
  void write_state(bool state) override;
 
 protected:
  static const char *const TAG;
};

class PowerSwitch : public switch_::Switch, public Parented<AUTOTerm> {
 public:
  // PowerSwitch() = default;
  void write_state(bool state) override;
 
 protected:
  static const char *const TAG;
};

} // namespace autoterm
} // namespace esphome
