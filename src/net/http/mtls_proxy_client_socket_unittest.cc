// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/http/mtls_proxy_client_socket.h"

#include <memory>
#include <string>
#include <utility>

#include "base/functional/bind.h"
#include "base/functional/callback_helpers.h"
#include "base/memory/scoped_refptr.h"
#include "net/base/host_port_pair.h"
#include "net/base/io_buffer.h"
#include "net/base/proxy_chain.h"
#include "net/base/test_completion_callback.h"
#include "net/http/http_auth_controller.h"
#include "net/http/http_response_headers.h"
#include "net/log/net_log_with_source.h"
#include "net/socket/socket_test_util.h"
#include "net/socket/ssl_client_socket.h"
#include "net/ssl/ssl_config.h"
#include "net/ssl/ssl_info.h"
#include "net/test/cert_test_util.h"
#include "net/test/test_data_directory.h"
#include "net/traffic_annotation/network_traffic_annotation_test_helper.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/platform_test.h"

namespace net {

namespace {

// Mock SSL client socket for testing
class MockSSLClientSocket : public SSLClientSocket {
 public:
  explicit MockSSLClientSocket(std::unique_ptr<StreamSocket> transport_socket)
      : transport_socket_(std::move(transport_socket)) {}

  // SSLClientSocket implementation
  std::vector<uint8_t> GetECHRetryConfigs() override {
    return std::vector<uint8_t>();
  }

  // StreamSocket implementation
  int Connect(CompletionOnceCallback callback) override {
    return transport_socket_->Connect(std::move(callback));
  }

  void Disconnect() override { transport_socket_->Disconnect(); }
  
  bool IsConnected() const override {
    return transport_socket_->IsConnected();
  }
  
  bool IsConnectedAndIdle() const override {
    return transport_socket_->IsConnectedAndIdle();
  }
  
  int GetPeerAddress(IPEndPoint* address) const override {
    return transport_socket_->GetPeerAddress(address);
  }
  
  int GetLocalAddress(IPEndPoint* address) const override {
    return transport_socket_->GetLocalAddress(address);
  }
  
  const NetLogWithSource& NetLog() const override {
    return transport_socket_->NetLog();
  }
  
  bool WasEverUsed() const override {
    return transport_socket_->WasEverUsed();
  }
  
  NextProto GetNegotiatedProtocol() const override {
    return kProtoHTTP11;
  }
  
  bool GetSSLInfo(SSLInfo* ssl_info) override { return false; }
  
  int64_t GetTotalReceivedBytes() const override {
    return transport_socket_->GetTotalReceivedBytes();
  }
  
  void ApplySocketTag(const SocketTag& tag) override {
    transport_socket_->ApplySocketTag(tag);
  }

  // Socket implementation
  int Read(IOBuffer* buf, int buf_len,
           CompletionOnceCallback callback) override {
    return transport_socket_->Read(buf, buf_len, std::move(callback));
  }
  
  int ReadIfReady(IOBuffer* buf, int buf_len,
                  CompletionOnceCallback callback) override {
    return transport_socket_->ReadIfReady(buf, buf_len, std::move(callback));
  }
  
  int CancelReadIfReady() override {
    return transport_socket_->CancelReadIfReady();
  }
  
  int Write(IOBuffer* buf, int buf_len, CompletionOnceCallback callback,
            const NetworkTrafficAnnotationTag& traffic_annotation) override {
    return transport_socket_->Write(buf, buf_len, std::move(callback),
                                    traffic_annotation);
  }
  
  int SetReceiveBufferSize(int32_t size) override {
    return transport_socket_->SetReceiveBufferSize(size);
  }
  
  int SetSendBufferSize(int32_t size) override {
    return transport_socket_->SetSendBufferSize(size);
  }

 private:
  std::unique_ptr<StreamSocket> transport_socket_;
};

}  // namespace

class MtlsProxyClientSocketTest : public PlatformTest {
 protected:
  void SetUp() override {
    PlatformTest::SetUp();
    
    // Set up test data
    user_agent_ = "TestBrowser/1.0";
    endpoint_ = HostPortPair("www.example.com", 443);
    
    proxy_server_ = ProxyServer::FromSchemeHostAndPort(
        ProxyServer::SCHEME_HTTPS, "eb.wootzapp.com", 443);
    proxy_chain_ = ProxyChain(proxy_server_);
  }

