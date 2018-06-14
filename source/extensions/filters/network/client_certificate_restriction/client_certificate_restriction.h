#pragma once

#include <openssl/x509v3.h>

#include "envoy/network/connection.h"
#include "envoy/network/filter.h"
#include "envoy/upstream/cluster_manager.h"

#include "common/common/logger.h"

#include "api/envoy/config/filter/network/client_certificate_restriction/v2/client_certificate_restriction.pb.validate.h"

namespace Envoy {
namespace Extensions {
namespace NetworkFilters {
namespace ClientCertificateRestriction {

/**
 * Global configuration for client certificate restriction.
 */
class ClientCertificateRestrictionConfig {
public:
  ClientCertificateRestrictionConfig(
      const envoy::config::filter::network::client_certificate_restriction::v2::
          ClientCertificateRestriction &config);

  const std::string &target() { return target_; }

private:
  std::string target_;
};

typedef std::shared_ptr<ClientCertificateRestrictionConfig>
    ClientCertificateRestrictionConfigSharedPtr;

/**
 * A client SSL certificate restriction filter instance. One per connection.
 */
class ClientCertificateRestrictionFilter
    : public Network::ReadFilter,
      public Network::ConnectionCallbacks,
      Logger::Loggable<Logger::Id::filter> {
public:
  ClientCertificateRestrictionFilter(
      ClientCertificateRestrictionConfigSharedPtr config,
      Upstream::ClusterManager &cm);

  // Network::ReadFilter
  Network::FilterStatus onData(Buffer::Instance &data,
                               bool end_stream) override;
  Network::FilterStatus onNewConnection() override;
  void initializeReadFilterCallbacks(
      Network::ReadFilterCallbacks &callbacks) override {
    read_callbacks_ = &callbacks;
    read_callbacks_->connection().addConnectionCallbacks(*this);
  }

  // Network::ConnectionCallbacks
  void onEvent(Network::ConnectionEvent event) override;
  void onAboveWriteBufferHighWatermark() override {}
  void onBelowWriteBufferLowWatermark() override {}

private:
  inline std::string getSerialNumber() const;

  // TODO(talnirdan): This code is duplicated from
  // Ssl::ContextImpl::getSerialNumber().
  static inline std::string getSerialNumber(const X509 *cert);

  static inline std::string getPayload(const std::string &target,
                                       const std::string &client_cert_uri,
                                       const std::string &client_cert_serial);

  ClientCertificateRestrictionConfigSharedPtr config_;
  Upstream::ClusterManager &cm_;
  Network::ReadFilterCallbacks *read_callbacks_{};
};

} // namespace ClientCertificateRestriction
} // namespace NetworkFilters
} // namespace Extensions
} // namespace Envoy
