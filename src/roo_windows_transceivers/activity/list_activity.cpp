#include "roo_windows_transceivers/activity/list_activity.h"

#include "roo_windows/core/application.h"
#include "roo_windows_transceivers/model.h"

namespace roo_windows_transceivers {

ListItem::ListItem(const roo_windows::Environment& env, ItemSelectedFn on_click,
                   const Ui* device_state_ui)
    : HorizontalLayout(env),
      thermometer_icon_(env, *device_state_ui->icon),
      id_(env, "", roo_windows::font_subtitle1()),
      reading_(device_state_ui->widget_creator_fn()),
      on_click_(on_click),
      device_state_ui_(device_state_ui) {
  setGravity(roo_windows::kGravityMiddle);
  add(thermometer_icon_);

  id_.setMargins(roo_windows::MarginSize::NONE);
  id_.setPadding(roo_windows::PaddingSize::TINY);
  add(id_, {weight : 1});

  // reading_.setMargins(roo_windows::MarginSize::NONE);
  // reading_.setPadding(roo_windows::PaddingSize::REGULAR, PaddingSize::TINY);
  add(*reading_);
}

void ListItem::set(int idx, const Model& model) {
  idx_ = idx;
  id_.setText(model.getBindingLabel(idx_));
  // roo_control::Measurement m = model.sensors().read(model.getBinding(idx_));
  device_state_ui_->widget_setter_fn(model.getBindingItemId(idx_), *reading_);
  thermometer_icon_.setVisibility(model.isBound(idx_) ? VISIBLE : INVISIBLE);
}

ListModel::ListModel(Model& model) : model_(model) {}

int ListModel::elementCount() const { return model_.getBindingCount(); }

void ListModel::set(int idx, roo_windows::Widget& dest) const {
  ((ListItem&)dest).set(idx, model_);
}

ListActivity::ListActivity(const roo_windows::Environment& env,
                           roo_scheduler::Scheduler& scheduler, Model& model,
                           ItemSelectedFn thermometer_selected_fn)
    : model_(model),
      contents_(env, model, thermometer_selected_fn),
      scrollable_container_(env, contents_),
      refresh_task_(scheduler, roo_time::Millis(1000),
                    [this]() { model_.requestUpdate(); }) {}

ListActivityContents::ListActivityContents(
    const roo_windows::Environment& env, Model& model,
    ItemSelectedFn thermometer_selected_fn)
    : VerticalLayout(env),
      model_(model),
      title_(env, model.ui()->labels.list_title),
      list_model_(model),
      list_(env, list_model_, [&, thermometer_selected_fn]() {
        return std::unique_ptr<Widget>(
            new ListItem(env, thermometer_selected_fn, model.ui()));
      }) {
  add(title_);
  add(list_, VerticalLayout::Params());
}

void ListActivityContents::measurementsChanged() { list_.modelChanged(); }

void ListActivity::onStart() {
  model_.addEventListener(&contents_);
  refresh_task_.start();
}

void ListActivity::onStop() {
  refresh_task_.stop();
  model_.removeEventListener(&contents_);
}

}  // namespace roo_windows_transceivers
