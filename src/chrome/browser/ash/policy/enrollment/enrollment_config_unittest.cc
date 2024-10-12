// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ash/policy/enrollment/enrollment_config.h"

#include "ash/constants/ash_switches.h"
#include "base/check_deref.h"
#include "base/command_line.h"
#include "base/test/gtest_util.h"
#include "base/test/scoped_command_line.h"
#include "base/values.h"
#include "chrome/browser/ash/login/configuration_keys.h"
#include "chrome/browser/ash/login/login_pref_names.h"
#include "chrome/browser/ash/login/oobe_configuration.h"
#include "chrome/browser/ash/login/ui/fake_login_display_host.h"
#include "chrome/browser/ash/login/wizard_context.h"
#include "chrome/browser/ash/policy/enrollment/enrollment_test_helper.h"
#include "chrome/browser/ash/policy/server_backed_state/server_backed_device_state.h"
#include "chrome/browser/prefs/browser_prefs.h"
#include "chrome/common/pref_names.h"
#include "chromeos/ash/components/install_attributes/stub_install_attributes.h"
#include "chromeos/ash/components/system/fake_statistics_provider.h"
#include "chromeos/ash/components/system/statistics_provider.h"
#include "components/prefs/testing_pref_service.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace policy {

constexpr char kTestDomain[] = "example.com";

struct ZeroTouchParam {
  const char* enable_zero_touch_flag;
  EnrollmentConfig::AuthMechanism auth_mechanism;
  EnrollmentConfig::AuthMechanism auth_mechanism_after_oobe;

  ZeroTouchParam(const char* flag,
                 EnrollmentConfig::AuthMechanism auth,
                 EnrollmentConfig::AuthMechanism auth_after_oobe)
      : enable_zero_touch_flag(flag),
        auth_mechanism(auth),
        auth_mechanism_after_oobe(auth_after_oobe) {}
};

class EnrollmentConfigTest : public testing::Test {
 protected:
  EnrollmentConfigTest() {
    RegisterLocalState(local_state_.registry());
    statistics_provider_.SetMachineStatistic(ash::system::kSerialNumberKey,
                                             "fake-serial");
    statistics_provider_.SetMachineStatistic(ash::system::kHardwareClassKey,
                                             "fake-hardware");
  }

  EnrollmentConfig GetPrescribedConfig() {
    return EnrollmentConfig::GetPrescribedEnrollmentConfig(
        &local_state_, install_attributes_, &statistics_provider_,
        enrollment_test_helper_.oobe_configuration());
  }

  ash::system::ScopedFakeStatisticsProvider statistics_provider_;
  TestingPrefServiceSimple local_state_;
  ash::StubInstallAttributes install_attributes_;
  base::test::ScopedCommandLine command_line_;
  test::EnrollmentTestHelper enrollment_test_helper_{&command_line_,
                                                     &statistics_provider_};
  ash::FakeLoginDisplayHost fake_login_display_host_;
};

TEST_F(EnrollmentConfigTest, TokenEnrollmentModeWithNoTokenYieldsModeNone) {
  enrollment_test_helper_.SetUpFlexDevice();
  auto state_dict = base::Value::Dict().Set(
      kDeviceStateMode, kDeviceStateInitialModeTokenEnrollment);
  local_state_.SetDict(prefs::kServerBackedDeviceState, state_dict.Clone());

  const auto config = GetPrescribedConfig();

  EXPECT_EQ(config.mode, EnrollmentConfig::MODE_NONE);
  EXPECT_FALSE(config.should_enroll());
}

