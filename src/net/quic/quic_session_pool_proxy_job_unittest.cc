// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "net/base/network_anonymization_key.h"
#include "net/base/proxy_chain.h"
#include "net/base/proxy_server.h"
#include "net/base/session_usage.h"
#include "net/cert/x509_certificate.h"
#include "net/quic/crypto/proof_verifier_chromium.h"
#include "net/quic/mock_quic_data.h"
#include "net/quic/quic_context.h"
#include "net/quic/quic_http_stream.h"
#include "net/quic/quic_session_pool.h"
#include "net/quic/quic_session_pool_test_base.h"
#include "net/quic/quic_test_packet_maker.h"
#include "net/test/cert_test_util.h"
#include "net/test/test_data_directory.h"
#include "net/third_party/quiche/src/quiche/quic/core/quic_types.h"
#include "net/third_party/quiche/src/quiche/quic/core/quic_versions.h"
#include "net/third_party/quiche/src/quiche/quic/test_tools/quic_test_utils.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace net::test {

class QuicSessionPoolProxyJobTest
    : public QuicSessionPoolTestBase,
      public ::testing::TestWithParam<quic::ParsedQuicVersion> {
 protected:
  QuicSessionPoolProxyJobTest() : QuicSessionPoolTestBase(GetParam()) {}
};

INSTANTIATE_TEST_SUITE_P(All,
                         QuicSessionPoolProxyJobTest,
                         ::testing::ValuesIn(AllSupportedQuicVersions()));

TEST_P(QuicSessionPoolProxyJobTest, CreateProxiedQuicSession) {
  Initialize();

  GURL url("https://www.example.org/");
  GURL proxy(kProxy1Url);
  auto origin = url::SchemeHostPort(url);
  auto proxy_origin = url::SchemeHostPort(proxy);
  auto nak = NetworkAnonymizationKey();

  scoped_refptr<X509Certificate> cert(
      ImportCertFromFile(GetTestCertsDirectory(), "wildcard.pem"));
  ASSERT_TRUE(cert->VerifyNameMatch(origin.host()));
  ASSERT_TRUE(cert->VerifyNameMatch(proxy_origin.host()));
  ASSERT_FALSE(cert->VerifyNameMatch(kDifferentHostname));

  ProofVerifyDetailsChromium verify_details;
  verify_details.cert_verify_result.verified_cert = cert;
  verify_details.cert_verify_result.is_issued_by_known_root = true;
  crypto_client_stream_factory_.AddProofVerifyDetails(&verify_details);

  // QUIC proxies do not use priority header.
  client_maker_.set_use_priority_header(false);

  // Use a separate packet maker for the connection to the endpoint.
  QuicTestPacketMaker endpoint_maker(
      version_,
      quic::QuicUtils::CreateRandomConnectionId(context_.random_generator()),
      context_.clock(), kDefaultServerHostName, quic::Perspective::IS_CLIENT,
      /*client_priority_uses_incremental=*/true,
      /*use_priority_header=*/true);

  const uint64_t stream_id = GetNthClientInitiatedBidirectionalStreamId(0);
  MockQuicData socket_data(version_);
  socket_data.AddWrite(SYNCHRONOUS, ConstructInitialSettingsPacket(1));
  socket_data.AddWrite(
      SYNCHRONOUS, ConstructConnectUdpRequestPacket(
                       2, stream_id, proxy.host(),
                       "/.well-known/masque/udp/www.example.org/443/", false));
  socket_data.AddRead(ASYNC, ConstructServerSettingsPacket(3));
  socket_data.AddRead(ASYNC, ConstructOkResponsePacket(4, stream_id, true));
  socket_data.AddReadPauseForever();
  socket_data.AddWrite(ASYNC, client_maker_.MakeAckPacket(3, 3, 4, 3));
  socket_data.AddWrite(ASYNC, ConstructClientH3DatagramPacket(
                                  4, stream_id, kConnectUdpContextId,
                                  endpoint_maker.MakeInitialSettingsPacket(1)));
  socket_data.AddSocketDataToFactory(socket_factory_.get());

  auto proxy_chain = ProxyChain::ForIpProtection({
      ProxyServer::FromSchemeHostAndPort(ProxyServer::SCHEME_QUIC,
                                         proxy_origin.host(), 443),
  });
  EXPECT_TRUE(proxy_chain.IsValid());

  RequestBuilder builder(this);
  builder.destination = origin;
  builder.proxy_chain = proxy_chain;
  builder.http_user_agent_settings = &http_user_agent_settings_;
  builder.url = url;

  // Note: `builder` defaults to using the parameterized `version_` member,
  // which we will assert here as a pre-condition for checking that the proxy
  // session ignores this and uses RFCv1 instead.
  ASSERT_EQ(builder.quic_version, version_);

  EXPECT_EQ(ERR_IO_PENDING, builder.CallRequest());
  ASSERT_EQ(OK, callback_.WaitForResult());
  std::unique_ptr<HttpStream> stream = CreateStream(&builder.request);
  EXPECT_TRUE(stream.get());
  QuicChromiumClientSession* session =
      GetActiveSession(origin, nak, proxy_chain);
  ASSERT_TRUE(session);

  // The direct connection to the proxy has a max packet size 1350. The
  // connection to the endpoint could use up to 1350 - (packet header = 38) -
  // (quarter-stream-id = 1) - (context-id = 1), but this value is greater than
  // the default maximum of 1250. We can only observe the largest datagram that
  // could be sent to the endpoint, which would be 1250 - (packet header = 38) =
  // 1212 bytes.
  EXPECT_EQ(session->GetGuaranteedLargestMessagePayload(), 1212);

  // Check that the session through the proxy uses the version from the request.
  EXPECT_EQ(session->GetQuicVersion(), version_);

  // Check that the session to the proxy always uses RFCv1.
  QuicChromiumClientSession* proxy_session = GetActiveSession(
      proxy_origin, nak, ProxyChain::ForIpProtection({}), SessionUsage::kProxy);
  ASSERT_TRUE(proxy_session);
  EXPECT_EQ(proxy_session->GetQuicVersion(), quic::ParsedQuicVersion::RFCv1());

  stream.reset();

  // Ensure the session finishes creating before proceeding.
  RunUntilIdle();

  socket_data.ExpectAllReadDataConsumed();
  socket_data.ExpectAllWriteDataConsumed();
}

