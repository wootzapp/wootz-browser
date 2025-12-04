// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_HTTP_MTLS_PROXY_CLIENT_SOCKET_H_
#define NET_HTTP_MTLS_PROXY_CLIENT_SOCKET_H_

#include <memory>
#include <string>

#include "base/memory/raw_ptr.h"
#include "base/memory/scoped_refptr.h"
#include "net/base/completion_once_callback.h"
#include "net/base/completion_repeating_callback.h"
#include "net/base/host_port_pair.h"
#include "net/base/net_export.h"
#include "net/base/proxy_chain.h"
#include "net/http/http_auth_controller.h"
#include "net/http/http_request_headers.h"
#include "net/http/http_request_info.h"
#include "net/http/http_response_info.h"
#include "net/http/proxy_client_socket.h"
#include "net/log/net_log_with_source.h"
#include "net/socket/ssl_client_socket.h"
#include "net/ssl/ssl_config.h"
#include "net/traffic_annotation/network_traffic_annotation.h"

namespace net {

class GrowableIOBuffer;
class HttpStreamParser;
class IOBuffer;
class ProxyDelegate;
class StreamSocket;
class SSLClientContext;
class X509Certificate;
class SSLPrivateKey;

// MtlsProxyClientSocket establishes an mTLS connection to a proxy server
// (eb.wootzapp.com:443) and then tunnels traffic through it using CONNECT.
// This socket handles both:
//  1. HTTPS tunneling via CONNECT method
//  2. HTTP absolute-form forwarding for plain HTTP requests
//
// The key difference from HttpProxyClientSocket is that this establishes
// an SSL connection with client certificate authentication to the proxy itself.
class NET_EXPORT_PRIVATE MtlsProxyClientSocket : public ProxyClientSocket {
 public:
  // Creates an mTLS proxy socket. The socket will first establish a TLS
  // connection to the proxy server with client certificate authentication,
  // then establish a tunnel (if tunnel=true) or forward HTTP requests.
  MtlsProxyClientSocket(std::unique_ptr<StreamSocket> tcp_socket,
                        const std::string& user_agent,
                        const HostPortPair& endpoint,
                        const ProxyChain& proxy_chain,
                        size_t proxy_chain_index,
                        bool tunnel,
                        scoped_refptr<HttpAuthController> http_auth_controller,
                        SSLClientContext* ssl_client_context,
                        const SSLConfig& ssl_config,
                        ProxyDelegate* proxy_delegate,
                        const NetworkTrafficAnnotationTag& traffic_annotation);

  MtlsProxyClientSocket(const MtlsProxyClientSocket&) = delete;
  MtlsProxyClientSocket& operator=(const MtlsProxyClientSocket&) = delete;

  ~MtlsProxyClientSocket() override;

  // ProxyClientSocket implementation.
  const HttpResponseInfo* GetConnectResponseInfo() const override;
  int RestartWithAuth(CompletionOnceCallback callback) override;
  const scoped_refptr<HttpAuthController>& GetAuthController() const override;

  // StreamSocket implementation.
  int Connect(CompletionOnceCallback callback) override;
  void Disconnect() override;
  bool IsConnected() const override;
  bool IsConnectedAndIdle() const override;
  const NetLogWithSource& NetLog() const override;
  bool WasEverUsed() const override;
  NextProto GetNegotiatedProtocol() const override;
  bool GetSSLInfo(SSLInfo* ssl_info) override;
  int64_t GetTotalReceivedBytes() const override;
  void ApplySocketTag(const SocketTag& tag) override;

  // Socket implementation.
  int Read(IOBuffer* buf,
           int buf_len,
           CompletionOnceCallback callback) override;
  int ReadIfReady(IOBuffer* buf,
                  int buf_len,
                  CompletionOnceCallback callback) override;
  int CancelReadIfReady() override;
  int Write(IOBuffer* buf,
            int buf_len,
            CompletionOnceCallback callback,
            const NetworkTrafficAnnotationTag& traffic_annotation) override;
  int SetReceiveBufferSize(int32_t size) override;
  int SetSendBufferSize(int32_t size) override;
  int GetPeerAddress(IPEndPoint* address) const override;
  int GetLocalAddress(IPEndPoint* address) const override;

 private:
  enum State {
    STATE_NONE,
    STATE_SSL_CONNECT,
    STATE_SSL_CONNECT_COMPLETE,
    STATE_GENERATE_AUTH_TOKEN,
    STATE_GENERATE_AUTH_TOKEN_COMPLETE,
    STATE_SEND_REQUEST,
    STATE_SEND_REQUEST_COMPLETE,
    STATE_READ_HEADERS,
    STATE_READ_HEADERS_COMPLETE,
    STATE_DRAIN_BODY,
    STATE_DRAIN_BODY_COMPLETE,
    STATE_DONE,
  };

  static const int kDrainBodyBufferSize = 1024;

  int PrepareForAuthRestart();
  int DidDrainBodyForAuthRestart();

  void DoCallback(int result);
  void OnIOComplete(int result);

  int DoLoop(int last_io_result);
  
  // Establish SSL connection to proxy with mTLS
  int DoSSLConnect();
  int DoSSLConnectComplete(int result);
  
  // HTTP tunnel establishment (similar to HttpProxyClientSocket)
  int DoGenerateAuthToken();
  int DoGenerateAuthTokenComplete(int result);
  int DoSendRequest();
  int DoSendRequestComplete(int result);
  int DoReadHeaders();
  int DoReadHeadersComplete(int result);
  int DoDrainBody();
  int DoDrainBodyComplete(int result);

  bool CheckDone();

  CompletionRepeatingCallback io_callback_;
  State next_state_ = STATE_NONE;

  CompletionOnceCallback user_callback_;

  // Connection parameters
  const std::string user_agent_;
  const HostPortPair endpoint_;
  const HostPortPair proxy_endpoint_;
  const ProxyChain proxy_chain_;
  const size_t proxy_chain_index_;
  const bool tunnel_;  // true for CONNECT tunnel, false for HTTP forwarding
  
  // SSL context and configuration for mTLS to proxy
  raw_ptr<SSLClientContext> ssl_client_context_;
  SSLConfig ssl_config_;
  
  // Underlying sockets: TCP -> SSL (mTLS to proxy) -> HTTP tunnel
  std::unique_ptr<StreamSocket> tcp_socket_;
  std::unique_ptr<SSLClientSocket> ssl_socket_;

  // HTTP tunnel setup
  HttpRequestInfo request_;
  HttpResponseInfo response_;
  scoped_refptr<GrowableIOBuffer> parser_buf_;
  scoped_refptr<IOBuffer> drain_buf_;
  std::unique_ptr<HttpStreamParser> http_stream_parser_;

  bool is_reused_ = false;

  scoped_refptr<HttpAuthController> auth_;

  std::string request_line_;
  HttpRequestHeaders request_headers_;

  raw_ptr<ProxyDelegate> proxy_delegate_;

  const NetworkTrafficAnnotationTag traffic_annotation_;

  const NetLogWithSource net_log_;
};

}  // namespace net

#endif  // NET_HTTP_MTLS_PROXY_CLIENT_SOCKET_H_

