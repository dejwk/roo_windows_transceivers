#include <Arduino.h>
#include <SPI.h>

#include "roo_display.h"
#include "roo_display/driver/ili9341.h"
#include "roo_display/driver/touch_xpt2046.h"
#include "roo_onewire.h"
#include "roo_scheduler.h"
#include "roo_time.h"
#include "roo_transceivers/binding/binding.h"
#include "roo_transceivers/binding/hal/defaults.h"
#include "roo_transceivers/onewire/onewire.h"
#include "roo_windows.h"
#include "roo_windows/composites/menu/basic_navigation_item.h"
#include "roo_windows/composites/menu/menu.h"
#include "roo_windows/containers/aligned_layout.h"
#include "roo_windows_transceivers.h"
#include "roo_windows_transceivers/model/thermometers/thermometer_selector_model.h"

#ifdef ROO_TESTING
#include "roo_testing/devices/display/ili9341/ili9341spi.h"
#include "roo_testing/devices/touch/xpt2046/xpt2046spi.h"
#include "roo_testing/microcontrollers/esp32/fake_esp32.h"
#include "roo_testing/transducers/ui/viewport/flex_viewport.h"
#include "roo_testing/transducers/ui/viewport/fltk/fltk_viewport.h"
#endif

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

#ifdef ROO_TESTING

using roo_testing_transducers::FlexViewport;
using roo_testing_transducers::FltkViewport;

struct Emulator {
  FltkViewport viewport;
  FlexViewport flex_viewport;
  FakeIli9341Spi display;
  FakeXpt2046Spi touch;
  FakeOneWireInterface onewire;

  Emulator()
      : viewport(), flex_viewport(viewport, 1, FlexViewport::kRotationRight),
        display(flex_viewport),
        touch(flex_viewport, FakeXpt2046Spi::Calibration(269, 249, 3829, 3684,
                                                         true, false, false)) {
    FakeEsp32().attachSpiDevice(display, 18, 19, 23);
    FakeEsp32().gpio.attachOutput(kCsPin, display.cs());
    FakeEsp32().gpio.attachOutput(kDcPin, display.dc());
    FakeEsp32().gpio.attachOutput(kRstPin, display.rst());
    FakeEsp32().attachSpiDevice(touch, 18, 19, 23);
    FakeEsp32().gpio.attachOutput(kTouchCsPin, touch.cs());
    FakeEsp32().attachOneWireBus(onewire, kOneWirePin);
  }
} emulator;

#endif

Ili9341spi<kCsPin, kDcPin, kRstPin> screen(Orientation().rotateLeft());
TouchXpt2046<kTouchCsPin> touch;

Display display(screen, touch,
                TouchCalibration(269, 249, 3829, 3684,
                                 Orientation::LeftDown()));

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);

roo_windows::Application app(&env, display);

roo_onewire::OneWire onewire(kOneWirePin, scheduler);
roo_transceivers::OneWireUniverse transceivers(onewire);

enum Role { KITCHEN, BEDROOM };
roo_transceivers::SensorBinding
    kitchen_binding(roo_transceivers::DefaultBindingStore(), KITCHEN);
roo_transceivers::SensorBinding
    bedroom_binding(roo_transceivers::DefaultBindingStore(), BEDROOM);
std::vector<
    roo_windows_transceivers::ModelItem<roo_transceivers::SensorBinding>>
    roles = {
        {kitchen_binding, "Kitchen"},
        {bedroom_binding, "Bedroom"},
};

roo_windows_transceivers::ThermometerSelectorModel
    thermometer_roles(&app.context(), transceivers, roles);

roo_transceivers::BoundSensor kitchen(transceivers, &kitchen_binding);
roo_transceivers::BoundSensor bedroom(transceivers, &bedroom_binding);

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
