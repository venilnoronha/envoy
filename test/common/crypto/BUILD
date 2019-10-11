licenses(["notice"])  # Apache 2

load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_test",
    "envoy_package",
)

envoy_package()

envoy_extension_cc_test(
    name = "utility_test",
    srcs = [
        "utility_test.cc",
    ],
    extension_name = "envoy.extensions.common.crypto.utility_lib",
    external_deps = [
        "ssl",
    ],
    deps = [
        "//bazel:crypto_utility_lib",
        "//source/common/buffer:buffer_lib",
        "//source/common/common:hex_lib",
        "//source/common/crypto:utility_lib",
    ],
)
