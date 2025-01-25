#include "roo_windows_onewire/activity/thermometer_assignment_dialog.h"

#include "roo_display/ui/string_printer.h"
#include "roo_windows_onewire/activity/resources.h"

using roo_windows::Dialog;

// using roo_onewire::ThermometerRoles;
namespace roo_windows_onewire {

UnassignedThermometerRadioGroupItem::UnassignedThermometerRadioGroupItem(
    const roo_windows::Environment& env, const DeviceStateUi* device_state_ui)
    : HorizontalLayout(env),
      id_(env, "1-Wire:1234567812345678", roo_windows::font_subtitle1()),
      reading_(device_state_ui->creator_fn()),
      device_state_ui_(device_state_ui) {
  setGravity(roo_windows::Gravity(roo_windows::kHorizontalGravityNone,
                                  roo_windows::kVerticalGravityMiddle));
  id_.setMargins(roo_windows::MARGIN_NONE);
  id_.setPadding(roo_windows::PADDING_SMALL);
  add(id_, HorizontalLayout::Params().setWeight(1));
  add(*reading_);
}

UnassignedThermometerRadioGroupItem::UnassignedThermometerRadioGroupItem(
    const UnassignedThermometerRadioGroupItem& other)
    : HorizontalLayout(other),
      id_(other.id_),
      device_state_ui_(other.device_state_ui_) {
  reading_ = device_state_ui_->creator_fn();
  add(id_);
  add(*reading_);
}

void UnassignedThermometerRadioGroupItem::set(std::string id) {
  device_state_ui_->setter_fn(id, *reading_);
  id_.setText(std::move(id));
}

int UnassignedThermometerRadioGroupModel::elementCount() const {
  // return model_.unassigned().size();
  return model_.unassignedItemCount();
}

void UnassignedThermometerRadioGroupModel::set(
    int idx, UnassignedThermometerRadioGroupItem& dest) const {
  dest.set(std::string(model_.unassignedItemId(idx)));
}

UnassignedThermometerSelectionDialog::UnassignedThermometerSelectionDialog(
    const roo_windows::Environment& env, Model& model)
    : roo_windows::RadioListDialog<UnassignedThermometerRadioGroupModel>(
          env, UnassignedThermometerRadioGroupItem(env, model.state_ui())),
      model_(model),
      list_model_(model) {
  setTitle(kStrSelectThermometer);
  setModel(list_model_);
}

void UnassignedThermometerSelectionDialog::onEnter() {
  model_.addEventListener(this);
  model_.requestUpdate();
  reset();
}

void UnassignedThermometerSelectionDialog::onExit(int result) {
  model_.removeEventListener(this);
}

void UnassignedThermometerSelectionDialog::sensorsChanged() {
  int s = selected();
  int new_s = -1;
  if (s >= 0) {
    // Update the selection to stick to the originally selected rom code.
    for (size_t i = 0; i < model_.unassignedItemCount(); ++i) {
      if (model_.unassignedItemId(i) == selected_device_id_) {
        // Found!
        new_s = i;
        break;
      }
    }
  }
  contentsChanged();
  setSelected(new_s);
}

void UnassignedThermometerSelectionDialog::newReadingsAvailable() {
  contentsChanged();
}

void UnassignedThermometerSelectionDialog::onChange() {
  int s = selected();
  selected_device_id_ =
      (s >= 0) ? std::string(model_.unassignedItemId(s)) : std::string();
  RadioListDialog::onChange();
}

}  // namespace roo_windows_onewire
