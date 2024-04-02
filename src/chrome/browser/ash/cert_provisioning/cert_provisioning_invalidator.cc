// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ash/cert_provisioning/cert_provisioning_invalidator.h"

#include <utility>

#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "chrome/browser/ash/cert_provisioning/cert_provisioning_common.h"
#include "chrome/browser/invalidation/profile_invalidation_provider_factory.h"
#include "components/invalidation/impl/profile_invalidation_provider.h"
#include "components/invalidation/public/invalidation.h"
#include "components/invalidation/public/invalidation_service.h"
#include "components/invalidation/public/invalidation_util.h"
#include "components/invalidation/public/invalidator_state.h"
#include "components/policy/core/common/cloud/cloud_policy_constants.h"

namespace ash::cert_provisioning {

namespace {

// Topics that start with this prefix are considered to be "public" FCM topics.
// This allows us to migrate to "private" FCM topics (which would get a
// different prefix) server-side without client-side changes.
constexpr char kFcmCertProvisioningPublicTopicPrefix[] = "cert-";

// Shall be expanded to cert.[scope].[topic]
constexpr char kOwnerNameFormat[] = "cert.%s.%s";

const char* CertScopeToString(CertScope scope) {
  switch (scope) {
    case CertScope::kUser:
      return "user";
    case CertScope::kDevice:
      return "device";
  }

  NOTREACHED() << "Unknown cert scope: " << static_cast<int>(scope);
  return "";
}

}  // namespace

//=============== CertProvisioningInvalidationHandler ==========================

namespace internal {

// static
std::unique_ptr<CertProvisioningInvalidationHandler>
CertProvisioningInvalidationHandler::BuildAndRegister(
    CertScope scope,
    invalidation::InvalidationService* invalidation_service,
    const invalidation::Topic& topic,
    OnInvalidationEventCallback on_invalidation_event_callback) {
  auto invalidator = std::make_unique<CertProvisioningInvalidationHandler>(
      scope, invalidation_service, topic,
      std::move(on_invalidation_event_callback));

  if (!invalidator->Register()) {
    return nullptr;
  }

  return invalidator;
}

CertProvisioningInvalidationHandler::CertProvisioningInvalidationHandler(
    CertScope scope,
    invalidation::InvalidationService* invalidation_service,
    const invalidation::Topic& topic,
    OnInvalidationEventCallback on_invalidation_event_callback)
    : scope_(scope),
      invalidation_service_(invalidation_service),
      topic_(topic),
      on_invalidation_event_callback_(
          std::move(on_invalidation_event_callback)) {
  DCHECK(invalidation_service_);
  DCHECK(!on_invalidation_event_callback_.is_null());
}

CertProvisioningInvalidationHandler::~CertProvisioningInvalidationHandler() {
  // Unregister is not called here so that subscription can be preserved if
  // browser restarts. If subscription is not needed a user must call Unregister
  // explicitly.
}

bool CertProvisioningInvalidationHandler::Register() {
  if (IsRegistered()) {
    return true;
  }

  OnInvalidatorStateChange(invalidation_service_->GetInvalidatorState());

  invalidation_service_observation_.Observe(invalidation_service_.get());

  if (!invalidation_service_->UpdateInterestedTopics(this,
                                                     /*topics=*/{topic_})) {
    LOG(WARNING) << "Failed to register with topic " << topic_;
    return false;
  }

  return true;
}

void CertProvisioningInvalidationHandler::Unregister() {
  if (!IsRegistered()) {
    return;
  }

  // Assuming that updating invalidator's topics with empty set can never fail
  // since failure is only possible non-empty set with topic associated with
  // some other invalidator.
  const bool topics_reset = invalidation_service_->UpdateInterestedTopics(
      this, invalidation::TopicSet());
  DCHECK(topics_reset);
  DCHECK(invalidation_service_observation_.IsObservingSource(
      invalidation_service_.get()));
  invalidation_service_observation_.Reset();
}

void CertProvisioningInvalidationHandler::OnInvalidatorStateChange(
    invalidation::InvalidatorState state) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
}

void CertProvisioningInvalidationHandler::OnSuccessfullySubscribed(
    const invalidation::Topic& topic) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  CHECK_EQ(topic, topic_)
      << "Successfully subscribed notification for wrong topic";

  on_invalidation_event_callback_.Run(
      InvalidationEvent::kSuccessfullySubscribed);
}

void CertProvisioningInvalidationHandler::OnIncomingInvalidation(
    const invalidation::Invalidation& invalidation) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (!AreInvalidationsEnabled()) {
    LOG(WARNING) << "Unexpected invalidation received.";
  }

  CHECK(invalidation.topic() == topic_)
      << "Incoming invalidation does not contain invalidation"
         " for certificate topic";

  invalidation.Acknowledge();

  on_invalidation_event_callback_.Run(InvalidationEvent::kInvalidationReceived);
}

std::string CertProvisioningInvalidationHandler::GetOwnerName() const {
  return base::StringPrintf(kOwnerNameFormat, CertScopeToString(scope_),
                            topic_.c_str());
}

