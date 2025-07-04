#pragma once

#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/widgets/text_label.h"

#include "roo_transceivers.pb.h"

namespace roo_windows_transceivers {

class DescriptorWidget : public roo_windows::VerticalLayout {
 public:
  DescriptorWidget(const roo_windows::Environment& env);

  void setDescriptor(const roo_transceivers_Descriptor& descriptor);

 private:
  roo_windows::StringViewLabel sensors_caption_;
  roo_windows::TextLabel sensors_[16];
  roo_windows::StringViewLabel actuators_caption_;
  roo_windows::TextLabel actuators_[16];
};

}  // namespace roo_windows_transceivers