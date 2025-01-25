#pragma once

#include <string>

#include "roo_windows/composites/radio/radio_list.h"
#include "roo_windows/containers/holder.h"
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

  void set(std::string name, roo_control::UniversalDeviceId id);

 private:
  roo_windows::TextLabel name_;
  roo_windows::Holder reading_;
  const DeviceStateUi* device_state_ui_;
};

class UnassignedThermometerRadioGroupModel
    : public roo_windows::ListModel<UnassignedThermometerRadioGroupItem> {
 public:
  UnassignedThermometerRadioGroupModel(Model& model) : model_(model) {}

  int elementCount() const override;
  void set(int idx, UnassignedThermometerRadioGroupItem& dest) const override;

 private:
  Model& model_;
};

class UnassignedThermometerSelectionDialog
    : public roo_windows::RadioListDialog<UnassignedThermometerRadioGroupModel>,
      public roo_control::SensorEventListener {
 public:
  UnassignedThermometerSelectionDialog(const roo_windows::Environment& env,
                                       Model& model);

  void onEnter() override;
  void onExit(int result) override;

  void onChange() override;

  void sensorsChanged() override;
  void newReadingsAvailable() override;

 private:
  Model& model_;
  UnassignedThermometerRadioGroupModel list_model_;
  roo_control::UniversalDeviceId selected_device_id_;
};

}  // namespace roo_windows_onewire