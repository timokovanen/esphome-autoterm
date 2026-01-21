
#pragma once

#include "esphome/components/number/number.h"
#include "esphome/core/helpers.h"

#include "autoterm.h"

namespace esphome {
namespace autoterm {
class AUTOTerm;

class PowerLevelNumber : public number::Number, public Parented<AUTOTerm> {
 public:
  void report(float value) { this->publish_state(value); }

 protected:
  void control(float value) override;
  static const char *const TAG;
};

class TemperatureSetpointNumber : public number::Number, public Parented<AUTOTerm> {
 public:
  void report(float value) { this->publish_state(value); }

 protected:
  void control(float value) override;
  static const char *const TAG;
};

}  // namespace autoterm
}  // namespace esphome
