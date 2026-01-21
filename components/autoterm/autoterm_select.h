
#pragma once

#include "esphome/components/select/select.h"
#include "esphome/core/helpers.h"

#include "autoterm.h"

namespace esphome {
namespace autoterm {
class AUTOTerm;


class ModeSelect : public select::Select, public Parented<AUTOTerm> {
  protected:
  void control(const std::string &value) override;
  static const char *const TAG;
};

}  // namespace autoterm
}  // namespace esphome
