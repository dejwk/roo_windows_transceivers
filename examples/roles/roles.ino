#include <Arduino.h>
#include <SPI.h>

#include "roo_display.h"
#include "roo_display/driver/ili9341.h"
#include "roo_display/driver/touch_xpt2046.h"
#include "roo_onewire.h"
#include "roo_scheduler.h"
#include "roo_time.h"
#include "roo_windows.h"
#include "roo_windows/composites/menu/basic_navigation_item.h"
#include "roo_windows/composites/menu/menu.h"
#include "roo_windows/containers/aligned_layout.h"
#include "roo_windows_transceivers.h"

using namespace roo_display;
using namespace roo_onewire;
using namespace roo_scheduler;
using namespace roo_windows;
using namespace roo_time;

// Set your configuration for the driver.
static constexpr int kCsPin = 5;
static constexpr int kDcPin = 17;
static constexpr int kRstPin = 27;
static constexpr int kBlPin = 16;

static constexpr int kTouchCsPin = 2;

static constexpr int kOneWirePin = 25;

Ili9341spi<kCsPin, kDcPin, kRstPin> screen(Orientation().rotateLeft());
TouchXpt2046<kTouchCsPin> touch;

Display display(screen, touch,
                TouchCalibration(269, 249, 3829, 3684,
                                 Orientation::LeftDown()));

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);

roo_windows::Application app(&env, display);

roo_onewire::OneWire onewire(kOneWirePin, scheduler);

enum Role { KITCHEN, BEDROOM };
std::vector<ThermometerRoles::Spec> roles = {{KITCHEN, "Kitchen"},
                                             {BEDROOM, "Bedroom"}};

ThermometerRoles thermometer_roles(onewire, roles);

auto& kitchen = thermometer_roles.thermometer_role(0);
auto& bedroom = thermometer_roles.thermometer_role(1);

roo_windows_transceivers::Configurator onewire_setup(app.context(),
                                                     thermometer_roles);
NavigationHost navigation;

class SettingsMenu : public menu::Menu {
 public:
  SettingsMenu(ApplicationContext& context, NavigationHost& navigation)
      : menu::Menu(context, "Settings"),
        onewire_item_(context, SCALED_ROO_ICON(filled, content_link),
                      "Thermometers", navigation, onewire_setup.main()) {
    add(onewire_item_);
  }

 private:
  menu::BasicNavigationItem onewire_item_;
};

SettingsMenu settings_menu(app.context(), navigation);

// Fetch temperatures every second.
RepetitiveTask converter(scheduler, []() { onewire.update(); }, Seconds(1));

// Report temperatures every 5 seconds.
RepetitiveTask reporter(
    scheduler,
    []() {
      LOG(INFO) << kitchen;
      LOG(INFO) << bedroom;
    },
    Seconds(5));

void setup() {
  SPI.begin();

  display.init();
  converter.startInstantly();
  reporter.start();
  app.addTaskFullScreen(navigation);
  navigation.push(settings_menu);
  app.start();
  scheduler.run();
}

void loop() {}
