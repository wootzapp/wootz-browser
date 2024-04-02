// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/enterprise/connectors/common.h"

#include "base/notreached.h"
#include "base/ranges/algorithm.h"
#include "base/strings/utf_string_conversions.h"
#include "build/chromeos_buildflags.h"
#include "chrome/browser/enterprise/connectors/analysis/content_analysis_delegate_base.h"
#include "chrome/browser/enterprise/connectors/analysis/content_analysis_dialog.h"
#include "chrome/browser/enterprise/connectors/analysis/content_analysis_downloads_delegate.h"
#include "chrome/browser/enterprise/connectors/connectors_prefs.h"
#include "chrome/browser/enterprise/connectors/connectors_service.h"
#include "chrome/browser/enterprise/util/affiliation.h"
#include "chrome/browser/policy/dm_token_utils.h"
#include "chrome/browser/profiles/profile.h"
#include "components/download/public/common/download_danger_type.h"
#include "components/download/public/common/download_item.h"
#include "components/enterprise/browser/controller/browser_dm_token_storage.h"
#include "ui/base/l10n/l10n_util.h"

#if BUILDFLAG(IS_CHROMEOS_ASH)
#include "chrome/browser/ash/profiles/profile_helper.h"
#include "components/user_manager/user.h"
#endif

#if BUILDFLAG(IS_CHROMEOS_LACROS)
#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "components/policy/core/common/policy_loader_lacros.h"
#endif

using safe_browsing::BinaryUploadService;

