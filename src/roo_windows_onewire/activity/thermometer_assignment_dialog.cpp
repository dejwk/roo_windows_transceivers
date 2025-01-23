#include "roo_windows_onewire/activity/thermometer_assignment_dialog.h"

#include "roo_display/ui/string_printer.h"
#include "roo_windows_onewire/activity/resources.h"

using roo_windows::Dialog;

// using roo_onewire::ThermometerRoles;

namespace roo_windows_onewire {

UnassignedThermometerRadioGroupItem::UnassignedThermometerRadioGroupItem(
    const roo_windows::Environment& env)
    : HorizontalLayout(env),
      rom_code_(env, "1-Wire:1234567812345678", roo_windows::font_subtitle1()),
      reading_(env, "00.0°C", roo_windows::font_subtitle1()) {
  setGravity(roo_windows::Gravity(roo_windows::kHorizontalGravityNone,
                                  roo_windows::kVerticalGravityMiddle));
  rom_code_.setMargins(roo_windows::MARGIN_NONE);
  rom_code_.setPadding(roo_windows::PADDING_SMALL);
  add(rom_code_, HorizontalLayout::Params().setWeight(1));
  add(reading_);
}

UnassignedThermometerRadioGroupItem::UnassignedThermometerRadioGroupItem(
    const UnassignedThermometerRadioGroupItem& other)
    : HorizontalLayout(other),
      rom_code_(other.rom_code_),
      reading_(other.reading_) {
  add(rom_code_);
  add(reading_);
}

void UnassignedThermometerRadioGroupItem::set(std::string rom_code,
                                              std::string reading) {
  rom_code_.setText(std::move(rom_code));
  reading_.setText(std::move(reading));
}

int UnassignedThermometerRadioGroupModel::elementCount() const {
  // return model_.unassigned().size();
  return model_.unassigned_sensors().size();
}

void UnassignedThermometerRadioGroupModel::set(
    int idx, UnassignedThermometerRadioGroupItem& dest) const {
  const roo_control::UniversalDeviceId id = model_.unassigned_sensors()[idx];
  std::string label = model_.sensors().sensorUserFriendlyName(id);
  roo_control::Measurement m = model_.sensors().read(id);
  CHECK_EQ(roo_control_Quantity_kTemperature, m.quantity());
  std::string reading;
  if (m.isUnknown() || m.value() >= 85 || m.value() <= -55) {
    reading = "";
  } else {
    reading = roo_display::StringPrintf("%2.1f°C", m.value());
  }
  dest.set(label, std::move(reading));
}

UnassignedThermometerSelectionDialog::UnassignedThermometerSelectionDialog(
    const roo_windows::Environment& env, Model& model)
    : roo_windows::RadioListDialog<UnassignedThermometerRadioGroupModel>(env),
      model_(model),
      list_model_(model) {
  setTitle(kStrSelectThermometer);
  setModel(list_model_);
}

void UnassignedThermometerSelectionDialog::onEnter() {
  model_.addEventListener(this);
  model_.sensors().requestUpdate();
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
    for (size_t i = 0; i < model_.unassigned_sensors().size(); ++i) {
      if (model_.unassigned_sensors()[i] == selected_device_id_) {
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
  selected_device_id_ = (s >= 0) ? model_.unassigned_sensors()[s]
                                 : roo_control::UniversalDeviceId();
  RadioListDialog::onChange();
}

}  // namespace roo_windows_onewire