#if BUILDFLAG(GOOGLE_CHROME_BRANDING)
TEST_F(
    EnrollmentConfigTest,
    TokenEnrollmentModeWithTokenPresentYieldsEnrollmentConfigModeTokenEnrollment) {
  enrollment_test_helper_.SetUpFlexDevice();
  enrollment_test_helper_.SetUpEnrollmentTokenConfig();
  auto state_dict = base::Value::Dict().Set(
      kDeviceStateMode, kDeviceStateInitialModeTokenEnrollment);
  local_state_.SetDict(prefs::kServerBackedDeviceState, state_dict.Clone());

  const EnrollmentConfig config = GetPrescribedConfig();

  EXPECT_EQ(config.mode,
            EnrollmentConfig::MODE_ENROLLMENT_TOKEN_INITIAL_SERVER_FORCED);
  EXPECT_EQ(config.enrollment_token, test::kEnrollmentToken);
  EXPECT_TRUE(config.should_enroll());
  EXPECT_TRUE(config.is_forced());
  EXPECT_TRUE(config.is_mode_with_manual_fallback());
  EXPECT_TRUE(config.is_automatic_enrollment());
  EXPECT_FALSE(config.is_mode_oauth());
  EXPECT_EQ(EnrollmentConfig::GetManualFallbackMode(config.mode),
            EnrollmentConfig::MODE_ENROLLMENT_TOKEN_INITIAL_MANUAL_FALLBACK);
}
#endif  // BUILDFLAG(GOOGLE_CHROME_BRANDING)

class EnrollmentConfigZeroTouchTest
    : public EnrollmentConfigTest,
      public testing::WithParamInterface<ZeroTouchParam> {
 protected:
  void SetupZeroTouchCommandLineSwitch();
};

void EnrollmentConfigZeroTouchTest::SetupZeroTouchCommandLineSwitch() {
  const ZeroTouchParam& param = GetParam();
  if (param.enable_zero_touch_flag != nullptr) {
    base::CommandLine::ForCurrentProcess()->AppendSwitchASCII(
        ash::switches::kEnterpriseEnableZeroTouchEnrollment,
        param.enable_zero_touch_flag);
  }
}

