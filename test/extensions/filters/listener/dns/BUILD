licenses(["notice"])  # MIT

load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_binary",
    "envoy_cc_mock",
    "envoy_cc_test",
)

envoy_cc_test(
    name = "dns_integration_test",
    srcs = ["dns_integration_test.cc"],
    deps = [
        "//source/extensions/filters/listener/dns:dns_config_factory",
        "//source/common/network:dns_lib",
        "//test/integration:integration_lib",
    ],
)

envoy_cc_test(
    name = "dns_server_impl_test",
    srcs = ["dns_server_impl_test.cc"],
    deps = [
        ":dns_filter_mocks",
        "//source/extensions/filters/listener/dns:dns_server_impl",
        "//test/mocks/event:event_mocks",
        "//test/mocks/network:network_mocks",
        "//test/mocks/upstream:upstream_mocks",
    ],
)

envoy_cc_binary(
    name = "envoy",
    deps = [
        "//source/extensions/filters/listener/dns:dns_filter",
        "//source/exe:envoy_main_entry_lib",
    ],
)

sh_test(
    name = "envoy_binary_test",
    srcs = ["envoy_binary_test.sh"],
    data = [":envoy"],
)

envoy_cc_mock(
    name = "dns_filter_mocks",
    srcs = ["mocks.cc"],
    hdrs = ["mocks.h"],
    deps = [
        "//source/extensions/filters/listener/dns:dns_codec",
        "//source/extensions/filters/listener/dns:dns_config",
    ],
)
