#pragma once

#include <chrono>
#include <cstdint>

#include "envoy/common/time.h"
#include "envoy/request_info/request_info.h"

#include "common/common/assert.h"
#include "common/stream_info/stream_info_impl.h"

namespace Envoy {
namespace RequestInfo {

struct RequestInfoImpl : public RequestInfo, Envoy::StreamInfo::StreamInfoImpl {
  explicit RequestInfoImpl(TimeSource& time_source) : RequestInfo(), StreamInfoImpl(time_source) {}

  RequestInfoImpl(Http::Protocol protocol, TimeSource& time_source) : RequestInfoImpl(time_source) {
    protocol_ = protocol;
  }

  SystemTime startTime() const override { return StreamInfoImpl::startTime(); }

  MonotonicTime startTimeMonotonic() const override { return StreamInfoImpl::startTimeMonotonic(); }

  absl::optional<std::chrono::nanoseconds> lastDownstreamRxByteReceived() const override {
    return StreamInfoImpl::lastDownstreamRxByteReceived();
  }

  void onLastDownstreamRxByteReceived() override {
    StreamInfoImpl::onLastDownstreamRxByteReceived();
  }

  absl::optional<std::chrono::nanoseconds> firstUpstreamTxByteSent() const override {
    return firstUpstreamTxByteSent();
  }

  void onFirstUpstreamTxByteSent() override { StreamInfoImpl::onFirstUpstreamTxByteSent(); }

  absl::optional<std::chrono::nanoseconds> lastUpstreamTxByteSent() const override {
    return StreamInfoImpl::lastUpstreamTxByteSent();
  }

  void onLastUpstreamTxByteSent() override { StreamInfoImpl::onLastUpstreamTxByteSent(); }

  absl::optional<std::chrono::nanoseconds> firstUpstreamRxByteReceived() const override {
    return StreamInfoImpl::firstUpstreamRxByteReceived();
  }

  void onFirstUpstreamRxByteReceived() override { StreamInfoImpl::onFirstUpstreamRxByteReceived(); }

  absl::optional<std::chrono::nanoseconds> lastUpstreamRxByteReceived() const override {
    return StreamInfoImpl::lastUpstreamRxByteReceived();
  }

  void onLastUpstreamRxByteReceived() override { StreamInfoImpl::onLastUpstreamRxByteReceived(); }

  absl::optional<std::chrono::nanoseconds> firstDownstreamTxByteSent() const override {
    return StreamInfoImpl::firstDownstreamTxByteSent();
  }

  void onFirstDownstreamTxByteSent() override {
    return StreamInfoImpl::onFirstDownstreamTxByteSent();
  }

  absl::optional<std::chrono::nanoseconds> lastDownstreamTxByteSent() const override {
    return StreamInfoImpl::lastDownstreamTxByteSent();
  }

  void onLastDownstreamTxByteSent() override { StreamInfoImpl::onLastDownstreamTxByteSent(); }

  absl::optional<std::chrono::nanoseconds> requestComplete() const override {
    return StreamInfoImpl::requestComplete();
  }

  void onRequestComplete() override { StreamInfoImpl::onRequestComplete(); }

  void resetUpstreamTimings() override { StreamInfoImpl::resetUpstreamTimings(); }

  void addBytesReceived(uint64_t bytes_received) override {
    StreamInfoImpl::addBytesReceived(bytes_received);
  }

  uint64_t bytesReceived() const override { return StreamInfoImpl::bytesReceived(); }

  absl::optional<Http::Protocol> protocol() const override { return protocol_; }

  void protocol(Http::Protocol protocol) override { protocol_ = protocol; }

  absl::optional<uint32_t> responseCode() const override { return response_code_; }

  void addBytesSent(uint64_t bytes_sent) override { StreamInfoImpl::addBytesSent(bytes_sent); }

  uint64_t bytesSent() const override { return StreamInfoImpl::bytesSent(); }

  void setResponseFlag(Envoy::StreamInfo::ResponseFlag response_flag) override {
    StreamInfoImpl::setResponseFlag(response_flag);
  }

  bool intersectResponseFlags(uint64_t response_flags) const override {
    return StreamInfoImpl::intersectResponseFlags(response_flags);
  }

  bool hasResponseFlag(Envoy::StreamInfo::ResponseFlag flag) const override {
    return StreamInfoImpl::hasResponseFlag(flag);
  }

  bool hasAnyResponseFlag() const override { return StreamInfoImpl::hasAnyResponseFlag(); }

  void onUpstreamHostSelected(Upstream::HostDescriptionConstSharedPtr host) override {
    StreamInfoImpl::onUpstreamHostSelected(host);
  }

  Upstream::HostDescriptionConstSharedPtr upstreamHost() const override {
    return StreamInfoImpl::upstreamHost();
  }

  void setUpstreamLocalAddress(
      const Network::Address::InstanceConstSharedPtr& upstream_local_address) override {
    StreamInfoImpl::setUpstreamLocalAddress(upstream_local_address);
  }

  const Network::Address::InstanceConstSharedPtr& upstreamLocalAddress() const override {
    return StreamInfoImpl::upstreamLocalAddress();
  }

  bool healthCheck() const override { return StreamInfoImpl::healthCheck(); }

  void healthCheck(bool is_hc) override { StreamInfoImpl::healthCheck(is_hc); }

  void setDownstreamLocalAddress(
      const Network::Address::InstanceConstSharedPtr& downstream_local_address) override {
    StreamInfoImpl::setDownstreamLocalAddress(downstream_local_address);
  }

  const Network::Address::InstanceConstSharedPtr& downstreamLocalAddress() const override {
    return StreamInfoImpl::downstreamLocalAddress();
  }

  void setDownstreamRemoteAddress(
      const Network::Address::InstanceConstSharedPtr& downstream_remote_address) override {
    StreamInfoImpl::setDownstreamRemoteAddress(downstream_remote_address);
  }

  const Network::Address::InstanceConstSharedPtr& downstreamRemoteAddress() const override {
    return StreamInfoImpl::downstreamRemoteAddress();
  }

  const Router::RouteEntry* routeEntry() const override { return StreamInfoImpl::routeEntry(); }

  const envoy::api::v2::core::Metadata& dynamicMetadata() const override {
    return StreamInfoImpl::dynamicMetadata();
  };

  void setDynamicMetadata(const std::string& name, const ProtobufWkt::Struct& value) override {
    StreamInfoImpl::setDynamicMetadata(name, value);
  };

  Envoy::StreamInfo::FilterState& perRequestState() override {
    return StreamInfoImpl::perRequestState();
  }
  const Envoy::StreamInfo::FilterState& perRequestState() const override {
    return StreamInfoImpl::perRequestState();
  }

  void setRequestedServerName(absl::string_view requested_server_name) override {
    StreamInfoImpl::setRequestedServerName(requested_server_name);
  }

  const std::string& requestedServerName() const override {
    return StreamInfoImpl::requestedServerName();
  }

  absl::optional<Http::Protocol> protocol_;
  absl::optional<uint32_t> response_code_;
};

} // namespace RequestInfo
} // namespace Envoy
