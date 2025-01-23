#include "roo_windows_onewire/activity/list_activity.h"

#include "roo_windows/core/application.h"
#include "roo_windows_onewire/activity/model.h"
#include "roo_windows_onewire/activity/resources.h"

namespace roo_windows_onewire {

ThermometerListItem::ThermometerListItem(
    const roo_windows::Environment& env, ThermometerSelectedFn on_click,
    const Model::DisplayValueCreator& display_value_creator)
    : HorizontalLayout(env),
      env_(&env),
      thermometer_icon_(env, SCALED_ROO_ICON(filled, device_thermostat)),
      name_(env, "", roo_windows::font_subtitle1()),
      reading_(display_value_creator(env)),
      on_click_(on_click),
      display_value_creator_(display_value_creator) {
  setGravity(roo_windows::Gravity(roo_windows::kHorizontalGravityNone,
                                  roo_windows::kVerticalGravityMiddle));
  add(thermometer_icon_);

  name_.setMargins(roo_windows::MARGIN_NONE);
  name_.setPadding(roo_windows::PADDING_TINY);
  add(name_, HorizontalLayout::Params().setWeight(1));

  // reading_.setMargins(roo_windows::MARGIN_NONE);
  // reading_.setPadding(roo_windows::PADDING_REGULAR, PADDING_TINY);
  add(*reading_, HorizontalLayout::Params().setWeight(0));
}

ThermometerListItem::ThermometerListItem(const ThermometerListItem& other)
    : HorizontalLayout(other),
      thermometer_icon_(other.thermometer_icon_),
      name_(other.name_),
      reading_(other.display_value_creator_(*other.env_)),
      on_click_(other.on_click_),
      display_value_creator_(other.display_value_creator_) {
  add(thermometer_icon_);
  add(name_, HorizontalLayout::Params().setWeight(1));
  add(*reading_, HorizontalLayout::Params().setWeight(0));
}

// Sets this item to show the specified network.
void ThermometerListItem::set(int idx, const Model& model) {
  idx_ = idx;
  name_.setText(model.binding_label(idx_));
  roo_control::Measurement m = model.sensors().read(model.getBinding(idx_));
  model.setDisplayValue(*reading_, m);
  thermometer_icon_.setVisibility(model.isBound(idx_) ? VISIBLE : INVISIBLE);
}

ThermometerListModel::ThermometerListModel(Model& model) : model_(model) {}

int ThermometerListModel::elementCount() const {
  return model_.binding_count();
}

void ThermometerListModel::set(int idx, ThermometerListItem& dest) const {
  dest.set(idx, model_);
}

ListActivity::ListActivity(const roo_windows::Environment& env,
                           roo_scheduler::Scheduler& scheduler, Model& model,
                           ThermometerSelectedFn thermometer_selected_fn)
    : model_(model),
      contents_(env, model, thermometer_selected_fn),
      scrollable_container_(env, contents_),
      refresh_task_(
          scheduler, [this]() { model_.sensors().requestUpdate(); },
          roo_time::Millis(1000)) {}

ListActivityContents::ListActivityContents(
    const roo_windows::Environment& env, Model& model,
    ThermometerSelectedFn thermometer_selected_fn)
    : VerticalLayout(env),
      model_(model),
      title_(env, kStrThermometers),
      list_model_(model),
      list_(env, list_model_,
            ThermometerListItem(env, thermometer_selected_fn,
                                model.getDisplayValueCreator())) {
  add(title_);
  add(list_, VerticalLayout::Params());
}

void ListActivityContents::newReadingsAvailable() { list_.modelChanged(); }

void ListActivity::onStart() {
  model_.addEventListener(&contents_);
  refresh_task_.start();
}

void ListActivity::onStop() {
  refresh_task_.stop();
  model_.removeEventListener(&contents_);
}

}  // namespace roo_windows_onewire
