licenses(["notice"])  # Apache 2

load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_package",
)

envoy_package()

envoy_cc_library(
    name = "request_info_lib",
    hdrs = ["request_info_impl.h"],
    deps = [
        "//include/envoy/stream_info:filter_state_interface",
        "//source/common/common:assert_lib",
        "//source/common/stream_info:stream_info_lib",
    ],
)
