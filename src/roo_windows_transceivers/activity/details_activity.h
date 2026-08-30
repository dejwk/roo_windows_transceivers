#pragma once

#include <memory>

#include "roo_icons.h"
#include "roo_icons/filled/action.h"
#include "roo_icons/filled/content.h"
#include "roo_icons/filled/notification.h"
#include "roo_windows.h"
#include "roo_windows/composites/menu/title.h"
#include "roo_windows/config.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/containers/stacked_layout.h"
#include "roo_windows/core/destination.h"
#include "roo_windows/core/task.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/widgets/icon_with_caption.h"
#include "roo_windows/widgets/text_field.h"
#include "roo_windows_transceivers/model.h"

namespace roo_windows_transceivers {

typedef std::function<void(roo_windows::Task& task, int id)> SelectFn;

class DetailsActivityContents : public roo_windows::FlexLayout,
                                public Model::EventListener {
 public:
  DetailsActivityContents(roo_windows::ApplicationContext& env, Model& model,
                          std::function<void()> assign_fn,
                          std::function<void()> unassign_fn)
      : roo_windows::FlexLayout(env, roo_windows::FlexDirection::kColumn),
        model_(model),
        title_(env, model.ui()->labels.item_details_title),
        name_(env, "", roo_windows::material2::text_style_subtitle1(),
              roo_windows::kGravityCenter | roo_windows::kGravityMiddle),
        id_(env, "", roo_windows::material2::text_style_caption(),
            roo_windows::kGravityCenter | roo_windows::kGravityMiddle),
        reading_(model.ui()->widget_creator_fn()),
        //  roo_display::kCenter | roo_display::kMiddle),
        d1_(env),
        actions_(env, roo_windows::FlexDirection::kRow),
        button_unassign_(env, SCALED_ROO_ICON(filled, content_link_off),
                         model.ui()->labels.unassign),
        button_assign_(env, SCALED_ROO_ICON(filled, content_link),
                       model.ui()->labels.assign) {
    setAlignItems(roo_windows::AlignItems::kCenter);
    setGap(roo_windows::Scaled(8));
    // edit_.setOnInteractiveChange(edit_fn);
    button_assign_.setOnInteractiveChange(assign_fn);
    button_unassign_.setOnInteractiveChange(unassign_fn);
    // title_.add(edit_, roo_windows::HorizontalLayout::Params());
    add(title_, {.flex_grow = 0,
                 .flex_shrink = 0,
                 .align_self = roo_windows::AlignSelf::kStretch});
    // indicator_.setPadding(roo_windows::PaddingSize::kTiny);
    // add(indicator_, VerticalLayout::Params());
    name_.setPadding(roo_windows::PaddingSize::kNone);
    name_.setMargins(roo_windows::MarginSize::kNone);
    // id_.setPadding(roo_windows::PaddingSize::kNone);
    // id_.setMargins(roo_windows::MarginSize::kNone);
    add(name_);
    add(id_);
    add(*reading_);
    add(d1_, {.flex_grow = 1,
              .flex_shrink = 0,
              .align_self = roo_windows::AlignSelf::kStretch});
    // indicator_.setConnectionStatus(roo_windows::WifiIndicator::DISCONNECTED);
    actions_.setGap(roo_windows::Scaled(8));
    button_unassign_.setPadding(roo_windows::PaddingSize::kLarge,
                                roo_windows::PaddingSize::kSmall);
    button_assign_.setPadding(roo_windows::PaddingSize::kLarge,
                              roo_windows::PaddingSize::kSmall);
    roo_display::Color pri = env.theme().material3Theme().color.primary;
    button_unassign_.setColor(pri);
    button_assign_.setColor(pri);
    actions_.add(button_unassign_,
                 {.flex_grow = 1,
                  .flex_shrink = 1,
                  .flex_basis = roo_windows::FlexBasis::kZero});
    actions_.add(button_assign_, {.flex_grow = 1,
                                  .flex_shrink = 1,
                                  .flex_basis = roo_windows::FlexBasis::kZero});

    add(actions_, {.flex_grow = 0,
                   .flex_shrink = 0,
                   .align_self = roo_windows::AlignSelf::kStretch});
  }

  roo_windows::PreferredSize getPreferredSize() const override {
    return roo_windows::PreferredSize(
        roo_windows::PreferredSize::MatchParentWidth(),
        roo_windows::PreferredSize::WrapContentHeight());
  }

  void enter(int idx) {
    idx_ = idx;
    name_.setText(model_.getBindingLabel(idx_));
    if (!model_.isBound(idx_)) {
      id_.setText(model_.ui()->labels.unassigned);
    } else {
      id_.setText(model_.getBindingItemId(idx_));
    }
  }

  void onDetailsChanged(bool has_id) {
    button_unassign_.setEnabled(has_id);
    button_assign_.setEnabled(!has_id);
  }

  void updateReading() {
    model_.ui()->widget_setter_fn(model_.getBindingItemId(idx_), *reading_);
  }

 private:
  void measurementsChanged() override { updateReading(); }

  Model& model_;
  int idx_;
  roo_windows::menu::Title title_;
  roo_windows::TextLabel name_;
  roo_windows::TextLabel id_;
  std::unique_ptr<Widget> reading_;
  roo_windows::HorizontalDivider d1_;
  roo_windows::FlexLayout actions_;
  roo_windows::IconWithCaption button_unassign_;
  roo_windows::IconWithCaption button_assign_;
};

class DetailsActivity : public roo_windows::Destination {
 public:
  DetailsActivity(roo_windows::ApplicationContext& env, Model& model,
                  SelectFn assign_fn, SelectFn unassign_fn)
      : roo_windows::Destination(),
        idx_(),
        model_(model),
        contents_(
            env, model,
            [this, assign_fn]() { assign_fn(*getContents().getTask(), idx_); },
            [this, unassign_fn]() {
              unassign_fn(*getContents().getTask(), idx_);
            }),
        scrollable_container_(env, contents_) {}

  roo_windows::Widget& getContents() override { return scrollable_container_; }

  void enter(roo_windows::NavigationHost& navigation, int idx) {
    idx_ = idx;
    navigation.push(*this);
  }

  void onResume() override {
    contents_.enter(idx_);
    contents_.onDetailsChanged(model_.isBound(idx_));
    contents_.updateReading();
  }

  void onStart() override { model_.addEventListener(&contents_); }

  void onStop() override {
    model_.removeEventListener(&contents_);
    idx_ = -1;
  }

 private:
  int idx_;
  Model& model_;
  DetailsActivityContents contents_;
  roo_windows::ScrollableBlitPanel scrollable_container_;
};

}  // namespace roo_windows_transceivers
