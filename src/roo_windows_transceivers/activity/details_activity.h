#pragma once

#include <memory>

#include "roo_icons.h"
#include "roo_icons/filled/action.h"
#include "roo_icons/filled/content.h"
#include "roo_icons/filled/notification.h"
#include "roo_windows.h"
#include "roo_windows/composites/menu/title.h"
#include "roo_windows/config.h"
#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/containers/stacked_layout.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/activity.h"
#include "roo_windows/core/task.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/widgets/icon_with_caption.h"
#include "roo_windows/widgets/text_field.h"
#include "roo_windows_transceivers/model.h"

namespace roo_windows_transceivers {

typedef std::function<void(roo_windows::Task& task, int id)> SelectFn;

class DetailsActivityContents : public roo_windows::VerticalLayout,
                                public Model::EventListener {
 public:
  DetailsActivityContents(const roo_windows::Environment& env, Model& model,
                          std::function<void()> assign_fn,
                          std::function<void()> unassign_fn)
      : roo_windows::VerticalLayout(env),
        model_(model),
        title_(env, model.ui()->labels.item_details_title),
        name_(env, "", roo_windows::font_subtitle1(),
              roo_windows::kGravityCenter | roo_windows::kGravityMiddle),
        id_(env, "", roo_windows::font_caption(),
            roo_windows::kGravityCenter | roo_windows::kGravityMiddle),
        reading_(model.ui()->widget_creator_fn()),
        //  roo_display::kCenter | roo_display::kMiddle),
        d1_(env),
        actions_(env),
        button_unassign_(env, SCALED_ROO_ICON(filled, content_link_off),
                         model.ui()->labels.unassign),
        button_assign_(env, SCALED_ROO_ICON(filled, content_link),
                       model.ui()->labels.assign) {
    setGravity(roo_windows::kGravityMiddle);
    // edit_.setOnInteractiveChange(edit_fn);
    button_assign_.setOnInteractiveChange(assign_fn);
    button_unassign_.setOnInteractiveChange(unassign_fn);
    // title_.add(edit_, roo_windows::HorizontalLayout::Params());
    add(title_, {gravity : roo_windows::kGravityLeft});
    // indicator_.setPadding(roo_windows::PaddingSize::TINY);
    // add(indicator_, VerticalLayout::Params());
    name_.setPadding(roo_windows::PaddingSize::NONE);
    name_.setMargins(roo_windows::MarginSize::NONE);
    // id_.setPadding(roo_windows::PaddingSize::NONE);
    // id_.setMargins(roo_windows::MarginSize::NONE);
    add(name_);
    add(id_);
    add(*reading_);
    add(d1_, {weight : 1});
    // indicator_.setConnectionStatus(roo_windows::WifiIndicator::DISCONNECTED);
    actions_.setUseLargestChild(true);
    button_unassign_.setPadding(roo_windows::PaddingSize::LARGE,
                                roo_windows::PaddingSize::SMALL);
    button_assign_.setPadding(roo_windows::PaddingSize::LARGE,
                              roo_windows::PaddingSize::SMALL);
    roo_display::Color pri = env.theme().color.primary;
    button_unassign_.setColor(pri);
    button_assign_.setColor(pri);
    actions_.add(button_unassign_, {weight : 1});
    actions_.add(button_assign_, {weight : 1});

    add(actions_, VerticalLayout::Params());
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
  roo_windows::HorizontalLayout actions_;
  roo_windows::IconWithCaption button_unassign_;
  roo_windows::IconWithCaption button_assign_;
};

class DetailsActivity : public roo_windows::Activity {
 public:
  DetailsActivity(const roo_windows::Environment& env, Model& model,
                  SelectFn assign_fn, SelectFn unassign_fn)
      : roo_windows::Activity(),
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

  void enter(roo_windows::Task& task, int idx) {
    idx_ = idx;
    task.enterActivity(this);
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
  roo_windows::ScrollablePanel scrollable_container_;
};

}  // namespace roo_windows_transceivers
