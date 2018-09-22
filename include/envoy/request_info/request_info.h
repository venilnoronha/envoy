#pragma once

#include <chrono>
#include <cstdint>
#include <string>

#include "envoy/common/pure.h"
#include "envoy/http/protocol.h"
#include "envoy/stream_info/stream_info.h"

#include "absl/types/optional.h"

namespace Envoy {

namespace RequestInfo {

/**
 * Additional information about a completed request for logging.
 */
class RequestInfo : public StreamInfo::StreamInfo {
public:
  virtual ~RequestInfo() {}

  /**
   * @return the protocol of the request.
   */
  virtual absl::optional<Http::Protocol> protocol() const PURE;

  /**
   * @param protocol the request's protocol.
   */
  virtual void protocol(Http::Protocol protocol) PURE;

  /**
   * @return the response code.
   */
  virtual absl::optional<uint32_t> responseCode() const PURE;
};

} // namespace RequestInfo
} // namespace Envoy