namespace enterprise_connectors {

RequestHandlerResult::RequestHandlerResult() = default;
RequestHandlerResult::~RequestHandlerResult() = default;
RequestHandlerResult::RequestHandlerResult(RequestHandlerResult&&) = default;
RequestHandlerResult& RequestHandlerResult::operator=(RequestHandlerResult&&) =
    default;
RequestHandlerResult::RequestHandlerResult(const RequestHandlerResult&) =
    default;
RequestHandlerResult& RequestHandlerResult::operator=(
    const RequestHandlerResult&) = default;

namespace {

bool ContentAnalysisActionAllowsDataUse(TriggeredRule::Action action) {
  switch (action) {
    case TriggeredRule::ACTION_UNSPECIFIED:
    case TriggeredRule::REPORT_ONLY:
      return true;
    case TriggeredRule::WARN:
    case TriggeredRule::BLOCK:
      return false;
  }
}

ContentAnalysisAcknowledgement::FinalAction RuleActionToAckAction(
    TriggeredRule::Action action) {
  switch (action) {
    case TriggeredRule::ACTION_UNSPECIFIED:
      return ContentAnalysisAcknowledgement::ACTION_UNSPECIFIED;
    case TriggeredRule::REPORT_ONLY:
      return ContentAnalysisAcknowledgement::REPORT_ONLY;
    case TriggeredRule::WARN:
      return ContentAnalysisAcknowledgement::WARN;
    case TriggeredRule::BLOCK:
      return ContentAnalysisAcknowledgement::BLOCK;
  }
}

}  // namespace

bool ResultShouldAllowDataUse(const AnalysisSettings& settings,
                              BinaryUploadService::Result upload_result) {
  bool default_action_allow_data_use =
      settings.default_action == DefaultAction::kAllow;

  // Keep this implemented as a switch instead of a simpler if statement so that
  // new values added to BinaryUploadService::Result cause a compiler error.
  switch (upload_result) {
    case BinaryUploadService::Result::SUCCESS:
    // UNAUTHORIZED allows data usage since it's a result only obtained if the
    // browser is not authorized to perform deep scanning. It does not make
    // sense to block data in this situation since no actual scanning of the
    // data was performed, so it's allowed.
    case BinaryUploadService::Result::UNAUTHORIZED:
      return true;

    case BinaryUploadService::Result::UPLOAD_FAILURE:
    case BinaryUploadService::Result::TIMEOUT:
    case BinaryUploadService::Result::FAILED_TO_GET_TOKEN:
    case BinaryUploadService::Result::TOO_MANY_REQUESTS:
    case BinaryUploadService::Result::UNKNOWN:
      DVLOG(1) << __func__
               << ": handled by fail-closed settings, "
                  "default_action_allow_data_use="
               << default_action_allow_data_use;
      return default_action_allow_data_use;

    case BinaryUploadService::Result::FILE_TOO_LARGE:
      return !settings.block_large_files;

    case BinaryUploadService::Result::FILE_ENCRYPTED:
      return !settings.block_password_protected_files;
  }
}

RequestHandlerResult CalculateRequestHandlerResult(
    const AnalysisSettings& settings,
    BinaryUploadService::Result upload_result,
    const ContentAnalysisResponse& response) {
  std::string tag;
  auto action = GetHighestPrecedenceAction(response, &tag);

  bool file_complies = ResultShouldAllowDataUse(settings, upload_result) &&
                       ContentAnalysisActionAllowsDataUse(action);

  RequestHandlerResult result;
  result.complies = file_complies;
  result.request_token = response.request_token();
  result.tag = tag;

  if (file_complies) {
    result.final_result = FinalContentAnalysisResult::SUCCESS;
    return result;
  }

  // If file is non-compliant, map it to the specific case.
  if (ResultIsFailClosed(upload_result)) {
    DVLOG(1) << __func__ << ": result mapped to fail-closed.";
    result.final_result = FinalContentAnalysisResult::FAIL_CLOSED;
  } else if (upload_result == BinaryUploadService::Result::FILE_TOO_LARGE) {
    result.final_result = FinalContentAnalysisResult::LARGE_FILES;
  } else if (upload_result == BinaryUploadService::Result::FILE_ENCRYPTED) {
    result.final_result = FinalContentAnalysisResult::ENCRYPTED_FILES;
  } else if (action == TriggeredRule::WARN) {
    result.final_result = FinalContentAnalysisResult::WARNING;
  } else {
    result.final_result = FinalContentAnalysisResult::FAILURE;
  }

  for (const auto& response_result : response.results()) {
    if (!response_result.has_status() ||
        response_result.status() != ContentAnalysisResponse::Result::SUCCESS) {
      continue;
    }
    for (const auto& rule : response_result.triggered_rules()) {
      // Ensures that lower precedence actions custom messages are skipped. The
      // message shown is arbitrary for rules with the same precedence.
      if (rule.action() == action && rule.has_custom_rule_message()) {
        result.custom_rule_message = rule.custom_rule_message();
      }
    }
  }
  return result;
}

std::u16string GetCustomRuleString(
    const ContentAnalysisResponse::Result::TriggeredRule::CustomRuleMessage&
        custom_rule_message) {
  std::u16string custom_message;
  for (const auto& custom_segment : custom_rule_message.message_segments()) {
    base::StrAppend(&custom_message,
                    {base::UTF8ToUTF16(custom_segment.text())});
  }
  return custom_message;
}

std::vector<std::pair<gfx::Range, GURL>> GetCustomRuleStyles(
    const ContentAnalysisResponse::Result::TriggeredRule::CustomRuleMessage&
        custom_rule_message,
    size_t offset) {
  std::vector<std::pair<gfx::Range, GURL>> linked_ranges;
  for (const auto& custom_segment : custom_rule_message.message_segments()) {
    if (custom_segment.has_link()) {
      GURL url(custom_segment.link());
      if (url.is_valid()) {
        linked_ranges.emplace_back(
            gfx::Range(offset, offset + custom_segment.text().length()), url);
      }
    }
    offset += custom_segment.text().length();
  }
  return linked_ranges;
}

safe_browsing::EventResult CalculateEventResult(
    const AnalysisSettings& settings,
    bool allowed_by_scan_result,
    bool should_warn) {
  bool wait_for_verdict =
      settings.block_until_verdict == BlockUntilVerdict::kBlock;
  return (allowed_by_scan_result || !wait_for_verdict)
             ? safe_browsing::EventResult::ALLOWED
             : (should_warn ? safe_browsing::EventResult::WARNED
                            : safe_browsing::EventResult::BLOCKED);
}

ContentAnalysisAcknowledgement::FinalAction GetAckFinalAction(
    const ContentAnalysisResponse& response) {
  auto final_action = ContentAnalysisAcknowledgement::ALLOW;
  for (const auto& result : response.results()) {
    if (!result.has_status() ||
        result.status() != ContentAnalysisResponse::Result::SUCCESS) {
      continue;
    }

    for (const auto& rule : result.triggered_rules()) {
      final_action = GetHighestPrecedenceAction(
          final_action, RuleActionToAckAction(rule.action()));
    }
  }

  return final_action;
}

ReportingSettings::ReportingSettings() = default;
ReportingSettings::ReportingSettings(GURL url,
                                     const std::string& dm_token,
                                     bool per_profile)
    : reporting_url(url), dm_token(dm_token), per_profile(per_profile) {}
ReportingSettings::ReportingSettings(ReportingSettings&&) = default;
ReportingSettings::ReportingSettings(const ReportingSettings&) = default;
ReportingSettings& ReportingSettings::operator=(ReportingSettings&&) = default;
ReportingSettings::~ReportingSettings() = default;

const char* ConnectorPref(AnalysisConnector connector) {
  switch (connector) {
    case AnalysisConnector::BULK_DATA_ENTRY:
      return kOnBulkDataEntryPref;
    case AnalysisConnector::FILE_DOWNLOADED:
      return kOnFileDownloadedPref;
    case AnalysisConnector::FILE_ATTACHED:
      return kOnFileAttachedPref;
    case AnalysisConnector::PRINT:
      return kOnPrintPref;
    case AnalysisConnector::FILE_TRANSFER:
#if BUILDFLAG(IS_CHROMEOS)
      return kOnFileTransferPref;
#endif
    case AnalysisConnector::ANALYSIS_CONNECTOR_UNSPECIFIED:
      NOTREACHED() << "Using unspecified analysis connector";
      return "";
  }
}

const char* ConnectorPref(ReportingConnector connector) {
  switch (connector) {
    case ReportingConnector::SECURITY_EVENT:
      return kOnSecurityEventPref;
  }
}

const char* ConnectorScopePref(AnalysisConnector connector) {
  switch (connector) {
    case AnalysisConnector::BULK_DATA_ENTRY:
      return kOnBulkDataEntryScopePref;
    case AnalysisConnector::FILE_DOWNLOADED:
      return kOnFileDownloadedScopePref;
    case AnalysisConnector::FILE_ATTACHED:
      return kOnFileAttachedScopePref;
    case AnalysisConnector::PRINT:
      return kOnPrintScopePref;
    case AnalysisConnector::FILE_TRANSFER:
#if BUILDFLAG(IS_CHROMEOS)
      return kOnFileTransferScopePref;
#endif
    case AnalysisConnector::ANALYSIS_CONNECTOR_UNSPECIFIED:
      NOTREACHED() << "Using unspecified analysis connector";
      return "";
  }
}

const char* ConnectorScopePref(ReportingConnector connector) {
  switch (connector) {
    case ReportingConnector::SECURITY_EVENT:
      return kOnSecurityEventScopePref;
  }
}

TriggeredRule::Action GetHighestPrecedenceAction(
    const ContentAnalysisResponse& response,
    std::string* tag) {
  auto action = TriggeredRule::ACTION_UNSPECIFIED;

  for (const auto& result : response.results()) {
    if (!result.has_status() ||
        result.status() != ContentAnalysisResponse::Result::SUCCESS) {
      continue;
    }

    for (const auto& rule : result.triggered_rules()) {
      auto higher_precedence_action =
          GetHighestPrecedenceAction(action, rule.action());
      if (higher_precedence_action != action && tag != nullptr) {
        *tag = result.tag();
      }
      action = higher_precedence_action;
    }
  }
  return action;
}

TriggeredRule::Action GetHighestPrecedenceAction(
    const TriggeredRule::Action& action_1,
    const TriggeredRule::Action& action_2) {
  // Don't use the enum's int values to determine precedence since that
  // may introduce bugs for new actions later.
  //
  // The current precedence is BLOCK > WARN > REPORT_ONLY > UNSPECIFIED
  if (action_1 == TriggeredRule::BLOCK || action_2 == TriggeredRule::BLOCK) {
    return TriggeredRule::BLOCK;
  }
  if (action_1 == TriggeredRule::WARN || action_2 == TriggeredRule::WARN) {
    return TriggeredRule::WARN;
  }
  if (action_1 == TriggeredRule::REPORT_ONLY ||
      action_2 == TriggeredRule::REPORT_ONLY) {
    return TriggeredRule::REPORT_ONLY;
  }
  if (action_1 == TriggeredRule::ACTION_UNSPECIFIED ||
      action_2 == TriggeredRule::ACTION_UNSPECIFIED) {
    return TriggeredRule::ACTION_UNSPECIFIED;
  }
  NOTREACHED();
  return TriggeredRule::ACTION_UNSPECIFIED;
}

ContentAnalysisAcknowledgement::FinalAction GetHighestPrecedenceAction(
    const ContentAnalysisAcknowledgement::FinalAction& action_1,
    const ContentAnalysisAcknowledgement::FinalAction& action_2) {
  // Don't use the enum's int values to determine precedence since that
  // may introduce bugs for new actions later.
  //
  // The current precedence is BLOCK > WARN > REPORT_ONLY > ALLOW > UNSPECIFIED
  if (action_1 == ContentAnalysisAcknowledgement::BLOCK ||
      action_2 == ContentAnalysisAcknowledgement::BLOCK) {
    return ContentAnalysisAcknowledgement::BLOCK;
  }
  if (action_1 == ContentAnalysisAcknowledgement::WARN ||
      action_2 == ContentAnalysisAcknowledgement::WARN) {
    return ContentAnalysisAcknowledgement::WARN;
  }
  if (action_1 == ContentAnalysisAcknowledgement::REPORT_ONLY ||
      action_2 == ContentAnalysisAcknowledgement::REPORT_ONLY) {
    return ContentAnalysisAcknowledgement::REPORT_ONLY;
  }
  if (action_1 == ContentAnalysisAcknowledgement::ALLOW ||
      action_2 == ContentAnalysisAcknowledgement::ALLOW) {
    return ContentAnalysisAcknowledgement::ALLOW;
  }
  if (action_1 == ContentAnalysisAcknowledgement::ACTION_UNSPECIFIED ||
      action_2 == ContentAnalysisAcknowledgement::ACTION_UNSPECIFIED) {
    return ContentAnalysisAcknowledgement::ACTION_UNSPECIFIED;
  }
  NOTREACHED();
  return ContentAnalysisAcknowledgement::ACTION_UNSPECIFIED;
}

FileMetadata::FileMetadata(const std::string& filename,
                           const std::string& sha256,
                           const std::string& mime_type,
                           int64_t size,
                           const ContentAnalysisResponse& scan_response)
    : filename(filename),
      sha256(sha256),
      mime_type(mime_type),
      size(size),
      scan_response(scan_response) {}
FileMetadata::FileMetadata(FileMetadata&&) = default;
FileMetadata::FileMetadata(const FileMetadata&) = default;
FileMetadata& FileMetadata::operator=(const FileMetadata&) = default;
FileMetadata::~FileMetadata() = default;

const char ScanResult::kKey[] = "enterprise_connectors.scan_result_key";
ScanResult::ScanResult() = default;
ScanResult::ScanResult(FileMetadata metadata) {
  file_metadata.push_back(std::move(metadata));
}
ScanResult::~ScanResult() = default;

const char SavePackageScanningData::kKey[] =
    "enterprise_connectors.save_package_scanning_key";
SavePackageScanningData::SavePackageScanningData(
    content::SavePackageAllowedCallback callback)
    : callback(std::move(callback)) {}
SavePackageScanningData::~SavePackageScanningData() = default;

void RunSavePackageScanningCallback(download::DownloadItem* item,
                                    bool allowed) {
  DCHECK(item);

  auto* data = static_cast<SavePackageScanningData*>(
      item->GetUserData(SavePackageScanningData::kKey));
  if (data && !data->callback.is_null())
    std::move(data->callback).Run(allowed);
}

bool ContainsMalwareVerdict(const ContentAnalysisResponse& response) {
  return base::ranges::any_of(response.results(), [](const auto& result) {
    return result.tag() == kMalwareTag && !result.triggered_rules().empty();
  });
}

bool IncludeDeviceInfo(Profile* profile, bool per_profile) {
#if BUILDFLAG(IS_CHROMEOS_ASH)
  const user_manager::User* user =
      ash::ProfileHelper::Get()->GetUserByProfile(profile);
  return user && user->IsAffiliated();
#elif BUILDFLAG(IS_CHROMEOS_LACROS)
  return policy::PolicyLoaderLacros::IsMainUserAffiliated();
#else
  // A browser managed through the device can send device info.
  if (!per_profile) {
    return true;
  }

  // An unmanaged browser shouldn't share its device info for privacy reasons.
  if (!policy::GetDMToken(profile).is_valid()) {
    return false;
  }

  // A managed device can share its info with the profile if they are
  // affiliated.
  return chrome::enterprise_util::IsProfileAffiliated(profile);
#endif
}

ContentAnalysisResponse::Result::TriggeredRule::CustomRuleMessage
CreateSampleCustomRuleMessage(const std::u16string& msg,
                              const std::string& url) {
  ContentAnalysisResponse::Result::TriggeredRule::CustomRuleMessage
      custom_message;
  auto* custom_segment = custom_message.add_message_segments();
  custom_segment->set_text(base::UTF16ToUTF8(msg));
  custom_segment->set_link(url);
  return custom_message;
}

std::optional<ContentAnalysisResponse::Result::TriggeredRule::CustomRuleMessage>
GetDownloadsCustomRuleMessage(const download::DownloadItem* download_item,
                              download::DownloadDangerType danger_type) {
  if (!download_item) {
    return std::nullopt;
  }

  // Custom rule message is currently only present for either warning or block
  // danger types.
  TriggeredRule::Action current_action;
  if (danger_type == download::DOWNLOAD_DANGER_TYPE_SENSITIVE_CONTENT_WARNING) {
    current_action = TriggeredRule::WARN;
  } else if (danger_type ==
             download::DOWNLOAD_DANGER_TYPE_SENSITIVE_CONTENT_BLOCK) {
    current_action = TriggeredRule::BLOCK;
  } else {
    return std::nullopt;
  }

  enterprise_connectors::ScanResult* scan_result =
      static_cast<enterprise_connectors::ScanResult*>(
          download_item->GetUserData(enterprise_connectors::ScanResult::kKey));
  if (!scan_result) {
    return std::nullopt;
  }
  for (const auto& metadata : scan_result->file_metadata) {
    for (const auto& result : metadata.scan_response.results()) {
      for (const auto& rule : result.triggered_rules()) {
        if (rule.action() == current_action && rule.has_custom_rule_message()) {
          return rule.custom_rule_message();
        }
      }
    }
  }
  return std::nullopt;
}

bool ShouldPromptReviewForDownload(
    Profile* profile,
    const download::DownloadItem* download_item) {
  // Review dialog only appears if custom UI has been set by the admin or custom
  // rule message present in download item.
  if (!download_item) {
    return false;
  }
  download::DownloadDangerType danger_type = download_item->GetDangerType();
  if (danger_type == download::DOWNLOAD_DANGER_TYPE_SENSITIVE_CONTENT_WARNING ||
      danger_type == download::DOWNLOAD_DANGER_TYPE_SENSITIVE_CONTENT_BLOCK) {
    return ConnectorsServiceFactory::GetForBrowserContext(profile)
               ->HasExtraUiToDisplay(AnalysisConnector::FILE_DOWNLOADED,
                                     kDlpTag) ||
           GetDownloadsCustomRuleMessage(download_item, danger_type);
  } else if (danger_type == download::DOWNLOAD_DANGER_TYPE_DANGEROUS_FILE ||
             danger_type == download::DOWNLOAD_DANGER_TYPE_DANGEROUS_URL ||
             danger_type == download::DOWNLOAD_DANGER_TYPE_DANGEROUS_CONTENT) {
    return ConnectorsServiceFactory::GetForBrowserContext(profile)
        ->HasExtraUiToDisplay(AnalysisConnector::FILE_DOWNLOADED, kMalwareTag);
  }
  return false;
}

void ShowDownloadReviewDialog(const std::u16string& filename,
                              Profile* profile,
                              download::DownloadItem* download_item,
                              content::WebContents* web_contents,
                              base::OnceClosure keep_closure,
                              base::OnceClosure discard_closure) {
  auto state = FinalContentAnalysisResult::FAILURE;
  download::DownloadDangerType danger_type = download_item->GetDangerType();

  if (danger_type == download::DOWNLOAD_DANGER_TYPE_SENSITIVE_CONTENT_WARNING) {
    state = FinalContentAnalysisResult::WARNING;
  }

  const char* tag =
      (danger_type ==
                   download::DOWNLOAD_DANGER_TYPE_SENSITIVE_CONTENT_WARNING ||
               danger_type ==
                   download::DOWNLOAD_DANGER_TYPE_SENSITIVE_CONTENT_BLOCK
           ? kDlpTag
           : kMalwareTag);

  auto* connectors_service =
      ConnectorsServiceFactory::GetForBrowserContext(profile);

  std::u16string custom_message =
      connectors_service
          ->GetCustomMessage(AnalysisConnector::FILE_DOWNLOADED, tag)
          .value_or(u"");
  GURL learn_more_url =
      connectors_service
          ->GetLearnMoreUrl(AnalysisConnector::FILE_DOWNLOADED, tag)
          .value_or(GURL());

  bool bypass_justification_required =
      connectors_service->GetBypassJustificationRequired(
          AnalysisConnector::FILE_DOWNLOADED, tag);

  // This dialog opens itself, and is thereafter owned by constrained window
  // code.
  new ContentAnalysisDialog(
      std::make_unique<ContentAnalysisDownloadsDelegate>(
          filename, custom_message, learn_more_url,
          bypass_justification_required, std::move(keep_closure),
          std::move(discard_closure), download_item,
          GetDownloadsCustomRuleMessage(download_item, danger_type)
              .value_or(ContentAnalysisResponse::Result::TriggeredRule::
                            CustomRuleMessage())),
      true,  // Downloads are always cloud-based for now.
      web_contents, safe_browsing::DeepScanAccessPoint::DOWNLOAD,
      /* file_count */ 1, state, download_item);
}

bool CloudResultIsFailure(BinaryUploadService::Result result) {
  return result != BinaryUploadService::Result::SUCCESS;
}

bool LocalResultIsFailure(BinaryUploadService::Result result) {
  return result != BinaryUploadService::Result::SUCCESS &&
         result != BinaryUploadService::Result::FILE_TOO_LARGE &&
         result != BinaryUploadService::Result::FILE_ENCRYPTED;
}

bool ResultIsFailClosed(BinaryUploadService::Result result) {
  return result == BinaryUploadService::Result::UPLOAD_FAILURE ||
         result == BinaryUploadService::Result::TIMEOUT ||
         result == BinaryUploadService::Result::FAILED_TO_GET_TOKEN ||
         result == BinaryUploadService::Result::TOO_MANY_REQUESTS ||
         result == BinaryUploadService::Result::UNKNOWN;
}

#if BUILDFLAG(IS_CHROMEOS_LACROS)
Profile* GetMainProfileLacros() {
  ProfileManager* profile_manager = g_browser_process->profile_manager();
  if (!profile_manager)
    return nullptr;
  auto profiles = g_browser_process->profile_manager()->GetLoadedProfiles();
  const auto main_it = base::ranges::find_if(profiles, &Profile::IsMainProfile);
  if (main_it == profiles.end())
    return nullptr;
  return *main_it;
}
#endif

}  // namespace enterprise_connectors
