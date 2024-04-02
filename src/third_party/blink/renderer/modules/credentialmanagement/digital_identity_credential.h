// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_CREDENTIALMANAGEMENT_DIGITAL_IDENTITY_CREDENTIAL_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_CREDENTIALMANAGEMENT_DIGITAL_IDENTITY_CREDENTIAL_H_

#include "third_party/blink/renderer/bindings/core/v8/script_promise.h"
#include "third_party/blink/renderer/bindings/core/v8/script_promise_resolver.h"
#include "third_party/blink/renderer/modules/modules_export.h"
#include "third_party/blink/renderer/platform/heap/garbage_collected.h"

namespace blink {
class Credential;
class CredentialRequestOptions;
class ExceptionState;
class ScriptState;

// Returns whether `options` contains a credential of digital-identity type.
//
// The return value is not affected by additional non-digital-identity
// credential types in `options`.
MODULES_EXPORT bool IsDigitalIdentityCredentialType(
    const CredentialRequestOptions& options);

// Requests the digital-identity credential specified by `options`.
MODULES_EXPORT ScriptPromiseTyped<IDLNullable<Credential>>
DiscoverDigitalIdentityCredentialFromExternalSource(
    ScriptState* script_state,
    ScriptPromiseResolverTyped<IDLNullable<Credential>>* resolver,
    const CredentialRequestOptions& options,
    ExceptionState& exception_state);

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_CREDENTIALMANAGEMENT_DIGITAL_IDENTITY_CREDENTIAL_H_
