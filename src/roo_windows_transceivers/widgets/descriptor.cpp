#include "roo_windows_transceivers/widgets/descriptor.h"

namespace roo_windows_transceivers {

DescriptorWidget::DescriptorWidget(const roo_windows::Environment& env)
    : roo_windows::VerticalLayout(env),
      sensors_caption_(env, "Sensors", roo_windows::font_caption()),
      sensors_{
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
      },
      actuators_caption_(env, "Actuators", roo_windows::font_caption()),
      actuators_{
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
          roo_windows::TextLabel(env, "", roo_windows::font_body1()),
      } {
  add(sensors_caption_);
  sensors_caption_.setVisibility(GONE);
  for (size_t i = 0; i < 16; ++i) {
    add(sensors_[i]);
    sensors_[i].setVisibility(GONE);
  }
  add(actuators_caption_);
  actuators_caption_.setVisibility(GONE);
  for (size_t i = 0; i < 16; ++i) {
    add(actuators_[i]);
    actuators_[i].setVisibility(GONE);
  }
}

void DescriptorWidget::setDescriptor(
    const roo_transceivers_Descriptor& descriptor) {
  sensors_caption_.setVisibility(descriptor.sensors_count > 0 ? VISIBLE : GONE);
  for (size_t i = 0; i < 16; ++i) {
    if (i < descriptor.sensors_count) {
      sensors_[i].setText(descriptor.sensors[i].id);
      sensors_[i].setVisibility(VISIBLE);
    } else {
      sensors_[i].setVisibility(GONE);
    }
  }
  for (size_t i = 0; i < 16; ++i) {
    if (i < descriptor.actuators_count) {
      actuators_[i].setText(descriptor.actuators[i].id);
      actuators_[i].setVisibility(VISIBLE);
    } else {
      actuators_[i].setVisibility(GONE);
    }
  }
}

}  // namespace roo_windows_transceivers