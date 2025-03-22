#pragma once

#include "roo_icons/filled/device.h"
#include "roo_windows_onewire/model/simple_model.h"
#include "roo_windows_onewire/model/thermometers/resources.h"

namespace roo_windows_onewire {

class ThermometerSelectorModel
    : public SimpleTransceiverSelectorModel<roo_transceivers::SensorLocator,
                                            roo_transceivers::SensorBinding> {
 public:
  using Base = SimpleTransceiverSelectorModel<roo_transceivers::SensorLocator,
                                              roo_transceivers::SensorBinding>;

  ThermometerSelectorModel(
      const roo_windows::Environment* env,
      roo_transceivers::Universe& transceivers,
      std::vector<ModelItem<roo_transceivers::SensorBinding>> bindings)
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
    state_ui_.canonical_id = "1-Wire:DDDDDDDDDDDDDDDD";
    state_ui_.labels = {
        .list_title = kStrThermometers,
        .item_details_title = kStrThermometerDetails,
        .assign = kStrAssign,
        .unassign = kStrUnassign,
        .unassign_question = kStrUnassignQuestion,
        .unassign_question_supporting_text = kStrUnassignSupportingText,
        .unassigned = kStrNotAssigned,
        .assign_from_list = kStrSelectThermometer};
    updateSensors();
    notifyItemsChanged();
  }

  const Ui* ui() const override { return &state_ui_; }

 private:
  std::string itemToString(
      const roo_transceivers::SensorLocator& locator) const override {
    char buf[64];
    locator.write_cstr(buf);
    return buf;
  }

  void maybeAddTransceiver(
      const roo_transceivers::DeviceLocator& device,
      const roo_transceivers_Descriptor& descriptor) override {
    for (size_t sensor_idx = 0; sensor_idx < descriptor.sensors_count;
         ++sensor_idx) {
      if (descriptor.sensors[sensor_idx].quantity !=
          roo_transceivers_Quantity_kTemperature) {
        continue;
      }
      roo_transceivers::SensorLocator sensor_loc(
          device, descriptor.sensors[sensor_idx].id);
      addItem(sensor_loc);
    }
  }

  void updateDisplayValue(roo_io::string_view item_id,
                          roo_windows::TextLabel& dest) const {
    const roo_transceivers::SensorLocator* sensor_loc = lookupItem(item_id);
    if (sensor_loc == nullptr) {
      dest.setText("");
      return;
    }
    roo_transceivers::Measurement m = transceivers_.read(*sensor_loc);
    if (!m.isDefined()) {
      dest.setText("");
    } else {
      // if (m.value() >= 85 || m.value() <= -55) {
      //   label.setTextf("");
      // } else {
      CHECK_EQ(roo_transceivers_Quantity_kTemperature, m.quantity());
      dest.setTextf("%3.1f°C", m.value());
      // }
    }
  }

  Ui state_ui_;
};

}  // namespace roo_windows_onewire
