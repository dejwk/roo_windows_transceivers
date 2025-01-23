#pragma once

#include "roo_control/sensors/sensor.h"
#include "roo_icons/filled/action.h"
#include "roo_icons/filled/content.h"
#include "roo_icons/filled/notification.h"
#include "roo_windows/composites/menu/title.h"
#include "roo_windows/config.h"
#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/containers/stacked_layout.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/activity.h"
#include "roo_windows/core/task.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/widgets/icon_with_caption.h"
#include "roo_windows/widgets/text_field.h"
#include "roo_windows_onewire/activity/model.h"
#include "roo_windows_onewire/activity/resources.h"

namespace roo_windows_onewire {

typedef std::function<void(roo_windows::Task& task, int id)> SelectFn;

class ThermometerDetailsActivityContents
    : public roo_windows::VerticalLayout,
      public roo_control::SensorEventListener {
 public:
  ThermometerDetailsActivityContents(const roo_windows::Environment& env,
                                     Model& model,
                                     std::function<void()> assign_fn,
                                     std::function<void()> unassign_fn)
      : roo_windows::VerticalLayout(env),
        model_(model),
        title_(env, kStrThermometerDetails),
        name_(env, "", roo_windows::font_subtitle1(),
              roo_display::kCenter | roo_display::kMiddle),
        rom_code_(env, "", roo_windows::font_caption(),
                  roo_display::kCenter | roo_display::kMiddle),
        reading_(env, "", roo_windows::font_body1(),
                 roo_display::kCenter | roo_display::kMiddle),
        d1_(env),
        actions_(env),
        button_unassign_(env, SCALED_ROO_ICON(filled, content_link_off),
                         kStrUnassign),
        button_assign_(env, SCALED_ROO_ICON(filled, content_link), kStrAssign) {
    setGravity(roo_windows::Gravity(roo_windows::kHorizontalGravityCenter,
                                    roo_windows::kVerticalGravityMiddle));
    // edit_.setOnInteractiveChange(edit_fn);
    button_assign_.setOnInteractiveChange(assign_fn);
    button_unassign_.setOnInteractiveChange(unassign_fn);
    // title_.add(edit_, roo_windows::HorizontalLayout::Params());
    add(title_, VerticalLayout::Params().setGravity(
                    roo_windows::kHorizontalGravityLeft));
    // indicator_.setPadding(roo_windows::PADDING_TINY);
    // add(indicator_, VerticalLayout::Params());
    name_.setPadding(roo_windows::PADDING_NONE);
    name_.setMargins(roo_windows::MARGIN_NONE);
    // rom_code_.setPadding(roo_windows::PADDING_NONE);
    // rom_code_.setMargins(roo_windows::MARGIN_NONE);
    add(name_, VerticalLayout::Params());
    add(rom_code_, VerticalLayout::Params());
    add(reading_);
    add(d1_, VerticalLayout::Params().setWeight(1));
    // indicator_.setConnectionStatus(roo_windows::WifiIndicator::DISCONNECTED);
    actions_.setUseLargestChild(true);
    button_unassign_.setPadding(roo_windows::PADDING_LARGE,
                                roo_windows::PADDING_SMALL);
    button_assign_.setPadding(roo_windows::PADDING_LARGE,
                              roo_windows::PADDING_SMALL);
    roo_display::Color pri = env.theme().color.primary;
    button_unassign_.setColor(pri);
    button_assign_.setColor(pri);
    actions_.add(button_unassign_,
                 roo_windows::HorizontalLayout::Params().setWeight(1));
    actions_.add(button_assign_,
                 roo_windows::HorizontalLayout::Params().setWeight(1));

    add(actions_, VerticalLayout::Params());
  }

  roo_windows::PreferredSize getPreferredSize() const override {
    return roo_windows::PreferredSize(
        roo_windows::PreferredSize::MatchParentWidth(),
        roo_windows::PreferredSize::WrapContentHeight());
  }

  void enter(int idx) {
    idx_ = idx;
    name_.setText(model_.binding_label(idx_));
    if (!model_.isBound(idx_)) {
      rom_code_.setText(kStrNotAssigned);
    } else {
      rom_code_.setText(
          model_.sensors().sensorUserFriendlyName(model_.getBinding(idx_)));
    }
  }

  void onDetailsChanged(bool has_rom_code) {
    button_unassign_.setEnabled(has_rom_code);
    button_assign_.setEnabled(!has_rom_code);
  }

  void updateReading() {
    if (!model_.isBound(idx_)) {
      reading_.setText("");
    } else {
      roo_control::Measurement m = model_.sensors().read(model_.getBinding(idx_));
      CHECK_EQ(roo_control_Quantity_kTemperature, m.quantity());
      if (m.isUnknown()) {
        reading_.setText("? °C");
      } else {
        reading_.setTextf("%3.1f°C", m.value());
      }
    }
  }

 private:
  void newReadingsAvailable() override { updateReading(); }

  Model& model_;
  int idx_;
  roo_windows::menu::Title title_;
  roo_windows::TextLabel name_;
  roo_windows::TextLabel rom_code_;
  roo_windows::TextLabel reading_;
  roo_windows::HorizontalDivider d1_;
  roo_windows::HorizontalLayout actions_;
  roo_windows::IconWithCaption button_unassign_;
  roo_windows::IconWithCaption button_assign_;
};

class ThermometerDetailsActivity : public roo_windows::Activity {
 public:
  ThermometerDetailsActivity(const roo_windows::Environment& env, Model& model,
                             SelectFn assign_fn, SelectFn unassign_fn)
      : roo_windows::Activity(),
        idx_(),
        model_(model),
        contents_(
            env, model,
            [this, assign_fn]() { assign_fn(*getContents().getTask(), idx_); },
            [this, unassign_fn]() {
              unassign_fn(*getContents().getTask(), idx_);
            }),
        scrollable_container_(env, contents_) {}

  roo_windows::Widget& getContents() override { return scrollable_container_; }

  void enter(roo_windows::Task& task, int idx) {
    idx_ = idx;
    task.enterActivity(this);
  }

  void onResume() override {
    contents_.enter(idx_);
    contents_.onDetailsChanged(model_.isBound(idx_));
    contents_.updateReading();
  }

  void onStart() override { model_.addEventListener(&contents_); }

  void onStop() override {
    model_.removeEventListener(&contents_);
    idx_ = -1;
  }

 private:
  int idx_;
  Model& model_;
  ThermometerDetailsActivityContents contents_;
  roo_windows::ScrollablePanel scrollable_container_;
};

}  // namespace roo_windows_onewire
