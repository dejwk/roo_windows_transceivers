cc_library(
    name = "roo_windows_onewire",
    visibility = ["//visibility:public"],
    srcs = glob(
        [
            "src/**/*.cpp",
            "src/**/*.h",
        ],
        exclude = ["test/**"],
    ),
    includes = [
        "src",
    ],
    defines = [
        "ROO_TESTING",
        "ARDUINO=10805",
    ],
    deps = [
        "//lib/roo_onewire",
        "//lib/roo_windows",
        "//lib/roo_control",
    ],
)
