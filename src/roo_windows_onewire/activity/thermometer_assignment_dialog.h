#pragma once

#include <string>
#include <memory>

#include "roo_windows/composites/radio/radio_list.h"
#include "roo_windows/containers/list_layout.h"
#include "roo_windows/dialogs/dialog.h"
#include "roo_windows/dialogs/radio_list_dialog.h"
#include "roo_windows_onewire/activity/model.h"

namespace roo_windows_onewire {

class UnassignedThermometerRadioGroupItem
    : public roo_windows::HorizontalLayout {
 public:
  UnassignedThermometerRadioGroupItem(
      const roo_windows::Environment& env,
      const DeviceStateUi* device_state_ui);

  UnassignedThermometerRadioGroupItem(
      const UnassignedThermometerRadioGroupItem& other);

  void set(std::string id);

 private:
  roo_windows::TextLabel id_;
  std::unique_ptr<Widget> reading_;
  const DeviceStateUi* device_state_ui_;
};

class UnassignedThermometerRadioGroupModel
    : public roo_windows::ListModel<UnassignedThermometerRadioGroupItem> {
 public:
  UnassignedThermometerRadioGroupModel(ThermometerSelectorModel& model) : model_(model) {}

  int elementCount() const override;
  void set(int idx, UnassignedThermometerRadioGroupItem& dest) const override;

 private:
  ThermometerSelectorModel& model_;
};

class UnassignedThermometerSelectionDialog
    : public roo_windows::RadioListDialog<UnassignedThermometerRadioGroupModel>,
      public Model::EventListener {
 public:
  UnassignedThermometerSelectionDialog(const roo_windows::Environment& env,
                                       ThermometerSelectorModel& model);

  void onEnter() override;
  void onExit(int result) override;

  void onChange() override;

  void itemsChanged() override;
  void measurementsChanged() override;

 private:
  ThermometerSelectorModel& model_;
  UnassignedThermometerRadioGroupModel list_model_;
  std::string selected_device_id_;
};

}  // namespace roo_windows_onewire