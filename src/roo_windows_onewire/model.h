#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "roo_collections/flat_small_hash_set.h"
#include "roo_windows/core/widget.h"
#include "roo_windows/widgets/text_label.h"

namespace roo_windows_onewire {

struct Ui {
  roo_windows::WidgetCreatorFn widget_creator_fn;
  roo_windows::WidgetSetterFn<roo_io::string_view> widget_setter_fn;
  const roo_display::Pictogram* icon;
  struct Labels {
    const char* list_title;
    const char* item_details_title;
    const char* assign;
    const char* unassign;
    const char* unassign_question;
    const char* unassign_question_supporting_text;
    const char* unassigned;
    const char* assign_from_list;
  } labels;
};

class Model {
 public:
  class EventListener {
   public:
    virtual ~EventListener() = default;

    virtual void itemsChanged() {}
    virtual void measurementsChanged() {}
  };

  virtual ~Model() = default;

  virtual void requestUpdate() = 0;

  virtual size_t getItemCount() const = 0;
  virtual roo_io::string_view getItemId(size_t idx) const = 0;

  virtual size_t getBindingCount() const = 0;
  virtual roo_io::string_view getBindingLabel(size_t idx) const = 0;
  virtual roo_io::string_view getBindingItemId(size_t idx) const = 0;
  virtual bool isBound(size_t idx) const = 0;

  virtual void bind(size_t idx, roo_io::string_view item_id) = 0;
  virtual void unbind(size_t idx) = 0;

  virtual const Ui* ui() const = 0;

  void addEventListener(EventListener* listener) {
    auto result = event_listeners_.insert(listener);
    CHECK(result.second) << "Event listener " << listener
                         << " was registered already.";
  }

  void removeEventListener(EventListener* listener) {
    event_listeners_.erase(listener);
  }

  size_t getUnassignedItemCount() const { return unassigned_items_.size(); }

  roo_io::string_view getUnassignedItemId(size_t idx) const {
    return getItemId(unassigned_items_[idx]);
  }

 protected:
  void notifyItemsChanged() {
    updateUnassignedItems();
    for (auto& listener : event_listeners_) {
      listener->itemsChanged();
    }
  }

  void notifyMeasurementsChanged() {
    for (auto& listener : event_listeners_) {
      listener->measurementsChanged();
    }
  }

 protected:
  // Indexes into the items vector.
  std::vector<size_t> unassigned_items_;

 private:
  void updateUnassignedItems() {
    const size_t binding_count = getBindingCount();
    roo_collections::FlatSmallHashSet<size_t> binding_set(binding_count);

    unassigned_items_.clear();
    for (size_t i = 0; i < binding_count; ++i) {
      if (isBound(i)) {
        binding_set.insert(i);
      }
    }

    const size_t item_count = getItemCount();
    for (size_t i = 0; i < item_count; ++i) {
      if (!binding_set.contains(i)) {
        unassigned_items_.push_back(i);
      }
    }
  }

  roo_collections::FlatSmallHashSet<EventListener*> event_listeners_;
};

}  // namespace roo_windows_onewire
