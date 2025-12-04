// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/http/mtls_proxy_client_socket.h"

#include <memory>
#include <utility>

#include "base/functional/bind.h"
#include "base/functional/callback_helpers.h"
#include "base/strings/string_util.h"
#include "base/values.h"
#include "net/base/auth.h"
#include "net/base/host_port_pair.h"
#include "net/base/io_buffer.h"
#include "net/base/proxy_chain.h"
#include "net/base/proxy_delegate.h"
#include "net/http/http_basic_stream.h"
#include "net/http/http_log_util.h"
#include "net/http/http_network_session.h"
#include "net/http/http_request_info.h"
#include "net/http/http_response_headers.h"
#include "net/http/http_stream_parser.h"
#include "net/log/net_log.h"
#include "net/log/net_log_event_type.h"
#include "net/socket/ssl_client_socket.h"
#include "net/socket/stream_socket.h"
#include "net/ssl/ssl_info.h"
#include "url/gurl.h"

namespace net {

const int MtlsProxyClientSocket::kDrainBodyBufferSize;

MtlsProxyClientSocket::MtlsProxyClientSocket(
    std::unique_ptr<StreamSocket> tcp_socket,
    const std::string& user_agent,
    const HostPortPair& endpoint,
    const ProxyChain& proxy_chain,
    size_t proxy_chain_index,
    bool tunnel,
    scoped_refptr<HttpAuthController> http_auth_controller,
    SSLClientContext* ssl_client_context,
    const SSLConfig& ssl_config,
    ProxyDelegate* proxy_delegate,
    const NetworkTrafficAnnotationTag& traffic_annotation)
    : io_callback_(
          base::BindRepeating(&MtlsProxyClientSocket::OnIOComplete,
                              base::Unretained(this))),
      user_agent_(user_agent),
      endpoint_(endpoint),
      proxy_endpoint_(proxy_chain.GetProxyServer(proxy_chain_index)
                          .host_port_pair()),
      proxy_chain_(proxy_chain),
      proxy_chain_index_(proxy_chain_index),
      tunnel_(tunnel),
      ssl_client_context_(ssl_client_context),
      ssl_config_(ssl_config),
      tcp_socket_(std::move(tcp_socket)),
      auth_(std::move(http_auth_controller)),
      proxy_delegate_(proxy_delegate),
      traffic_annotation_(traffic_annotation),
      net_log_(tcp_socket_ ? tcp_socket_->NetLog() : NetLogWithSource()) {
  // Synthesize the bits of a request that are actually used.
  if (tunnel_) {
    request_.url = GURL("https://" + endpoint.ToString());
    request_.method = "CONNECT";
  } else {
    // For HTTP forwarding, the full URL will be set per-request
    request_.method = "GET";  // Will be overridden
  }
}

MtlsProxyClientSocket::~MtlsProxyClientSocket() {
  Disconnect();
}

int MtlsProxyClientSocket::RestartWithAuth(CompletionOnceCallback callback) {
  DCHECK_EQ(STATE_NONE, next_state_);
  DCHECK(user_callback_.is_null());

  int rv = PrepareForAuthRestart();
  if (rv != OK)
    return rv;

  rv = DoLoop(OK);
  if (rv == ERR_IO_PENDING) {
    if (!callback.is_null())
      user_callback_ = std::move(callback);
  }

  return rv;
}

const scoped_refptr<HttpAuthController>&
MtlsProxyClientSocket::GetAuthController() const {
  return auth_;
}

const HttpResponseInfo* MtlsProxyClientSocket::GetConnectResponseInfo() const {
  return response_.headers.get() ? &response_ : nullptr;
}

int MtlsProxyClientSocket::Connect(CompletionOnceCallback callback) {
  DCHECK(tcp_socket_);
  DCHECK(user_callback_.is_null());

  if (next_state_ == STATE_DONE)
    return OK;

  DCHECK_EQ(STATE_NONE, next_state_);
  next_state_ = STATE_SSL_CONNECT;

  int rv = DoLoop(OK);
  if (rv == ERR_IO_PENDING)
    user_callback_ = std::move(callback);
  return rv;
}

void MtlsProxyClientSocket::Disconnect() {
  if (ssl_socket_)
    ssl_socket_->Disconnect();
  if (tcp_socket_)
    tcp_socket_->Disconnect();

  // Reset other states to make sure they aren't mistakenly used later.
  next_state_ = STATE_NONE;
  user_callback_.Reset();
}

bool MtlsProxyClientSocket::IsConnected() const {
  if (tunnel_) {
    return next_state_ == STATE_DONE && ssl_socket_ && ssl_socket_->IsConnected();
  } else {
    // For HTTP forwarding, we're connected after SSL handshake
    return ssl_socket_ && ssl_socket_->IsConnected();
  }
}

bool MtlsProxyClientSocket::IsConnectedAndIdle() const {
  if (tunnel_) {
    return next_state_ == STATE_DONE && ssl_socket_ && ssl_socket_->IsConnectedAndIdle();
  } else {
    return ssl_socket_ && ssl_socket_->IsConnectedAndIdle();
  }
}

const NetLogWithSource& MtlsProxyClientSocket::NetLog() const {
  return net_log_;
}

bool MtlsProxyClientSocket::WasEverUsed() const {
  if (ssl_socket_)
    return ssl_socket_->WasEverUsed();
  if (tcp_socket_)
    return tcp_socket_->WasEverUsed();
  return false;
}

NextProto MtlsProxyClientSocket::GetNegotiatedProtocol() const {
  if (tunnel_) {
    // For tunnels, return kProtoUnknown as we're tunneling
    return kProtoUnknown;
  } else {
    // For HTTP forwarding, return the protocol negotiated with the proxy
    if (ssl_socket_) {
      return ssl_socket_->GetNegotiatedProtocol();
    }
    return kProtoUnknown;
  }
}

bool MtlsProxyClientSocket::GetSSLInfo(SSLInfo* ssl_info) {
  if (tunnel_) {
    // For tunnels, don't delegate - the tunnel is to the origin, not the proxy
    return false;
  } else {
    // For HTTP forwarding, return the SSL info of the connection to the proxy
    if (ssl_socket_) {
      return ssl_socket_->GetSSLInfo(ssl_info);
    }
    return false;
  }
}

int64_t MtlsProxyClientSocket::GetTotalReceivedBytes() const {
  if (ssl_socket_)
    return ssl_socket_->GetTotalReceivedBytes();
  if (tcp_socket_)
    return tcp_socket_->GetTotalReceivedBytes();
  return 0;
}

void MtlsProxyClientSocket::ApplySocketTag(const SocketTag& tag) {
  if (tcp_socket_)
    tcp_socket_->ApplySocketTag(tag);
}

int MtlsProxyClientSocket::Read(IOBuffer* buf,
                                int buf_len,
                                CompletionOnceCallback callback) {
  DCHECK(user_callback_.is_null());
  
  if (tunnel_) {
    if (!CheckDone())
      return ERR_TUNNEL_CONNECTION_FAILED;
  }
  
  if (ssl_socket_)
    return ssl_socket_->Read(buf, buf_len, std::move(callback));
  
  return ERR_SOCKET_NOT_CONNECTED;
}

int MtlsProxyClientSocket::ReadIfReady(IOBuffer* buf,
                                       int buf_len,
                                       CompletionOnceCallback callback) {
  DCHECK(user_callback_.is_null());
  
  if (tunnel_) {
    if (!CheckDone())
      return ERR_TUNNEL_CONNECTION_FAILED;
  }
  
  if (ssl_socket_)
    return ssl_socket_->ReadIfReady(buf, buf_len, std::move(callback));
  
  return ERR_SOCKET_NOT_CONNECTED;
}

int MtlsProxyClientSocket::CancelReadIfReady() {
  if (ssl_socket_)
    return ssl_socket_->CancelReadIfReady();
  return ERR_SOCKET_NOT_CONNECTED;
}

int MtlsProxyClientSocket::Write(
    IOBuffer* buf,
    int buf_len,
    CompletionOnceCallback callback,
    const NetworkTrafficAnnotationTag& traffic_annotation) {
  if (tunnel_) {
    DCHECK_EQ(STATE_DONE, next_state_);
  }
  DCHECK(user_callback_.is_null());

  if (ssl_socket_)
    return ssl_socket_->Write(buf, buf_len, std::move(callback),
                              traffic_annotation);
  
  return ERR_SOCKET_NOT_CONNECTED;
}

int MtlsProxyClientSocket::SetReceiveBufferSize(int32_t size) {
  if (ssl_socket_)
    return ssl_socket_->SetReceiveBufferSize(size);
  if (tcp_socket_)
    return tcp_socket_->SetReceiveBufferSize(size);
  return ERR_SOCKET_NOT_CONNECTED;
}

int MtlsProxyClientSocket::SetSendBufferSize(int32_t size) {
  if (ssl_socket_)
    return ssl_socket_->SetSendBufferSize(size);
  if (tcp_socket_)
    return tcp_socket_->SetSendBufferSize(size);
  return ERR_SOCKET_NOT_CONNECTED;
}

int MtlsProxyClientSocket::GetPeerAddress(IPEndPoint* address) const {
  if (ssl_socket_)
    return ssl_socket_->GetPeerAddress(address);
  if (tcp_socket_)
    return tcp_socket_->GetPeerAddress(address);
  return ERR_SOCKET_NOT_CONNECTED;
}

int MtlsProxyClientSocket::GetLocalAddress(IPEndPoint* address) const {
  if (ssl_socket_)
    return ssl_socket_->GetLocalAddress(address);
  if (tcp_socket_)
    return tcp_socket_->GetLocalAddress(address);
  return ERR_SOCKET_NOT_CONNECTED;
}

int MtlsProxyClientSocket::PrepareForAuthRestart() {
  if (!response_.headers.get())
    return ERR_CONNECTION_RESET;

  if (!response_.headers->IsKeepAlive() ||
      !http_stream_parser_->CanFindEndOfResponse() ||
      !ssl_socket_->IsConnected()) {
    ssl_socket_->Disconnect();
    return ERR_UNABLE_TO_REUSE_CONNECTION_FOR_PROXY_AUTH;
  }

  if (!http_stream_parser_->IsResponseBodyComplete()) {
    next_state_ = STATE_DRAIN_BODY;
    drain_buf_ = base::MakeRefCounted<IOBufferWithSize>(kDrainBodyBufferSize);
    return OK;
  }

  return DidDrainBodyForAuthRestart();
}

int MtlsProxyClientSocket::DidDrainBodyForAuthRestart() {
  if (!ssl_socket_->IsConnectedAndIdle())
    return ERR_UNABLE_TO_REUSE_CONNECTION_FOR_PROXY_AUTH;

  next_state_ = STATE_GENERATE_AUTH_TOKEN;
  is_reused_ = true;

  drain_buf_ = nullptr;
  parser_buf_ = nullptr;
  http_stream_parser_.reset();
  request_line_.clear();
  request_headers_.Clear();
  response_ = HttpResponseInfo();
  return OK;
}

void MtlsProxyClientSocket::DoCallback(int result) {
  DCHECK_NE(ERR_IO_PENDING, result);
  DCHECK(!user_callback_.is_null());

  std::move(user_callback_).Run(result);
}

void MtlsProxyClientSocket::OnIOComplete(int result) {
  DCHECK_NE(STATE_NONE, next_state_);
  DCHECK_NE(STATE_DONE, next_state_);
  int rv = DoLoop(result);
  if (rv != ERR_IO_PENDING)
    DoCallback(rv);
}

int MtlsProxyClientSocket::DoLoop(int last_io_result) {
  DCHECK_NE(next_state_, STATE_NONE);
  int rv = last_io_result;
  do {
    State state = next_state_;
    next_state_ = STATE_NONE;
    switch (state) {
      case STATE_SSL_CONNECT:
        DCHECK_EQ(OK, rv);
        rv = DoSSLConnect();
        break;
      case STATE_SSL_CONNECT_COMPLETE:
        rv = DoSSLConnectComplete(rv);
        break;
      case STATE_GENERATE_AUTH_TOKEN:
        DCHECK_EQ(OK, rv);
        rv = DoGenerateAuthToken();
        break;
      case STATE_GENERATE_AUTH_TOKEN_COMPLETE:
        rv = DoGenerateAuthTokenComplete(rv);
        break;
      case STATE_SEND_REQUEST:
        DCHECK_EQ(OK, rv);
        net_log_.BeginEvent(
            NetLogEventType::HTTP_TRANSACTION_TUNNEL_SEND_REQUEST);
        rv = DoSendRequest();
        break;
      case STATE_SEND_REQUEST_COMPLETE:
        rv = DoSendRequestComplete(rv);
        net_log_.EndEventWithNetErrorCode(
            NetLogEventType::HTTP_TRANSACTION_TUNNEL_SEND_REQUEST, rv);
        break;
      case STATE_READ_HEADERS:
        DCHECK_EQ(OK, rv);
        net_log_.BeginEvent(
            NetLogEventType::HTTP_TRANSACTION_TUNNEL_READ_HEADERS);
        rv = DoReadHeaders();
        break;
      case STATE_READ_HEADERS_COMPLETE:
        rv = DoReadHeadersComplete(rv);
        net_log_.EndEventWithNetErrorCode(
            NetLogEventType::HTTP_TRANSACTION_TUNNEL_READ_HEADERS, rv);
        break;
      case STATE_DRAIN_BODY:
        DCHECK_EQ(OK, rv);
        rv = DoDrainBody();
        break;
      case STATE_DRAIN_BODY_COMPLETE:
        rv = DoDrainBodyComplete(rv);
        break;
      case STATE_DONE:
        break;
      default:
        NOTREACHED_IN_MIGRATION() << "bad state";
        rv = ERR_UNEXPECTED;
        break;
    }
  } while (rv != ERR_IO_PENDING && next_state_ != STATE_NONE &&
           next_state_ != STATE_DONE);
  return rv;
}

int MtlsProxyClientSocket::DoSSLConnect() {
  DCHECK(tcp_socket_);
  DCHECK(!ssl_socket_);
  
  next_state_ = STATE_SSL_CONNECT_COMPLETE;
  
  // Create SSL socket over the TCP connection to the proxy
  // with mTLS (client certificate authentication)
  if (ssl_client_context_) {
    ssl_socket_ = ssl_client_context_->CreateSSLClientSocket(
        std::move(tcp_socket_), proxy_endpoint_, ssl_config_);
    
    return ssl_socket_->Connect(io_callback_);
  }
  
  return ERR_UNEXPECTED;
}

int MtlsProxyClientSocket::DoSSLConnectComplete(int result) {
  if (result != OK) {
    return result;
  }
  
  // SSL connection with mTLS established successfully
  if (tunnel_) {
    // For CONNECT tunnels, proceed to send CONNECT request
    next_state_ = STATE_GENERATE_AUTH_TOKEN;
  } else {
    // For HTTP forwarding, we're done with connection setup
    // The actual HTTP requests will be sent through this socket
    next_state_ = STATE_DONE;
  }
  
  return OK;
}

int MtlsProxyClientSocket::DoGenerateAuthToken() {
  next_state_ = STATE_GENERATE_AUTH_TOKEN_COMPLETE;
  if (auth_) {
    return auth_->MaybeGenerateAuthToken(&request_, io_callback_, net_log_);
  }
  return OK;
}

int MtlsProxyClientSocket::DoGenerateAuthTokenComplete(int result) {
  DCHECK_NE(ERR_IO_PENDING, result);
  if (result == OK)
    next_state_ = STATE_SEND_REQUEST;
  return result;
}

int MtlsProxyClientSocket::DoSendRequest() {
  next_state_ = STATE_SEND_REQUEST_COMPLETE;

  if (request_line_.empty()) {
    DCHECK(request_headers_.IsEmpty());

    HttpRequestHeaders extra_headers;
    if (auth_ && auth_->HaveAuth())
      auth_->AddAuthorizationHeader(&extra_headers);
    
    response_.did_use_http_auth =
        extra_headers.HasHeader(HttpRequestHeaders::kProxyAuthorization);

    if (proxy_delegate_) {
      HttpRequestHeaders proxy_delegate_headers;
      int result = proxy_delegate_->OnBeforeTunnelRequest(
          proxy_chain_, proxy_chain_index_, &proxy_delegate_headers);
      if (result < 0) {
        return result;
      }
      extra_headers.MergeFrom(proxy_delegate_headers);
    }

    BuildTunnelRequest(endpoint_, extra_headers, user_agent_, &request_line_,
                       &request_headers_);

    NetLogRequestHeaders(net_log_,
                         NetLogEventType::HTTP_TRANSACTION_SEND_TUNNEL_HEADERS,
                         request_line_, &request_headers_);
  }

  parser_buf_ = base::MakeRefCounted<GrowableIOBuffer>();
  http_stream_parser_ = std::make_unique<HttpStreamParser>(
      ssl_socket_.get(), is_reused_, request_.url, request_.method,
      /*upload_data_stream=*/nullptr, parser_buf_.get(), net_log_);
  return http_stream_parser_->SendRequest(request_line_, request_headers_,
                                          traffic_annotation_, &response_,
                                          io_callback_);
}

int MtlsProxyClientSocket::DoSendRequestComplete(int result) {
  if (result < 0)
    return result;

  next_state_ = STATE_READ_HEADERS;
  return OK;
}

int MtlsProxyClientSocket::DoReadHeaders() {
  next_state_ = STATE_READ_HEADERS_COMPLETE;
  return http_stream_parser_->ReadResponseHeaders(io_callback_);
}

int MtlsProxyClientSocket::DoReadHeadersComplete(int result) {
  if (result < 0)
    return result;

  if (response_.headers->GetHttpVersion() < HttpVersion(1, 0))
    return ERR_TUNNEL_CONNECTION_FAILED;

  NetLogResponseHeaders(
      net_log_, NetLogEventType::HTTP_TRANSACTION_READ_TUNNEL_RESPONSE_HEADERS,
      response_.headers.get());

  if (proxy_delegate_) {
    int rv = proxy_delegate_->OnTunnelHeadersReceived(
        proxy_chain_, proxy_chain_index_, *response_.headers);
    if (rv != OK) {
      return rv;
    }
  }

  switch (response_.headers->response_code()) {
    case 200:  // OK
      next_state_ = STATE_DONE;
      return OK;

    case 407:  // Proxy Authentication Required
      next_state_ = STATE_NONE;
      if (auth_ && auth_->HaveAuth()) {
        next_state_ = STATE_DONE;
      }
      SanitizeProxyAuth(response_);
      return HandleProxyAuthChallenge(auth_.get(), &response_, net_log_);

    default:
      next_state_ = STATE_NONE;
      return ERR_TUNNEL_CONNECTION_FAILED;
  }
}

int MtlsProxyClientSocket::DoDrainBody() {
  next_state_ = STATE_DRAIN_BODY_COMPLETE;
  return http_stream_parser_->ReadResponseBody(
      drain_buf_.get(), kDrainBodyBufferSize, io_callback_);
}

int MtlsProxyClientSocket::DoDrainBodyComplete(int result) {
  if (result < 0)
    return ERR_UNABLE_TO_REUSE_CONNECTION_FOR_PROXY_AUTH;

  if (!http_stream_parser_->IsResponseBodyComplete()) {
    next_state_ = STATE_DRAIN_BODY;
    return OK;
  }

  return DidDrainBodyForAuthRestart();
}

bool MtlsProxyClientSocket::CheckDone() {
  return next_state_ == STATE_DONE;
}

}  // namespace net