TEST_P(QuicSessionPoolProxyJobTest, CreateProxySessionFails) {
  Initialize();

  GURL url("https://www.example.org/");
  GURL proxy(kProxy1Url);
  auto origin = url::SchemeHostPort(url);
  auto proxy_origin = url::SchemeHostPort(proxy);

  scoped_refptr<X509Certificate> cert(
      ImportCertFromFile(GetTestCertsDirectory(), "wildcard.pem"));
  ASSERT_TRUE(cert->VerifyNameMatch(origin.host()));
  ASSERT_TRUE(cert->VerifyNameMatch(proxy_origin.host()));
  ASSERT_FALSE(cert->VerifyNameMatch(kDifferentHostname));

  ProofVerifyDetailsChromium verify_details;
  verify_details.cert_verify_result.verified_cert = cert;
  verify_details.cert_verify_result.is_issued_by_known_root = true;
  crypto_client_stream_factory_.AddProofVerifyDetails(&verify_details);

  MockQuicData socket_data(version_);
  socket_data.AddReadPauseForever();
  // Creation of underlying session fails immediately.
  socket_data.AddWrite(SYNCHRONOUS, ERR_SOCKET_NOT_CONNECTED);
  socket_data.AddSocketDataToFactory(socket_factory_.get());

  auto proxy_chain = ProxyChain::ForIpProtection({
      ProxyServer::FromSchemeHostAndPort(ProxyServer::SCHEME_QUIC,
                                         proxy_origin.host(), 443),
  });
  EXPECT_TRUE(proxy_chain.IsValid());

  RequestBuilder builder(this);
  builder.destination = origin;
  builder.proxy_chain = proxy_chain;
  builder.http_user_agent_settings = &http_user_agent_settings_;
  builder.url = url;
  EXPECT_EQ(ERR_IO_PENDING, builder.CallRequest());
  ASSERT_EQ(ERR_QUIC_HANDSHAKE_FAILED, callback_.WaitForResult());

  socket_data.ExpectAllReadDataConsumed();
  socket_data.ExpectAllWriteDataConsumed();
}

TEST_P(QuicSessionPoolProxyJobTest, CreateSessionFails) {
  Initialize();

  GURL url("https://www.example.org/");
  GURL proxy(kProxy1Url);
  auto origin = url::SchemeHostPort(url);
  auto proxy_origin = url::SchemeHostPort(proxy);

  scoped_refptr<X509Certificate> cert(
      ImportCertFromFile(GetTestCertsDirectory(), "wildcard.pem"));
  ASSERT_TRUE(cert->VerifyNameMatch(origin.host()));
  ASSERT_TRUE(cert->VerifyNameMatch(proxy_origin.host()));
  ASSERT_FALSE(cert->VerifyNameMatch(kDifferentHostname));

  ProofVerifyDetailsChromium verify_details;
  verify_details.cert_verify_result.verified_cert = cert;
  verify_details.cert_verify_result.is_issued_by_known_root = true;
  crypto_client_stream_factory_.AddProofVerifyDetails(&verify_details);

  // QUIC proxies do not use priority header.
  client_maker_.set_use_priority_header(false);

  MockQuicData socket_data(version_);
  socket_data.AddReadPauseForever();  // SYNC/ERR_IO_PENDING
  socket_data.AddWritePause();
  socket_data.AddWrite(ASYNC, ConstructInitialSettingsPacket(1));
  socket_data.AddSocketDataToFactory(socket_factory_.get());

  auto proxy_chain = ProxyChain::ForIpProtection({
      ProxyServer::FromSchemeHostAndPort(ProxyServer::SCHEME_QUIC,
                                         proxy_origin.host(), 443),
  });
  EXPECT_TRUE(proxy_chain.IsValid());

  RequestBuilder builder(this);
  builder.destination = origin;
  builder.proxy_chain = proxy_chain;
  builder.http_user_agent_settings = &http_user_agent_settings_;
  builder.url = url;
  EXPECT_EQ(ERR_IO_PENDING, builder.CallRequest());

  RunUntilIdle();

  // Oops, the session went away. This generates an error
  // from `QuicSessionPool::CreateSessionOnProxyStream`.
  factory_->CloseAllSessions(ERR_QUIC_HANDSHAKE_FAILED,
                             quic::QuicErrorCode::QUIC_INTERNAL_ERROR);
  socket_data.Resume();

  ASSERT_EQ(ERR_QUIC_HANDSHAKE_FAILED, callback_.WaitForResult());
}

}  // namespace net::test
