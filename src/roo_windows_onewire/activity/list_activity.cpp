#include "roo_windows_onewire/activity/list_activity.h"

#include "roo_windows/core/application.h"
#include "roo_windows_onewire/activity/model.h"
#include "roo_windows_onewire/activity/resources.h"

namespace roo_windows_onewire {

ThermometerListItem::ThermometerListItem(const roo_windows::Environment& env,
                                         ThermometerSelectedFn on_click,
                                         const DeviceStateUi* device_state_ui)
    : HorizontalLayout(env),
      thermometer_icon_(env, *device_state_ui->icon),
      id_(env, "", roo_windows::font_subtitle1()),
      reading_(device_state_ui->creator_fn()),
      on_click_(on_click),
      device_state_ui_(device_state_ui) {
  setGravity(roo_windows::Gravity(roo_windows::kHorizontalGravityNone,
                                  roo_windows::kVerticalGravityMiddle));
  add(thermometer_icon_);

  id_.setMargins(roo_windows::MARGIN_NONE);
  id_.setPadding(roo_windows::PADDING_TINY);
  add(id_, HorizontalLayout::Params().setWeight(1));

  // reading_.setMargins(roo_windows::MARGIN_NONE);
  // reading_.setPadding(roo_windows::PADDING_REGULAR, PADDING_TINY);
  add(*reading_, HorizontalLayout::Params().setWeight(0));
}

ThermometerListItem::ThermometerListItem(const ThermometerListItem& other)
    : HorizontalLayout(other),
      thermometer_icon_(other.thermometer_icon_),
      id_(other.id_),
      reading_(other.device_state_ui_->creator_fn()),
      on_click_(other.on_click_),
      device_state_ui_(other.device_state_ui_) {
  add(thermometer_icon_);
  add(id_, HorizontalLayout::Params().setWeight(1));
  add(*reading_, HorizontalLayout::Params().setWeight(0));
}

void ThermometerListItem::set(int idx, const ThermometerSelectorModel& model) {
  idx_ = idx;
  id_.setText(model.getBindingLabel(idx_));
  // roo_control::Measurement m = model.sensors().read(model.getBinding(idx_));
  device_state_ui_->setter_fn(model.getBindingItemId(idx_), *reading_);
  thermometer_icon_.setVisibility(model.isBound(idx_) ? VISIBLE : INVISIBLE);
}

ThermometerListModel::ThermometerListModel(ThermometerSelectorModel& model) : model_(model) {}

int ThermometerListModel::elementCount() const {
  return model_.getBindingCount();
}

void ThermometerListModel::set(int idx, ThermometerListItem& dest) const {
  dest.set(idx, model_);
}

ListActivity::ListActivity(const roo_windows::Environment& env,
                           roo_scheduler::Scheduler& scheduler, ThermometerSelectorModel& model,
                           ThermometerSelectedFn thermometer_selected_fn)
    : model_(model),
      contents_(env, model, thermometer_selected_fn),
      scrollable_container_(env, contents_),
      refresh_task_(
          scheduler, [this]() { model_.requestUpdate(); },
          roo_time::Millis(1000)) {}

ListActivityContents::ListActivityContents(
    const roo_windows::Environment& env, ThermometerSelectorModel& model,
    ThermometerSelectedFn thermometer_selected_fn)
    : VerticalLayout(env),
      model_(model),
      title_(env, kStrThermometers),
      list_model_(model),
      list_(
          env, list_model_,
          ThermometerListItem(env, thermometer_selected_fn, model.state_ui())) {
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

}  // namespace roo_windows_onewire