// Test enrollment configuration based on device state with precedence.
TEST_P(EnrollmentConfigZeroTouchTest, GetPrescribedEnrollmentConfigDuringOOBE) {
  SetupZeroTouchCommandLineSwitch();

  // Default configuration is empty.
  {
    const auto config = GetPrescribedConfig();
    EXPECT_EQ(EnrollmentConfig::MODE_NONE, config.mode);
    EXPECT_TRUE(config.management_domain.empty());
    EXPECT_EQ(GetParam().auth_mechanism, config.auth_mechanism);
  }

  // Set signals in increasing order of precedence, check results.

  // OEM manifest: advertised enrollment.
  statistics_provider_.SetMachineFlag(ash::system::kOemIsEnterpriseManagedKey,
                                      true);
  {
    const auto config = GetPrescribedConfig();
    EXPECT_EQ(EnrollmentConfig::MODE_LOCAL_ADVERTISED, config.mode);
    EXPECT_TRUE(config.management_domain.empty());
    EXPECT_EQ(GetParam().auth_mechanism, config.auth_mechanism);
  }

  // Pref: advertised enrollment. The resulting |config| is indistinguishable
  // from the OEM manifest configuration, so clear the latter to at least
  // verify the pref configuration results in the expect behavior on its own.
  statistics_provider_.ClearMachineFlag(
      ash::system::kOemIsEnterpriseManagedKey);
  local_state_.SetBoolean(prefs::kDeviceEnrollmentAutoStart, true);
  {
    const auto config = GetPrescribedConfig();
    EXPECT_EQ(EnrollmentConfig::MODE_LOCAL_ADVERTISED, config.mode);
    EXPECT_TRUE(config.management_domain.empty());
    EXPECT_EQ(GetParam().auth_mechanism, config.auth_mechanism);
  }

  // Server-backed state: advertised enrollment.
  auto state_dict =
      base::Value::Dict()
          .Set(kDeviceStateMode, kDeviceStateRestoreModeReEnrollmentRequested)
          .Set(kDeviceStateManagementDomain, kTestDomain);
  local_state_.SetDict(prefs::kServerBackedDeviceState, state_dict.Clone());
  {
    const auto config = GetPrescribedConfig();
    EXPECT_EQ(EnrollmentConfig::MODE_SERVER_ADVERTISED, config.mode);
    EXPECT_EQ(kTestDomain, config.management_domain);
    EXPECT_EQ(GetParam().auth_mechanism, config.auth_mechanism);
  }

  // OEM manifest: forced enrollment.
  statistics_provider_.SetMachineFlag(ash::system::kOemIsEnterpriseManagedKey,
                                      true);
  statistics_provider_.SetMachineFlag(
      ash::system::kOemCanExitEnterpriseEnrollmentKey, false);
  {
    const auto config = GetPrescribedConfig();
    EXPECT_EQ(EnrollmentConfig::MODE_LOCAL_FORCED, config.mode);
    EXPECT_TRUE(config.management_domain.empty());
    EXPECT_EQ(GetParam().auth_mechanism, config.auth_mechanism);
  }

  // Pref: forced enrollment. The resulting |config| is indistinguishable from
  // the OEM manifest configuration, so clear the latter to at least verify the
  // pref configuration results in the expect behavior on its own.
  statistics_provider_.ClearMachineFlag(
      ash::system::kOemIsEnterpriseManagedKey);
  local_state_.SetBoolean(prefs::kDeviceEnrollmentCanExit, false);
  {
    const auto config = GetPrescribedConfig();
    EXPECT_EQ(EnrollmentConfig::MODE_LOCAL_FORCED, config.mode);
    EXPECT_TRUE(config.management_domain.empty());
    EXPECT_EQ(GetParam().auth_mechanism, config.auth_mechanism);
  }

  // Server-backed state: forced initial attestation-based enrollment.
  local_state_.SetDict(
      prefs::kServerBackedDeviceState,
      base::Value::Dict()
          .Set(kDeviceStateMode, kDeviceStateInitialModeEnrollmentZeroTouch)
          .Set(kDeviceStateManagementDomain, kTestDomain));
  {
    const auto config = GetPrescribedConfig();
    EXPECT_EQ(EnrollmentConfig::MODE_ATTESTATION_INITIAL_SERVER_FORCED,
              config.mode);
    EXPECT_EQ(kTestDomain, config.management_domain);
    EXPECT_EQ(EnrollmentConfig::AUTH_MECHANISM_ATTESTATION_PREFERRED,
              config.auth_mechanism);
  }

  // Server-backed state: forced attestation-based re-enrollment.
  local_state_.SetDict(
      prefs::kServerBackedDeviceState,
      base::Value::Dict()
          .Set(kDeviceStateMode, kDeviceStateRestoreModeReEnrollmentZeroTouch)
          .Set(kDeviceStateManagementDomain, kTestDomain));
  {
    const auto config = GetPrescribedConfig();
    EXPECT_EQ(EnrollmentConfig::MODE_ATTESTATION_SERVER_FORCED, config.mode);
    EXPECT_EQ(kTestDomain, config.management_domain);
    EXPECT_EQ(EnrollmentConfig::AUTH_MECHANISM_ATTESTATION_PREFERRED,
              config.auth_mechanism);
  }

  // Server-backed state: forced initial enrollment.
  local_state_.SetDict(
      prefs::kServerBackedDeviceState,
      base::Value::Dict()
          .Set(kDeviceStateMode, kDeviceStateInitialModeEnrollmentEnforced)
          .Set(kDeviceStateManagementDomain, kTestDomain));
  {
    const auto config = GetPrescribedConfig();
    EXPECT_EQ(EnrollmentConfig::MODE_INITIAL_SERVER_FORCED, config.mode);
    EXPECT_EQ(kTestDomain, config.management_domain);
    EXPECT_EQ(GetParam().auth_mechanism, config.auth_mechanism);
  }

  // Server-backed state: forced re-enrollment.
  local_state_.SetDict(
      prefs::kServerBackedDeviceState,
      base::Value::Dict()
          .Set(kDeviceStateMode, kDeviceStateRestoreModeReEnrollmentEnforced)
          .Set(kDeviceStateManagementDomain, kTestDomain));
  {
    const auto config = GetPrescribedConfig();
    EXPECT_EQ(EnrollmentConfig::MODE_SERVER_FORCED, config.mode);
    EXPECT_EQ(kTestDomain, config.management_domain);
    EXPECT_EQ(GetParam().auth_mechanism, config.auth_mechanism);
  }

  // OOBE config: rollback re-enrollment.
  CHECK_DEREF(fake_login_display_host_.GetWizardContext())
      .configuration.Set(ash::configuration::kRestoreAfterRollback, true);
  {
    const auto config = GetPrescribedConfig();
    EXPECT_EQ(EnrollmentConfig::MODE_ATTESTATION_ROLLBACK_FORCED, config.mode);
    EXPECT_TRUE(config.management_domain.empty());
    EXPECT_EQ(EnrollmentConfig::AUTH_MECHANISM_ATTESTATION_PREFERRED,
              config.auth_mechanism);
  }
}

