package(default_visibility = ["//visibility:public"])

load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_proto_library",
)

envoy_cc_library(
    name = "dns_config",
    srcs = ["dns_config.cc"],
    hdrs = ["dns_config.h"],
    deps = [
        "@envoy_api//envoy/config/filter/listener/dns:dns_cc",
        "//source/common/protobuf:utility_lib",
    ],
)

envoy_cc_library(
    name = "dns_config_factory",
    srcs = ["dns_config_factory.cc"],
    hdrs = ["dns_config_factory.h"],
    deps = [
        ":dns_config",
        ":dns_filter",
        "//include/envoy/network:filter_interface",
        "//include/envoy/registry",
        "//include/envoy/server:filter_config_interface",
        "//source/server:active_raw_udp_listener_config",
    ],
)

envoy_cc_library(
    name = "dns_server",
    hdrs = ["dns_server.h"],
    deps = [
        ":dns_codec",
        ":dns_config",
        "//include/envoy/network:address_interface",
    ],
)

envoy_cc_library(
    name = "dns_server_impl",
    srcs = ["dns_server_impl.cc"],
    hdrs = ["dns_server_impl.h"],
    deps = [
        ":dns_codec_impl",
        ":dns_server",
        "//include/envoy/upstream:cluster_manager_interface",
        "//include/envoy/upstream:thread_local_cluster_interface",
        "//include/envoy/upstream:upstream_interface",
        "//source/common/common:assert_lib",
        "//source/common/common:minimal_logger_lib",
        "//source/common/network:dns_lib",
    ],
)

envoy_cc_library(
    name = "dns_filter",
    srcs = ["dns_filter.cc"],
    hdrs = ["dns_filter.h"],
    deps = [
        ":dns_config",
        ":dns_server_impl",
        "//include/envoy/network:address_interface",
        "//include/envoy/network:connection_interface",
        "//include/envoy/network:listener_interface",
        "//source/common/common:assert_lib",
        "//source/common/common:minimal_logger_lib",
    ],
)

envoy_cc_library(
    name = "dns_codec",
    hdrs = ["dns_codec.h"],
    deps = [
        "//include/envoy/buffer:buffer_interface",
        "//include/envoy/network:address_interface",
    ],
)

envoy_cc_library(
    name = "dns_codec_impl",
    srcs = ["dns_codec_impl.cc"],
    hdrs = ["dns_codec_impl.h"],
    external_deps = ["ares"],
    deps = [
        ":dns_codec",
        "//source/common/buffer:buffer_lib",
        "//source/common/common:assert_lib",
        "//source/common/common:minimal_logger_lib",
    ],
)
