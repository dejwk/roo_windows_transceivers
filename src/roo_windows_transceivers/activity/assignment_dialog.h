#pragma once

#include <memory>
#include <string>

#include "roo_windows.h"
#include "roo_windows/composites/radio/radio_list.h"
#include "roo_windows/containers/list_layout.h"
#include "roo_windows/dialogs/dialog.h"
#include "roo_windows/dialogs/radio_list_dialog.h"
#include "roo_windows_transceivers/model.h"

namespace roo_windows_transceivers {

class UnassignedRadioGroupItem : public roo_windows::HorizontalLayout {
 public:
  UnassignedRadioGroupItem(const roo_windows::Environment& env, const Ui* ui);

  UnassignedRadioGroupItem(const UnassignedRadioGroupItem& other);

  void set(std::string id);

 private:
  roo_windows::TextLabel id_;
  std::unique_ptr<Widget> reading_;
  const Ui* ui_;
};

class UnassignedRadioGroupModel : public roo_windows::RadioListModel {
 public:
  UnassignedRadioGroupModel(Model& model) : model_(model) {}

  int elementCount() const override;
  void set(int idx, roo_windows::Widget& dest) const override;

 private:
  Model& model_;
};

class UnassignedItemSelectionDialog : public roo_windows::RadioListDialog,
                                      public Model::EventListener {
 public:
  UnassignedItemSelectionDialog(const roo_windows::Environment& env,
                                Model& model);

  void onEnter() override;
  void onExit(int result) override;

  void onChange() override;

  void itemsChanged() override;
  void measurementsChanged() override;

 private:
  Model& model_;
  UnassignedRadioGroupModel list_model_;
  std::string selected_device_id_;
};

}  // namespace roo_windows_transceivers