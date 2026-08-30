#include "roo_windows_transceivers/widgets/descriptor.h"

namespace roo_windows_transceivers {

using roo_windows::Visibility;

DescriptorWidget::DescriptorWidget(roo_windows::ApplicationContext& env)
    : roo_windows::FlexLayout(env, roo_windows::FlexDirection::kColumn),
      sensors_caption_(env, "Sensors",
                       roo_windows::material2::text_style_caption()),
      sensors_{
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
      },
      actuators_caption_(env, "Actuators",
                         roo_windows::material2::text_style_caption()),
      actuators_{
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
          roo_windows::TextLabel(env, "",
                                 roo_windows::material2::text_style_body1()),
      } {
  setAlignItems(roo_windows::AlignItems::kStretch);
  setGap(roo_windows::Scaled(4));
  add(sensors_caption_);
  sensors_caption_.setVisibility(Visibility::kGone);
  for (size_t i = 0; i < 16; ++i) {
    add(sensors_[i]);
    sensors_[i].setVisibility(Visibility::kGone);
  }
  add(actuators_caption_);
  actuators_caption_.setVisibility(Visibility::kGone);
  for (size_t i = 0; i < 16; ++i) {
    add(actuators_[i]);
    actuators_[i].setVisibility(Visibility::kGone);
  }
}

void DescriptorWidget::setDescriptor(
    const roo_transceivers_Descriptor& descriptor) {
  sensors_caption_.setVisibility(
      descriptor.sensors_count > 0 ? Visibility::kVisible : Visibility::kGone);
  for (size_t i = 0; i < 16; ++i) {
    if (i < descriptor.sensors_count) {
      sensors_[i].setText(descriptor.sensors[i].id);
      sensors_[i].setVisibility(Visibility::kVisible);
    } else {
      sensors_[i].setVisibility(Visibility::kGone);
    }
  }
  for (size_t i = 0; i < 16; ++i) {
    if (i < descriptor.actuators_count) {
      actuators_[i].setText(descriptor.actuators[i].id);
      actuators_[i].setVisibility(Visibility::kVisible);
    } else {
      actuators_[i].setVisibility(Visibility::kGone);
    }
  }
}

}  // namespace roo_windows_transceivers