bool CertProvisioningInvalidationHandler::IsPublicTopic(
    const invalidation::Topic& topic) const {
  return base::StartsWith(topic, kFcmCertProvisioningPublicTopicPrefix,
                          base::CompareCase::SENSITIVE);
}

bool CertProvisioningInvalidationHandler::IsRegistered() const {
  return invalidation_service_ && invalidation_service_->HasObserver(this);
}

bool CertProvisioningInvalidationHandler::AreInvalidationsEnabled() const {
  return IsRegistered() && invalidation_service_->GetInvalidatorState() ==
                               invalidation::INVALIDATIONS_ENABLED;
}

}  // namespace internal

//=============== CertProvisioningUserInvalidator ==============================

CertProvisioningInvalidator::CertProvisioningInvalidator() = default;
CertProvisioningInvalidator::~CertProvisioningInvalidator() = default;

void CertProvisioningInvalidator::Unregister() {
  if (invalidation_handler_) {
    invalidation_handler_->Unregister();
    invalidation_handler_.reset();
  }
}

//=============== CertProvisioningUserInvalidatorFactory =======================

CertProvisioningUserInvalidatorFactory::CertProvisioningUserInvalidatorFactory(
    Profile* profile)
    : profile_(profile) {
  DCHECK(profile_);
}

std::unique_ptr<CertProvisioningInvalidator>
CertProvisioningUserInvalidatorFactory::Create() {
  return std::make_unique<CertProvisioningUserInvalidator>(profile_);
}

//=============== CertProvisioningUserInvalidator ==============================

CertProvisioningUserInvalidator::CertProvisioningUserInvalidator(
    Profile* profile)
    : profile_(profile) {
  DCHECK(profile_);
}

void CertProvisioningUserInvalidator::Register(
    const invalidation::Topic& topic,
    OnInvalidationEventCallback on_invalidation_event_callback) {
  invalidation::ProfileInvalidationProvider* invalidation_provider =
      invalidation::ProfileInvalidationProviderFactory::GetForProfile(profile_);
  DCHECK(invalidation_provider);
  invalidation::InvalidationService* invalidation_service =
      invalidation_provider->GetInvalidationServiceForCustomSender(
          policy::kPolicyFCMInvalidationSenderID);
  DCHECK(invalidation_service);

  invalidation_handler_ =
      internal::CertProvisioningInvalidationHandler::BuildAndRegister(
          CertScope::kUser, invalidation_service, topic,
          std::move(on_invalidation_event_callback));
  if (!invalidation_handler_) {
    LOG(ERROR) << "Failed to register for invalidation topic";
  }
}

//=============== CertProvisioningDeviceInvalidatorFactory =====================

CertProvisioningDeviceInvalidatorFactory::
    CertProvisioningDeviceInvalidatorFactory(
        policy::AffiliatedInvalidationServiceProvider* service_provider)
    : service_provider_(service_provider) {
  DCHECK(service_provider_);
}

std::unique_ptr<CertProvisioningInvalidator>
CertProvisioningDeviceInvalidatorFactory::Create() {
  return std::make_unique<CertProvisioningDeviceInvalidator>(service_provider_);
}

//=============== CertProvisioningDeviceInvalidator ============================

CertProvisioningDeviceInvalidator::CertProvisioningDeviceInvalidator(
    policy::AffiliatedInvalidationServiceProvider* service_provider)
    : service_provider_(service_provider) {
  DCHECK(service_provider_);
}

CertProvisioningDeviceInvalidator::~CertProvisioningDeviceInvalidator() {
  // As mentioned in the class-level comment, this intentionally doesn't call
  // Unregister so that a subscription can be preserved across process restarts.
  //
  // Note that it is OK to call this even if this instance has not called
  // RegisterConsumer yet.
  service_provider_->UnregisterConsumer(this);
}

void CertProvisioningDeviceInvalidator::Register(
    const invalidation::Topic& topic,
    OnInvalidationEventCallback on_invalidation_event_callback) {
  topic_ = topic;
  DCHECK(!topic_.empty());
  on_invalidation_event_callback_ = std::move(on_invalidation_event_callback);
  service_provider_->RegisterConsumer(this);
}

void CertProvisioningDeviceInvalidator::Unregister() {
  service_provider_->UnregisterConsumer(this);
  CertProvisioningInvalidator::Unregister();
  topic_.clear();
}

void CertProvisioningDeviceInvalidator::OnInvalidationServiceSet(
    invalidation::InvalidationService* invalidation_service) {
  // This can only be called after Register() has been called, so the `topic_`
  // must be non-empty.
  DCHECK(!topic_.empty());

  // Reset any previously active `invalidation_handler` as it could be referring
  // to the previous `invalidation_service`.
  invalidation_handler_.reset();

  if (!invalidation_service) {
    return;
  }

  invalidation_handler_ =
      internal::CertProvisioningInvalidationHandler::BuildAndRegister(
          CertScope::kDevice, invalidation_service, topic_,
          on_invalidation_event_callback_);
  if (!invalidation_handler_) {
    LOG(ERROR) << "Failed to register for invalidation topic";
  }
}

}  // namespace ash::cert_provisioning
