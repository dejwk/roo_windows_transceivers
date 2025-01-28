#pragma once

#include <Arduino.h>

#include "roo_scheduler.h"
#include "roo_windows/composites/menu/title.h"
#include "roo_windows/containers/holder.h"
#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/containers/list_layout.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/activity.h"
#include "roo_windows/indicators/wifi.h"
#include "roo_windows/widgets/blank.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/widgets/progress_bar.h"
#include "roo_windows/widgets/switch.h"
// #include "roo_windows/widgets/text_block.h"
#include "roo_windows/widgets/text_label.h"
#include "roo_windows_onewire/activity/model.h"

namespace roo_windows_onewire {

typedef std::function<void(roo_windows::Task& task, int id)> ItemSelectedFn;

class ListItem : public roo_windows::HorizontalLayout {
 public:
  ListItem(const roo_windows::Environment& env, ItemSelectedFn on_click,
           const DeviceStateUi* device_state_ui);

  ListItem(const ListItem& other);

  bool isClickable() const override { return true; }

  void set(int idx, const Model& model);

  roo_windows::PreferredSize getPreferredSize() const override {
    return roo_windows::PreferredSize(
        roo_windows::PreferredSize::MatchParentWidth(),
        roo_windows::PreferredSize::WrapContentHeight());
  }

  void onClicked() override { on_click_(*getTask(), idx_); }

 private:
  // bool isOpen() const { return is_open_; }

  const roo_windows::Environment* env_;
  roo_windows::Icon thermometer_icon_;
  int idx_;
  roo_windows::TextLabel id_;
  std::unique_ptr<Widget> reading_;
  // roo_windows::Icon lock_icon_;
  ItemSelectedFn on_click_;
  const DeviceStateUi* device_state_ui_;
};

class ListModel : public roo_windows::ListModel<ListItem> {
 public:
  ListModel(Model& model);

  int elementCount() const override;
  void set(int idx, ListItem& dest) const override;

 private:
  Model& model_;
};

// The list of WiFi networks.
class List : public roo_windows::ListLayout<ListItem> {
 public:
  using roo_windows::ListLayout<ListItem>::ListLayout;
};

// All of the widgets of the list activity.
class ListActivityContents : public roo_windows::VerticalLayout,
                             public Model::EventListener {
 public:
  ListActivityContents(const roo_windows::Environment& env, Model& model,
                       ItemSelectedFn thermometer_selected_fn);

  roo_windows::PreferredSize getPreferredSize() const override {
    return roo_windows::PreferredSize(
        roo_windows::PreferredSize::MatchParentWidth(),
        roo_windows::PreferredSize::WrapContentHeight());
  }

  void update() { list_.modelChanged(); }

 private:
  void measurementsChanged() override;

  Model& model_;
  roo_windows::menu::Title title_;
  ListModel list_model_;
  List list_;
};

class ListActivity : public roo_windows::Activity {
 public:
  ListActivity(const roo_windows::Environment& env,
               roo_scheduler::Scheduler& scheduler, Model& model,
               ItemSelectedFn network_selected_fn);

  roo_windows::Widget& getContents() override { return scrollable_container_; }

  void onStart() override;
  void onStop() override;

  void onResume() override { contents_.update(); }

 private:
  Model& model_;

  ListActivityContents contents_;
  roo_windows::ScrollablePanel scrollable_container_;

  // Used to periodically refresh (discover, convert) thermometers. We keep it
  // here, taking advantage of the fact that the list activity is always at the
  // bottom of all thermometer config activities, so we can use Start/Stop to
  // control the polling.
  roo_scheduler::PeriodicTask refresh_task_;
};

}  // namespace roo_windows_onewire
