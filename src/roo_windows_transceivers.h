#pragma once

#include "roo_windows_transceivers/activity/assignment_dialog.h"
#include "roo_windows_transceivers/activity/details_activity.h"
#include "roo_windows_transceivers/activity/list_activity.h"
#include "roo_windows_transceivers/model.h"

#include "roo_windows_transceivers/widgets/descriptor.h"

namespace roo_windows_transceivers {

class Configurator {
 public:
  Configurator(const roo_windows::Environment& env, Model& model)
      : model_(model),
        list_(env, env.scheduler(), model_,
              [this](roo_windows::Task& task, int idx) {
                itemSelected(task, idx);
              }),
        details_(
            env, model_,
            [this](roo_windows::Task& task, int idx) { assignItem(task, idx); },
            [this](roo_windows::Task& task, int idx) {
              unassignItem(task, idx);
            }),
        assignment_(env, model_) {}

  roo_windows::Activity& main() { return list_; }

  void itemSelected(roo_windows::Task& task, int idx) {
    if (model_.isBound(idx)) {
      details_.enter(task, idx);
    } else {
      assignItem(task, idx);
    }
  }

  void assignItem(roo_windows::Task& task, int idx) {
    task.showDialog(assignment_, [&task, this, idx](int dialog_response_id) {
      if (dialog_response_id == 1) {
        model_.bind(idx, model_.getUnassignedItemId(assignment_.selected()));
      }
    });
  }

  void unassignItem(roo_windows::Task& task, int idx) {
    task.showAlertDialog(
        model_.ui()->labels.unassign_question,
        model_.ui()->labels.unassign_question_supporting_text,
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

}  // namespace roo_windows_transceivers
