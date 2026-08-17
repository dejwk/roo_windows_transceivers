load("@roo_testing//roo_testing/emulation:arduino.bzl", "roo_arduino_example")

roo_arduino_example(
    name = "roles",
    defines = ["ROO_LOGGING_COLORLOGTOSTDERR=1"],
    sketch = "roles.ino",
    visibility = ["//visibility:public"],
    deps = [
        "//:roo_windows_transceivers",
        "@roo_display",
        "@roo_onewire",
        "@roo_scheduler",
        "@roo_testing//roo_testing/devices/display/ili9341:spi",
        "@roo_testing//roo_testing/devices/touch/xpt2046:spi",
        "@roo_testing//roo_testing/microcontrollers/esp32",
        "@roo_testing//roo_testing/transducers/ui/viewport:flex",
        "@roo_testing//roo_testing/transducers/ui/viewport/fltk",
        "@roo_time",
        "@roo_windows",
    ],
)
