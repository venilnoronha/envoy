licenses(["notice"])  # Apache 2

load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_package",
)

envoy_package()

envoy_cc_library(
    name = "request_info_interface",
    hdrs = ["request_info.h"],
    external_deps = ["abseil_optional"],
    deps = [
        "//include/envoy/http:protocol_interface",
        "//include/envoy/stream_info:stream_info_interface",
    ],
)
