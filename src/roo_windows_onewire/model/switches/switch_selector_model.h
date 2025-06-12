#pragma once

#include "roo_icons/filled/device.h"
#include "roo_windows_onewire/model/simple_model.h"
#include "roo_windows_onewire/model/switches/resources.h"

namespace roo_windows_onewire {

class SwitchSelectorModel
    : public SimpleTransceiverSelectorModel<roo_transceivers::ActuatorLocator,
                                            roo_transceivers::ActuatorBinding> {
 public:
  using Base =
      SimpleTransceiverSelectorModel<roo_transceivers::ActuatorLocator,
                                     roo_transceivers::ActuatorBinding>;

  SwitchSelectorModel(
      const roo_windows::Environment* env,
      roo_transceivers::Universe& transceivers,
      std::vector<ModelItem<roo_transceivers::ActuatorBinding>> bindings)
      : Base(env, transceivers, std::move(bindings)) {
    state_ui_.widget_creator_fn = [env]() {
      return std::unique_ptr<roo_windows::Widget>(
          new roo_windows::TextLabel(*env, "", roo_windows::font_subtitle1()));
    };
    state_ui_.widget_setter_fn = [this](roo_io::string_view item_id,
                                        roo_windows::Widget& dest) {
      updateDisplayValue(item_id, (roo_windows::TextLabel&)dest);
    };
    state_ui_.icon = &SCALED_ROO_ICON(filled, device_thermostat);
    state_ui_.canonical_id = "roo:DD-DD-DD-DD-DD-DD/relay24";
    state_ui_.labels = {
        .list_title = kStrSwitches,
        .item_details_title = kStrSwitchDetails,
        .assign = kStrAssignSwitch,
        .unassign = kStrUnassignSwitch,
        .unassign_question = kStrUnassignSwitchQuestion,
        .unassign_question_supporting_text = kStrUnassignSwitchSupportingText,
        .unassigned = kStrSwitchNotAssigned,
        .assign_from_list = kStrSelectSwitch};
    updateSensors();
    notifyItemsChanged();
  }

  const Ui* ui() const override { return &state_ui_; }

 private:
  std::string itemToString(
      const roo_transceivers::ActuatorLocator& locator) const override {
    char buf[64];
    locator.write_cstr(buf);
    return buf;
  }

  void maybeAddTransceiver(
      const roo_transceivers::DeviceLocator& device,
      const roo_transceivers_Descriptor& descriptor) override {
    for (size_t actuator_idx = 0; actuator_idx < descriptor.actuators_count;
         ++actuator_idx) {
      if (descriptor.actuators[actuator_idx].quantity !=
          roo_transceivers_Quantity_kBinaryState) {
        continue;
      }
      if (strcmp(descriptor.sensors[actuator_idx].id,
                 descriptor.actuators[actuator_idx].id) != 0) {
        continue;
      }
      if (descriptor.sensors[actuator_idx].quantity !=
          roo_transceivers_Quantity_kBinaryState) {
        continue;
      }
      roo_transceivers::ActuatorLocator actuator_loc(
          device, descriptor.actuators[actuator_idx].id);
      addItem(actuator_loc);
    }
  }

  void updateDisplayValue(roo_io::string_view item_id,
                          roo_windows::TextLabel& dest) const {
    const roo_transceivers::ActuatorLocator* actuator = lookupItem(item_id);
    if (actuator == nullptr) {
      dest.setText("");
      return;
    }
    roo_transceivers::SensorLocator sensor(actuator->device_locator(),
                                           actuator->actuator_id());
    roo_transceivers::Measurement m = transceivers_.read(sensor);
    if (!m.isDefined()) {
      dest.setText("");
    } else {
      CHECK_EQ(roo_transceivers_Quantity_kBinaryState, m.quantity());
      dest.setTextf(m.value() == 0.0f   ? kStrSwitchOff
                    : m.value() == 1.0f ? kStrSwitchOn
                                        : "");
    }
  }

  Ui state_ui_;
};

}  // namespace roo_windows_onewire