// Test enrollment configuration after OOBE completed.
TEST_P(EnrollmentConfigZeroTouchTest, GetPrescribedEnrollmentConfigAfterOOBE) {
  SetupZeroTouchCommandLineSwitch();

  // If OOBE is complete, we may re-enroll to the domain configured in install
  // attributes. This is only enforced after detecting enrollment loss.
  local_state_.SetBoolean(ash::prefs::kOobeComplete, true);
  {
    const auto config = GetPrescribedConfig();
    EXPECT_EQ(EnrollmentConfig::MODE_NONE, config.mode);
    EXPECT_TRUE(config.management_domain.empty());
    EXPECT_EQ(GetParam().auth_mechanism_after_oobe, config.auth_mechanism);
  }

  // Advertised enrollment gets ignored.
  local_state_.SetBoolean(prefs::kDeviceEnrollmentAutoStart, true);
  statistics_provider_.SetMachineFlag(ash::system::kOemIsEnterpriseManagedKey,
                                      true);
  {
    const auto config = GetPrescribedConfig();
    EXPECT_EQ(EnrollmentConfig::MODE_NONE, config.mode);
    EXPECT_TRUE(config.management_domain.empty());
    EXPECT_EQ(GetParam().auth_mechanism_after_oobe, config.auth_mechanism);
  }

  // If the device is enterprise-managed, the management domain gets pulled from
  // install attributes.
  install_attributes_.SetCloudManaged(kTestDomain, "fake-id");
  {
    const auto config = GetPrescribedConfig();
    EXPECT_EQ(EnrollmentConfig::MODE_NONE, config.mode);
    EXPECT_EQ(kTestDomain, config.management_domain);
    EXPECT_EQ(GetParam().auth_mechanism_after_oobe, config.auth_mechanism);
  }

  // If enrollment recovery is on, this is signaled in |config.mode|.
  local_state_.SetBoolean(prefs::kEnrollmentRecoveryRequired, true);
  {
    const auto config = GetPrescribedConfig();
    EXPECT_EQ(EnrollmentConfig::MODE_RECOVERY, config.mode);
    EXPECT_EQ(kTestDomain, config.management_domain);
    EXPECT_EQ(GetParam().auth_mechanism_after_oobe, config.auth_mechanism);
  }
}

INSTANTIATE_TEST_SUITE_P(
    ZeroTouchFlag,
    EnrollmentConfigZeroTouchTest,
    ::testing::Values(
        ZeroTouchParam(nullptr,  // No flag set.
                       EnrollmentConfig::AUTH_MECHANISM_INTERACTIVE,
                       EnrollmentConfig::AUTH_MECHANISM_INTERACTIVE),
        ZeroTouchParam("",  // Flag set without a set value.
                       EnrollmentConfig::AUTH_MECHANISM_ATTESTATION_PREFERRED,
                       EnrollmentConfig::AUTH_MECHANISM_INTERACTIVE),
        ZeroTouchParam("forced",
                       EnrollmentConfig::AUTH_MECHANISM_ATTESTATION,
                       EnrollmentConfig::AUTH_MECHANISM_ATTESTATION)));

}  // namespace policy
