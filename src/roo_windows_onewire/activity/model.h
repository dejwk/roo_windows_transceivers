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
  roo_windows::WidgetSetterFn<roo_control::UniversalDeviceId> setter_fn;
};

// TODO: refactor inline.
inline DeviceStateUi TemperatureWidgetSetter(
    const roo_windows::Environment* env,
    const roo_control::SensorUniverse* sensor_universe) {
  return DeviceStateUi{
      .creator_fn =
          [env]() {
            return std::unique_ptr<roo_windows::Widget>(
                new roo_windows::TextLabel(*env, "",
                                           roo_windows::font_subtitle1()));
          },
      .setter_fn =
          [sensor_universe](roo_control::UniversalDeviceId id,
                            roo_windows::Widget& dest) {
            roo_control::Measurement m = sensor_universe->read(id);
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
          }};
}

class Model : public roo_control::SensorEventListener {
 public:
  Model(roo_control::SensorUniverse& sensors, std::vector<ModelItem> bindings,
        DeviceStateUi& state_ui)
      : sensors_(sensors), bindings_(bindings), state_ui_(state_ui) {
    sensors_.addEventListener(this);
  }

  ~Model() { sensors_.removeEventListener(this); }

  Model(const Model&) = delete;
  Model(Model&&) = delete;

  Model& operator=(const Model& m) = delete;

  roo_control::SensorUniverse& sensors() { return sensors_; }

  const roo_control::SensorUniverse& sensors() const { return sensors_; }

  size_t binding_count() const { return bindings_.size(); }

  const std::string& binding_label(int idx) const {
    return bindings_[idx].label;
    ;
  }

  void bind(int idx, roo_control::UniversalDeviceId id) {
    bindings_[idx].binding.bind(id);
  }

  void unbind(int idx) { bindings_[idx].binding.unbind(); }

  bool isBound(int idx) const { return bindings_[idx].binding.isBound(); }

  roo_control::UniversalDeviceId getBinding(int idx) const {
    return bindings_[idx].binding.get();
  }

  void addEventListener(SensorEventListener* listener) {
    auto result = event_listeners_.insert(listener);
    CHECK(result.second) << "Event listener " << listener
                         << " was registered already.";
  }

  void removeEventListener(SensorEventListener* listener) {
    event_listeners_.erase(listener);
  }

  // const std::vector<roo_control::UniversalDeviceId> all_sensors() const {
  //   return all_sensors_;
  // }

  const std::vector<roo_control::UniversalDeviceId>& unassigned_sensors()
      const {
    return unassigned_sensors_;
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
    unassigned_sensors_.clear();

    binding_counts_.clear();
    for (size_t i = 0; i < bindings_.size(); ++i) {
      roo_control::UniversalDeviceId id = bindings_[i].binding.get();
      if (!id.isDefined()) continue;
      binding_counts_[id].increment();
    }

    for (size_t i = 0; i < sensors_.familyCount(); ++i) {
      const roo_control::SensorFamily& family = sensors_.family(i);
      size_t sensor_count = family.sensorCount();
      roo_control::SensorFamilyId family_id = sensors_.family_id(i);
      for (size_t j = 0; j < sensor_count; j++) {
        roo_control::UniversalDeviceId id(family_id, family.sensorId(j));
        all_sensors_.push_back(id);
        if (!binding_counts_.contains(id)) {
          unassigned_sensors_.push_back(id);
        }
      }
    }
  }

  roo_control::SensorUniverse& sensors_;
  std::vector<ModelItem> bindings_;

  roo_collections::FlatSmallHashSet<SensorEventListener*> event_listeners_;

  roo_collections::FlatSmallHashMap<roo_control::UniversalDeviceId, RefCounter>
      binding_counts_;

  std::vector<roo_control::UniversalDeviceId> all_sensors_;
  std::vector<roo_control::UniversalDeviceId> unassigned_sensors_;

  DeviceStateUi& state_ui_;
};

}  // namespace roo_windows_onewire
