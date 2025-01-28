#pragma once

#include <Arduino.h>

#include <functional>
#include <memory>
#include <vector>

#include "roo_collections/flat_small_hash_set.h"
#include "roo_control/sensors/binding/binding.h"
#include "roo_control/sensors/sensor.h"
#include "roo_icons/filled/device.h"
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
  const roo_display::Pictogram* icon;
};

class Model {
 public:
  class EventListener {
   public:
    virtual ~EventListener() = default;

    virtual void itemsChanged() {}
    virtual void measurementsChanged() {}
  };

  virtual ~Model() = default;

  virtual void requestUpdate() = 0;

  virtual size_t getItemCount() const = 0;
  virtual roo_io::string_view getItemId(size_t idx) = 0;

  virtual size_t getBindingCount() const = 0;
  virtual roo_io::string_view getBindingLabel(size_t idx) const = 0;
  virtual roo_io::string_view getBindingItemId(size_t idx) const = 0;
  virtual bool isBound(size_t idx) const = 0;

  virtual void bind(size_t idx, roo_io::string_view item_id) = 0;
  virtual void unbind(size_t idx) = 0;

  virtual const DeviceStateUi* state_ui() const = 0;

  void addEventListener(EventListener* listener) {
    auto result = event_listeners_.insert(listener);
    CHECK(result.second) << "Event listener " << listener
                         << " was registered already.";
  }

  void removeEventListener(EventListener* listener) {
    event_listeners_.erase(listener);
  }

 protected:
  void notifyItemsChanged() {
    for (auto& listener : event_listeners_) {
      listener->itemsChanged();
    }
  }

  void notifyMeasurementsChanged() {
    for (auto& listener : event_listeners_) {
      listener->measurementsChanged();
    }
  }

 private:
  roo_collections::FlatSmallHashSet<EventListener*> event_listeners_;
};

class ThermometerSelectorModel : public roo_control::SensorEventListener,
                                 public Model {
 public:
  ThermometerSelectorModel(const roo_windows::Environment* env,
                           roo_control::SensorUniverse& sensors,
                           std::vector<ModelItem> bindings)
      : sensors_(sensors), bindings_(bindings) {
    // state_ui_(TemperatureWidgetSetter(env, &sensors, *this)) {
    state_ui_.creator_fn = [env]() {
      return std::unique_ptr<roo_windows::Widget>(
          new roo_windows::TextLabel(*env, "", roo_windows::font_subtitle1()));
    };
    state_ui_.setter_fn = [this](roo_io::string_view item_id,
                                 roo_windows::Widget& dest) {
      roo_control::UniversalDeviceId device_id = deviceIdFromItemId(item_id);
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
    state_ui_.icon = &SCALED_ROO_ICON(filled, device_thermostat);
    sensors_.addEventListener(this);
  }

  ~ThermometerSelectorModel() { sensors_.removeEventListener(this); }

  ThermometerSelectorModel(const ThermometerSelectorModel&) = delete;
  ThermometerSelectorModel(ThermometerSelectorModel&&) = delete;

  ThermometerSelectorModel& operator=(const ThermometerSelectorModel& m) =
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

  roo_io::string_view getItemId(size_t idx) override {
    return all_item_ids_[idx];
  }

  size_t getUnassignedItemCount() const { return unassigned_sensors_.size(); }

  roo_io::string_view getUnassignedItemId(size_t idx) const {
    return all_item_ids_[unassigned_sensors_[idx]];
  }

  void sensorsChanged() override {
    updateSensors();
    notifyItemsChanged();
  }

  void newReadingsAvailable() override { notifyMeasurementsChanged(); }

  const DeviceStateUi* state_ui() const override { return &state_ui_; }

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

  roo_control::UniversalDeviceId deviceIdFromItemId(
      roo_io::string_view item_id) const {
    auto itr = item_id_mapping_.find(item_id);
    if (itr == item_id_mapping_.end()) {
      return roo_control::UniversalDeviceId();
    }
    return itr->second;
  }

  roo_control::SensorUniverse& sensors_;
  std::vector<ModelItem> bindings_;

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
