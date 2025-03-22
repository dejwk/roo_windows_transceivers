#pragma once

#include <Arduino.h>

#include "roo_transceivers/binding/binding.h"
#include "roo_transceivers/notification.h"
#include "roo_windows/dialogs/string_constants.h"
#include "roo_windows/locale/languages.h"
#include "roo_windows_onewire/model.h"

namespace roo_windows_onewire {

template <typename Binding>
struct ModelItem {
  Binding& binding;
  roo::string_view label;
};

template <typename Item, typename Binding>
class SimpleSelectorModel : public Model {
 public:
  SimpleSelectorModel(const roo_windows::Environment* env,
                      std::vector<ModelItem<Binding>> bindings)
      : bindings_(std::move(bindings)) {}

  SimpleSelectorModel(const SimpleSelectorModel&) = delete;
  SimpleSelectorModel(SimpleSelectorModel&&) = delete;

  SimpleSelectorModel& operator=(const SimpleSelectorModel& m) = delete;

  size_t getBindingCount() const override { return bindings_.size(); }

  roo_io::string_view getBindingLabel(size_t idx) const override {
    return bindings_[idx].label;
  }

  roo_io::string_view getBindingItemId(size_t idx) const override {
    initBindings();
    return binding_ids_[idx];
  }

  void bind(size_t idx, roo_io::string_view item_id) override {
    initBindings();
    bindings_[idx].binding.bind(item_id_mapping_[item_id]);
    binding_ids_[idx] = std::string(item_id.data(), item_id.size());
    notifyItemsChanged();
  }

  void unbind(size_t idx) override {
    initBindings();
    bindings_[idx].binding.unbind();
    binding_ids_[idx] = "";
    notifyItemsChanged();
  }

  bool isBound(size_t idx) const override {
    return bindings_[idx].binding.isBound();
  }

  size_t getItemCount() const override { return all_items_.size(); }

  roo_io::string_view getItemId(size_t idx) const override {
    return all_item_ids_[idx];
  }

 protected:
  virtual std::string itemToString(const Item& item) const = 0;

  void clear() {
    all_items_.clear();
    all_item_ids_.clear();
    item_id_mapping_.clear();
  }

  void addItem(const Item& item) {
    all_item_ids_.push_back(itemToString(item));
    all_items_.push_back(item);
    item_id_mapping_[all_item_ids_.back()] = all_items_.back();
  }

  const Item* lookupItem(roo_io::string_view item_id) const {
    auto itr = item_id_mapping_.find(item_id);
    if (itr == item_id_mapping_.end()) {
      return nullptr;
    }
    return &itr->second;
  }

 private:
  void initBindings() const {
    if (binding_ids_.size() == bindings_.size()) {
      // Already initialized.
      return;
    }
    for (size_t i = 0; i < bindings_.size(); ++i) {
      binding_ids_.push_back(itemToString(bindings_[i].binding.get()));
    }
  }

  std::vector<ModelItem<Binding>> bindings_;

  std::vector<Item> all_items_;
  std::vector<std::string> all_item_ids_;
  mutable std::vector<std::string> binding_ids_;

  roo_collections::FlatSmallHashMap<roo_io::string_view, Item> item_id_mapping_;
};

template <typename Item, typename Binding>
class SimpleTransceiverSelectorModel
    : public SimpleSelectorModel<Item, Binding>,
      public roo_transceivers::EventListener {
 public:
  using Base = SimpleSelectorModel<roo_transceivers::SensorLocator,
                                   roo_transceivers::SensorBinding>;

  SimpleTransceiverSelectorModel(
      const roo_windows::Environment* env,
      roo_transceivers::Universe& transceivers,
      std::vector<ModelItem<roo_transceivers::SensorBinding>> bindings)
      : Base(env, std::move(bindings)), transceivers_(transceivers) {
    transceivers_.addEventListener(this);
    updateSensors();
  }

  ~SimpleTransceiverSelectorModel() { transceivers_.removeEventListener(this); }

  void requestUpdate() override { transceivers_.requestUpdate(); }

  void devicesChanged() override {
    updateSensors();
    Base::notifyItemsChanged();
  }

  void newReadingsAvailable() override { Base::notifyMeasurementsChanged(); }

 protected:
  virtual void maybeAddTransceiver(
      const roo_transceivers::DeviceLocator& device,
      const roo_transceivers_Descriptor& descriptor) = 0;

  void updateSensors() {
    Base::clear();
    roo_transceivers_Descriptor descriptor;
    transceivers_.forEachDevice(
        [&](const roo_transceivers::DeviceLocator& device_loc) {
          if (!transceivers_.getDeviceDescriptor(device_loc, descriptor)) {
            LOG(ERROR) << "No descriptor for " << device_loc;
            return true;
          }
          maybeAddTransceiver(device_loc, descriptor);
          return true;
        });
  }

  roo_transceivers::Universe& transceivers_;
};

}  // namespace roo_windows_onewire
