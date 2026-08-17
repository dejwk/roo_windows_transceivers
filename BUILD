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
        "@roo_transceivers",
        "@roo_windows",
    ],
)
