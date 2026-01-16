
#pragma once

#include "esphome/components/number/number.h"
#include "esphome/core/helpers.h"

#include "autoterm.h"  // to access parent's API

namespace esphome {
namespace autoterm {
class AUTOTerm;

namespace numbers {

class PowerLevelNumber : public number::Number, public Parented<AUTOTerm> {
 public:
  // Called when the user/API sets a new value
  void control(float value) override;

  // Helper to let parent publish updates (e.g., readback from hardware)
  void report(float value) { this->publish_state(value); }

 protected:
  static const char *const TAG;
};

class TemperatureSetpointNumber : public number::Number, public Parented<AUTOTerm> {
 public:
  // Called when the user/API sets a new value
  void control(float value) override;

  // Helper to let parent publish updates (e.g., readback from hardware)
  void report(float value) { this->publish_state(value); }

 protected:
  static const char *const TAG;
};

}  // namespace numbers
}  // namespace autoterm
}  // namespace esphome
