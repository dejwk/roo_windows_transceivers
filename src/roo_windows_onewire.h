#pragma once

#include <Arduino.h>

#include "roo_control/sensors/binding/binding.h"
#include "roo_windows/dialogs/string_constants.h"
#include "roo_windows/locale/languages.h"
#include "roo_windows_onewire/activity/list_activity.h"
#include "roo_windows_onewire/activity/model.h"
#include "roo_windows_onewire/activity/thermometer_assignment_dialog.h"
#include "roo_windows_onewire/activity/thermometer_details_activity.h"

namespace roo_windows_onewire {

class Configurator {
 public:
  Configurator(const roo_windows::Environment& env,
               roo_control::SensorUniverse& sensors,
               std::vector<ModelItem> bindings)
      : model_(sensors, std::move(bindings)),
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
        model_.bind(idx, model_.unassigned_sensors()[assignment_.selected()]);
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

  Model model_;
  ListActivity list_;
  ThermometerDetailsActivity details_;
  UnassignedThermometerSelectionDialog assignment_;
};

}  // namespace roo_windows_onewire
