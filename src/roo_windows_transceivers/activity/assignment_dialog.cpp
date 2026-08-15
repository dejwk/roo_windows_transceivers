#include "roo_windows_transceivers/activity/assignment_dialog.h"

using roo_windows::Dialog;

namespace roo_windows_transceivers {

UnassignedRadioGroupItem::UnassignedRadioGroupItem(
    roo_windows::ApplicationContext& env, const Ui* ui)
    : HorizontalLayout(env),
      id_(env, ui->canonical_id, roo_windows::material2::text_style_subtitle1()),
      reading_(ui->widget_creator_fn()),
      ui_(ui) {
  setGravity(roo_windows::kGravityMiddle);
  id_.setMargins(roo_windows::MarginSize::kNone);
  id_.setPadding(roo_windows::PaddingSize::kSmall);
  add(id_, {weight : 1});
  add(*reading_);
}

void UnassignedRadioGroupItem::set(std::string id) {
  ui_->widget_setter_fn(id, *reading_);
  id_.setText(std::move(id));
}

int UnassignedRadioGroupModel::elementCount() const {
  // return model_.unassigned().size();
  return model_.getUnassignedItemCount();
}

void UnassignedRadioGroupModel::set(int idx, roo_windows::Widget& dest) const {
  roo::string_view v = model_.getUnassignedItemId(idx);
  ((UnassignedRadioGroupItem&)dest).set(std::string(v.data(), v.size()));
}

UnassignedItemSelectionDialog::UnassignedItemSelectionDialog(
    roo_windows::ApplicationContext& env, Model& model)
    : roo_windows::RadioListDialog(
          env,
          [&]() {
            return std::make_unique<UnassignedRadioGroupItem>(env, model.ui());
          }),
      model_(model),
      list_model_(model) {
  setTitle(model.ui()->labels.assign_from_list);
  setModel(static_cast<roo_windows::ListModel&>(list_model_));
}

void UnassignedItemSelectionDialog::onEnter() {
  model_.addEventListener(this);
  model_.requestUpdate();
  reset();
}

void UnassignedItemSelectionDialog::onExit(int result) {
  model_.removeEventListener(this);
}

void UnassignedItemSelectionDialog::itemsChanged() {
  int s = selected();
  int new_s = -1;
  if (s >= 0) {
    // Update the selection to stick to the originally selected rom code.
    for (size_t i = 0; i < model_.getUnassignedItemCount(); ++i) {
      if (model_.getUnassignedItemId(i) == selected_device_id_) {
        // Found!
        new_s = i;
        break;
      }
    }
  }
  contentsChanged();
  setSelected(new_s);
}

void UnassignedItemSelectionDialog::measurementsChanged() { contentsChanged(); }

void UnassignedItemSelectionDialog::onChange() {
  int s = selected();
  selected_device_id_ = "";
  if (s >= 0) {
    roo::string_view v = model_.getUnassignedItemId(s);
    selected_device_id_ = std::string(v.data(), v.size());
  }
  RadioListDialog::onChange();
}

}  // namespace roo_windows_transceivers
