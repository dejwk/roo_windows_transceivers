#include "roo_windows_transceivers/activity/list_activity.h"

#include "roo_windows/core/application.h"
#include "roo_windows_transceivers/model.h"

namespace roo_windows_transceivers {

ListItem::ListItem(roo_windows::ApplicationContext& env,
                   ItemSelectedFn on_click, const Ui* device_state_ui)
    : FlexLayout(env, roo_windows::FlexDirection::kRow),
      thermometer_icon_(env, *device_state_ui->icon),
      id_(env, "", roo_windows::material2::text_style_subtitle1()),
      reading_(device_state_ui->widget_creator_fn()),
      on_click_(on_click),
      device_state_ui_(device_state_ui) {
  setAlignItems(roo_windows::AlignItems::kCenter);
  setPadding(roo_windows::Padding(roo_windows::PaddingSize::kSmall,
                                  roo_windows::PaddingSize::kTiny));
  setGap(roo_windows::Scaled(8));
  add(thermometer_icon_, {.flex_grow = 0, .flex_shrink = 0});

  id_.setMargins(roo_windows::MarginSize::kNone);
  id_.setPadding(roo_windows::PaddingSize::kNone);
  add(id_, {.flex_grow = 1, .flex_shrink = 1});

  // reading_.setMargins(roo_windows::MarginSize::kNone);
  // reading_.setPadding(roo_windows::PaddingSize::kRegular,
  // PaddingSize::kTiny);
  add(*reading_, {.flex_grow = 0, .flex_shrink = 0});
}

void ListItem::set(int idx, const Model& model) {
  idx_ = idx;
  id_.setText(model.getBindingLabel(idx_));
  // roo_control::Measurement m = model.sensors().read(model.getBinding(idx_));
  device_state_ui_->widget_setter_fn(model.getBindingItemId(idx_), *reading_);
  thermometer_icon_.setVisibility(model.isBound(idx_)
                                      ? roo_windows::Visibility::kVisible
                                      : roo_windows::Visibility::kInvisible);
}

ListModel::ListModel(Model& model) : model_(model) {}

int ListModel::elementCount() const { return model_.getBindingCount(); }

void ListModel::set(int idx, roo_windows::Widget& dest) const {
  ((ListItem&)dest).set(idx, model_);
}

ListActivity::ListActivity(roo_windows::ApplicationContext& env,
                           roo_scheduler::Scheduler& scheduler, Model& model,
                           ItemSelectedFn thermometer_selected_fn)
    : model_(model),
      contents_(env, model, thermometer_selected_fn),
      scrollable_container_(env, contents_),
      refresh_task_(scheduler, roo_time::Millis(1000),
                    [this]() { model_.requestUpdate(); }) {}

ListActivityContents::ListActivityContents(
    roo_windows::ApplicationContext& env, Model& model,
    ItemSelectedFn thermometer_selected_fn)
    : FlexLayout(env, roo_windows::FlexDirection::kColumn),
      model_(model),
      title_(env, model.ui()->labels.list_title),
      list_model_(model),
      list_(env, list_model_, [&, thermometer_selected_fn]() {
        return std::unique_ptr<Widget>(
            new ListItem(env, thermometer_selected_fn, model.ui()));
      }) {
  add(title_, {.flex_grow = 0, .flex_shrink = 0});
  add(list_, {.flex_grow = 0, .flex_shrink = 1});
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