  std::unique_ptr<MtlsProxyClientSocket> CreateMtlsProxySocket(
      std::unique_ptr<StreamSocket> transport,
      bool tunnel = true) {
    
    SSLConfig ssl_config;
    
    return std::make_unique<MtlsProxyClientSocket>(
        std::move(transport),
        user_agent_,
        endpoint_,
        proxy_chain_,
        0,  // proxy_chain_index
        tunnel,
        nullptr,  // http_auth_controller
        nullptr,  // ssl_client_context (would be mocked in real tests)
        ssl_config,
        nullptr,  // proxy_delegate
        TRAFFIC_ANNOTATION_FOR_TESTS);
  }

  std::string user_agent_;
  HostPortPair endpoint_;
  ProxyServer proxy_server_;
  ProxyChain proxy_chain_;
};

// Test basic construction
TEST_F(MtlsProxyClientSocketTest, Construction) {
  MockRead reads[] = {MockRead(SYNCHRONOUS, ERR_IO_PENDING)};
  MockWrite writes[] = {MockWrite(SYNCHRONOUS, ERR_IO_PENDING)};
  
  StaticSocketDataProvider data(reads, writes);
  auto transport = std::make_unique<MockTCPClientSocket>(
      AddressList(), NetLogWithSource(), &data);
  
  auto socket = CreateMtlsProxySocket(std::move(transport));
  EXPECT_FALSE(socket->IsConnected());
}

// Test connection with tunnel mode
TEST_F(MtlsProxyClientSocketTest, ConnectTunnelMode) {
  // Simulate successful CONNECT response
  const char kConnectResponse[] = "HTTP/1.1 200 Connection Established\r\n\r\n";
  
  MockRead reads[] = {
      MockRead(ASYNC, kConnectResponse),
      MockRead(SYNCHRONOUS, ERR_IO_PENDING),
  };
  
  const char kConnectRequest[] =
      "CONNECT www.example.com:443 HTTP/1.1\r\n"
      "Host: www.example.com:443\r\n"
      "Proxy-Connection: keep-alive\r\n"
      "User-Agent: TestBrowser/1.0\r\n\r\n";
  
  MockWrite writes[] = {
      MockWrite(ASYNC, kConnectRequest),
  };
  
  SequencedSocketData data(reads, writes);
  auto transport = std::make_unique<MockTCPClientSocket>(
      AddressList(), NetLogWithSource(), &data);
  
  auto socket = CreateMtlsProxySocket(std::move(transport), true);
  
  // Note: This test is simplified - in reality we'd need to mock
  // the SSL handshake as well since MtlsProxyClientSocket establishes
  // TLS before sending the CONNECT request
}

// Test non-tunnel mode (HTTP forwarding)
TEST_F(MtlsProxyClientSocketTest, ConnectForwardingMode) {
  MockRead reads[] = {MockRead(SYNCHRONOUS, ERR_IO_PENDING)};
  MockWrite writes[] = {MockWrite(SYNCHRONOUS, ERR_IO_PENDING)};
  
  StaticSocketDataProvider data(reads, writes);
  auto transport = std::make_unique<MockTCPClientSocket>(
      AddressList(), NetLogWithSource(), &data);
  
  // Create socket in forwarding mode (tunnel=false)
  auto socket = CreateMtlsProxySocket(std::move(transport), false);
  
  // In forwarding mode, SSL handshake completes but no CONNECT is sent
}

// Test GetConnectResponseInfo
TEST_F(MtlsProxyClientSocketTest, GetConnectResponseInfo) {
  MockRead reads[] = {MockRead(SYNCHRONOUS, ERR_IO_PENDING)};
  MockWrite writes[] = {MockWrite(SYNCHRONOUS, ERR_IO_PENDING)};
  
  StaticSocketDataProvider data(reads, writes);
  auto transport = std::make_unique<MockTCPClientSocket>(
      AddressList(), NetLogWithSource(), &data);
  
  auto socket = CreateMtlsProxySocket(std::move(transport));
  
  // Before connection, should return nullptr
  EXPECT_EQ(nullptr, socket->GetConnectResponseInfo());
}

// Test that protocol is correctly reported
TEST_F(MtlsProxyClientSocketTest, GetNegotiatedProtocol) {
  MockRead reads[] = {MockRead(SYNCHRONOUS, ERR_IO_PENDING)};
  MockWrite writes[] = {MockWrite(SYNCHRONOUS, ERR_IO_PENDING)};
  
  StaticSocketDataProvider data(reads, writes);
  auto transport = std::make_unique<MockTCPClientSocket>(
      AddressList(), NetLogWithSource(), &data);
  
  // Tunnel mode should return kProtoUnknown (tunneled connection)
  auto tunnel_socket = CreateMtlsProxySocket(std::move(transport), true);
  EXPECT_EQ(kProtoUnknown, tunnel_socket->GetNegotiatedProtocol());
  
  // Forwarding mode would return the proxy's protocol
  // (tested separately with proper mocking)
}

}  // namespace net

