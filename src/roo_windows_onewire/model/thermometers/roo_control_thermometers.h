#pragma once

#include <Arduino.h>

#include "roo_control/transceivers/binding/binding.h"
#include "roo_control/transceivers/notification.h"
#include "roo_icons/filled/device.h"
#include "roo_windows/dialogs/string_constants.h"
#include "roo_windows/locale/languages.h"
#include "roo_windows_onewire/model.h"
#include "roo_windows_onewire/model/thermometers/resources.h"

namespace roo_windows_onewire {

struct ModelItem {
  roo_control::SensorBinding& binding;
  std::string label;
};

class NewThermometerSelectorModel
    : public roo_control::TransceiverEventListener,
      public Model {
 public:
  NewThermometerSelectorModel(const roo_windows::Environment* env,
                              roo_control::TransceiverUniverse& sensors,
                              std::vector<ModelItem> bindings)
      : sensors_(sensors), bindings_(bindings) {
    // state_ui_(TemperatureWidgetSetter(env, &sensors, *this)) {
    state_ui_.widget_creator_fn = [env]() {
      return std::unique_ptr<roo_windows::Widget>(
          new roo_windows::TextLabel(*env, "", roo_windows::font_subtitle1()));
    };
    state_ui_.widget_setter_fn = [this](roo_io::string_view item_id,
                                        roo_windows::Widget& dest) {
      roo_control::TransceiverSensorLocator sensor_loc =
          deviceIdFromItemId(item_id);
      roo_control::Measurement m = sensors_.read(sensor_loc);
      auto& label = (roo_windows::TextLabel&)dest;
      if (!m.isDefined()) {
        label.setText("");
      } else {
        // if (m.value() >= 85 || m.value() <= -55) {
        //   label.setTextf("");
        // } else {
        CHECK_EQ(roo_control_Quantity_kTemperature, m.quantity());
        label.setTextf("%3.1f°C", m.value());
        // }
      }
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
    sensors_.addEventListener(this);
  }

  ~NewThermometerSelectorModel() { sensors_.removeEventListener(this); }

  NewThermometerSelectorModel(const NewThermometerSelectorModel&) = delete;
  NewThermometerSelectorModel(NewThermometerSelectorModel&&) = delete;

  NewThermometerSelectorModel& operator=(const NewThermometerSelectorModel& m) =
      delete;

  void requestUpdate() override { sensors_.requestUpdate(); }

  size_t getBindingCount() const override { return bindings_.size(); }

  roo_io::string_view getBindingLabel(size_t idx) const override {
    return bindings_[idx].label;
  }

  roo_io::string_view getBindingItemId(size_t idx) const override {
    return binding_ids_[idx];
  }

  void bind(size_t idx, roo_io::string_view item_id) override {
    bindings_[idx].binding.bind(item_id_mapping_[item_id]);
    binding_ids_[idx] = std::string(item_id.data(), item_id.size());
  }

  void unbind(size_t idx) override {
    bindings_[idx].binding.unbind();
    binding_ids_[idx] = "";
  }

  bool isBound(size_t idx) const override {
    return bindings_[idx].binding.isBound();
  }

  size_t getItemCount() const override { return all_sensors_.size(); }

  roo_io::string_view getItemId(size_t idx) const override {
    return all_item_ids_[idx];
  }

  void devicesChanged() override {
    updateSensors();
    notifyItemsChanged();
  }

  void newReadingsAvailable() override { notifyMeasurementsChanged(); }

  const Ui* ui() const override { return &state_ui_; }

 private:
  void updateSensors() {
    all_sensors_.clear();
    all_item_ids_.clear();
    binding_ids_.clear();
    item_id_mapping_.clear();

    char buf[64];
    for (size_t i = 0; i < bindings_.size(); ++i) {
      roo_control::TransceiverSensorLocator loc = bindings_[i].binding.get();
      if (!loc.isDefined()) {
        binding_ids_.push_back("");
        continue;
      }
      bindings_[i].binding.get().write_cstr(buf);
      binding_ids_.emplace_back(buf);
    }

    size_t device_count = sensors_.deviceCount();
    roo_control_TransceiverDescriptor descriptor;
    for (size_t j = 0; j < device_count; j++) {
      roo_control::TransceiverDeviceLocator device_loc = sensors_.device(j);
      if (!sensors_.getDeviceDescriptor(device_loc, descriptor)) continue;
      for (size_t sensor_idx = 0; sensor_idx < descriptor.sensors_count;
           ++sensor_idx) {
        if (descriptor.sensors[sensor_idx].quantity !=
            roo_control_Quantity_kTemperature) {
          continue;
        }
        roo_control::TransceiverSensorLocator sensor_loc(
            device_loc, descriptor.sensors[sensor_idx].id);
        all_sensors_.push_back(sensor_loc);
        sensor_loc.write_cstr(buf);
        all_item_ids_.emplace_back(buf);
      }
    }
    for (size_t i = 0; i < all_sensors_.size(); ++i) {
      item_id_mapping_[all_item_ids_[i]] = all_sensors_[i];
    }
  }

  roo_control::TransceiverSensorLocator deviceIdFromItemId(
      roo_io::string_view item_id) const {
    auto itr = item_id_mapping_.find(item_id);
    if (itr == item_id_mapping_.end()) {
      return roo_control::TransceiverSensorLocator();
    }
    return itr->second;
  }

  roo_control::TransceiverUniverse& sensors_;
  std::vector<ModelItem> bindings_;

  std::vector<roo_control::TransceiverSensorLocator> all_sensors_;
  std::vector<std::string> all_item_ids_;
  std::vector<std::string> binding_ids_;

  roo_collections::FlatSmallHashMap<roo_io::string_view,
                                    roo_control::TransceiverSensorLocator>
      item_id_mapping_;

  Ui state_ui_;
};

}  // namespace roo_windows_onewire
