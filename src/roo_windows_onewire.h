#pragma once

#include <Arduino.h>

#include "roo_control/sensors/binding/binding.h"
#include "roo_windows/dialogs/string_constants.h"
#include "roo_windows/locale/languages.h"
#include "roo_windows_onewire/activity/assignment_dialog.h"
#include "roo_windows_onewire/activity/details_activity.h"
#include "roo_windows_onewire/activity/list_activity.h"
#include "roo_windows_onewire/activity/model.h"

namespace roo_windows_onewire {

struct ModelItem {
  roo_control::SensorBinding& binding;
  std::string label;
};

class ThermometerSelectorModel : public roo_control::SensorEventListener,
                                 public Model {
 public:
  ThermometerSelectorModel(const roo_windows::Environment* env,
                           roo_control::SensorUniverse& sensors,
                           std::vector<ModelItem> bindings)
      : sensors_(sensors), bindings_(bindings) {
    // state_ui_(TemperatureWidgetSetter(env, &sensors, *this)) {
    state_ui_.widget_creator_fn = [env]() {
      return std::unique_ptr<roo_windows::Widget>(
          new roo_windows::TextLabel(*env, "", roo_windows::font_subtitle1()));
    };
    state_ui_.widget_setter_fn = [this](roo_io::string_view item_id,
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

  roo_io::string_view getItemId(size_t idx) const override {
    return all_item_ids_[idx];
  }

  void sensorsChanged() override {
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

    for (size_t i = 0; i < bindings_.size(); ++i) {
      roo_control::UniversalDeviceId id = bindings_[i].binding.get();
      if (!id.isDefined()) {
        binding_ids_.push_back("");
        continue;
      }
      binding_ids_.push_back(
          sensors_.sensorUserFriendlyName(bindings_[i].binding.get()));
    }

    for (size_t i = 0; i < sensors_.familyCount(); ++i) {
      const roo_control::SensorFamily& family = sensors_.family(i);
      size_t sensor_count = family.sensorCount();
      roo_control::SensorFamilyId family_id = sensors_.family_id(i);
      for (size_t j = 0; j < sensor_count; j++) {
        roo_control::UniversalDeviceId id(family_id, family.sensorId(j));
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

  std::vector<roo_control::UniversalDeviceId> all_sensors_;
  std::vector<std::string> all_item_ids_;
  std::vector<std::string> binding_ids_;

  roo_collections::FlatSmallHashMap<roo_io::string_view,
                                    roo_control::UniversalDeviceId>
      item_id_mapping_;

  Ui state_ui_;
};

class Configurator {
 public:
  Configurator(const roo_windows::Environment& env, Model& model)
      : model_(model),
        list_(env, env.scheduler(), model_,
              [this](roo_windows::Task& task, int idx) {
                thermometerSelected(task, idx);
              }),
        details_(
            env, model_,
            [this](roo_windows::Task& task, int idx) {
              assignThermometer(task, idx);
            },
            [this](roo_windows::Task& task, int idx) {
              unassignThermometer(task, idx);
            }),
        assignment_(env, model_) {}

  roo_windows::Activity& main() { return list_; }

  void thermometerSelected(roo_windows::Task& task, int idx) {
    if (model_.isBound(idx)) {
      details_.enter(task, idx);
    } else {
      assignThermometer(task, idx);
    }
  }

  void assignThermometer(roo_windows::Task& task, int idx) {
    task.showDialog(assignment_, [&task, this, idx](int dialog_response_id) {
      if (dialog_response_id == 1) {
        model_.bind(idx, model_.getUnassignedItemId(assignment_.selected()));
      }
    });
  }

  void unassignThermometer(roo_windows::Task& task, int idx) {
    task.showAlertDialog(
        kStrUnassignQuestion, kStrUnassignSupportingText,
        {roo_windows::kStrDialogCancel, roo_windows::kStrDialogOK},
        [this, idx](int id) {
          if (id == 1) {
            model_.unbind(idx);
          }
        });
  }

  Model& model_;
  ListActivity list_;
  DetailsActivity details_;
  UnassignedItemSelectionDialog assignment_;
};

}  // namespace roo_windows_onewire
