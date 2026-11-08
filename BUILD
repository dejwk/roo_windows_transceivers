load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "roo_windows_transceivers",
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
    visibility = ["//visibility:public"],
    deps = [
        "@roo_pb",
        "@roo_transceivers",
        "@roo_windows",
    ],
)

load("@rules_cc//cc:cc_test.bzl", "cc_test")

cc_library(
    name = "resources_pl",
    srcs = [
        "src/roo_windows_transceivers/model/switches/resources.cpp",
        "src/roo_windows_transceivers/model/thermometers/resources.cpp",
    ],
    hdrs = [
        "src/roo_windows_transceivers/model/switches/resources.h",
        "src/roo_windows_transceivers/model/thermometers/resources.h",
    ],
    includes = ["src"],
    local_defines = ["ROO_LANG=ROO_LANG_pl"],
    deps = ["@roo_windows"],
)

cc_test(
    name = "resources_pl_test",
    srcs = ["test/resources_pl_test.cpp"],
    deps = [":resources_pl"],
)
