#pragma once

#include <Arduino.h>

#include <functional>
#include <memory>
#include <vector>

#include "roo_collections/flat_small_hash_set.h"
#include "roo_control/sensors/binding/binding.h"
#include "roo_control/sensors/sensor.h"
#include "roo_windows/core/widget.h"
#include "roo_windows/widgets/text_label.h"

namespace roo_windows_onewire {

struct ModelItem {
  roo_control::SensorBinding& binding;
  std::string label;
};

struct DeviceStateUi {
  roo_windows::WidgetCreatorFn creator_fn;
  roo_windows::WidgetSetterFn<roo_io::string_view> setter_fn;
};

class Model : public roo_control::SensorEventListener {
 public:
  Model(const roo_windows::Environment* env,
        roo_control::SensorUniverse& sensors, std::vector<ModelItem> bindings)
      : sensors_(sensors), bindings_(bindings) {
    // state_ui_(TemperatureWidgetSetter(env, &sensors, *this)) {
    state_ui_.creator_fn = [env]() {
      return std::unique_ptr<roo_windows::Widget>(
          new roo_windows::TextLabel(*env, "", roo_windows::font_subtitle1()));
    };
    state_ui_.setter_fn = [this](roo_io::string_view id,
                                 roo_windows::Widget& dest) {
      roo_control::UniversalDeviceId device_id = deviceIdFromName(id);
      roo_control::Measurement m = sensors_.read(device_id);
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
    sensors_.addEventListener(this);
  }

  ~Model() { sensors_.removeEventListener(this); }

  Model(const Model&) = delete;
  Model(Model&&) = delete;

  Model& operator=(const Model& m) = delete;

  void requestUpdate() { sensors_.requestUpdate(); }

  size_t binding_count() const { return bindings_.size(); }

  roo_io::string_view binding_label(int idx) const {
    return bindings_[idx].label;
  }

  roo_io::string_view getBinding(int idx) const { return binding_ids_[idx]; }

  void bind(int idx, roo_io::string_view id) {
    bindings_[idx].binding.bind(item_id_mapping_[id]);
    binding_ids_[idx] = std::string(id);
  }

  void unbind(int idx) {
    bindings_[idx].binding.unbind();
    binding_ids_[idx] = "";
  }

  bool isBound(int idx) const { return bindings_[idx].binding.isBound(); }

  void addEventListener(SensorEventListener* listener) {
    auto result = event_listeners_.insert(listener);
    CHECK(result.second) << "Event listener " << listener
                         << " was registered already.";
  }

  void removeEventListener(SensorEventListener* listener) {
    event_listeners_.erase(listener);
  }

  size_t availableItemCount() const { return all_sensors_.size(); }

  roo_io::string_view availableItemId(size_t idx) { return all_item_ids_[idx]; }

  roo_control::UniversalDeviceId deviceIdFromName(
      roo_io::string_view id) const {
    auto itr = item_id_mapping_.find(id);
    if (itr == item_id_mapping_.end()) {
      return roo_control::UniversalDeviceId();
    }
    return itr->second;
  }

  size_t unassignedItemCount() const { return unassigned_sensors_.size(); }

  roo_io::string_view unassignedItemId(size_t idx) const {
    return all_item_ids_[unassigned_sensors_[idx]];
  }

  size_t unassignedItemIdx(size_t idx) const {
    return unassigned_sensors_[idx];
  }

  void sensorsChanged() override {
    updateSensors();
    for (auto& listener : event_listeners_) {
      listener->sensorsChanged();
    }
  }

  void newReadingsAvailable() override {
    for (auto& listener : event_listeners_) {
      listener->newReadingsAvailable();
    }
  }

  const DeviceStateUi* state_ui() const { return &state_ui_; }

 private:
  // Zero-initialized integer.
  struct RefCounter {
    RefCounter() : val(0) {}
    int val;
    void increment() { ++val; }
    operator int() const { return val; }
  };

  void updateSensors() {
    all_sensors_.clear();
    all_item_ids_.clear();
    binding_ids_.clear();
    unassigned_sensors_.clear();
    item_id_mapping_.clear();

    binding_counts_.clear();
    for (size_t i = 0; i < bindings_.size(); ++i) {
      roo_control::UniversalDeviceId id = bindings_[i].binding.get();
      if (!id.isDefined()) {
        binding_ids_.push_back("");
        continue;
      }
      binding_ids_.push_back(
          sensors_.sensorUserFriendlyName(bindings_[i].binding.get()));
      binding_counts_[id].increment();
    }

    for (size_t i = 0; i < sensors_.familyCount(); ++i) {
      const roo_control::SensorFamily& family = sensors_.family(i);
      size_t sensor_count = family.sensorCount();
      roo_control::SensorFamilyId family_id = sensors_.family_id(i);
      for (size_t j = 0; j < sensor_count; j++) {
        roo_control::UniversalDeviceId id(family_id, family.sensorId(j));
        if (!binding_counts_.contains(id)) {
          unassigned_sensors_.push_back(all_sensors_.size());
        }
        all_sensors_.push_back(id);
        all_item_ids_.push_back(sensors_.sensorUserFriendlyName(id));
      }
    }
    for (size_t i = 0; i < all_sensors_.size(); ++i) {
      item_id_mapping_[all_item_ids_[i]] = all_sensors_[i];
    }
  }

  roo_control::SensorUniverse& sensors_;
  std::vector<ModelItem> bindings_;

  roo_collections::FlatSmallHashSet<SensorEventListener*> event_listeners_;

  roo_collections::FlatSmallHashMap<roo_control::UniversalDeviceId, RefCounter>
      binding_counts_;

  std::vector<roo_control::UniversalDeviceId> all_sensors_;
  std::vector<std::string> all_item_ids_;
  std::vector<std::string> binding_ids_;
  std::vector<size_t> unassigned_sensors_;

  roo_collections::FlatSmallHashMap<roo_io::string_view,
                                    roo_control::UniversalDeviceId>
      item_id_mapping_;

  DeviceStateUi state_ui_;
};

}  // namespace roo_windows_onewire
