// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/manta/sparky/sparky_util.h"

#include "base/test/task_environment.h"
#include "components/manta/proto/sparky.pb.h"
#include "components/manta/sparky/sparky_delegate.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace manta {

class SparkyUtilTest : public testing::Test {
 public:
  SparkyUtilTest() = default;

  SparkyUtilTest(const SparkyUtilTest&) = delete;
  SparkyUtilTest& operator=(const SparkyUtilTest&) = delete;

  ~SparkyUtilTest() override = default;

 protected:
  base::test::TaskEnvironment task_environment_;

  bool ContainsSetting(
      const google::protobuf::RepeatedPtrField<proto::Setting>& repeatedField,
      SettingsData* settings_data) {
    for (const auto& proto_setting : repeatedField) {
      if (proto_setting.settings_id() == settings_data->pref_name &&
          proto_setting.has_value()) {
        if (proto_setting.type() == proto::SETTING_TYPE_BOOL) {
          return (proto_setting.value().has_bool_val() &&
                  proto_setting.value().bool_val() ==
                      settings_data->value->GetBool());
        } else if (proto_setting.type() == proto::SETTING_TYPE_STRING) {
          return (proto_setting.value().has_text_val() &&
                  (proto_setting.value().text_val() ==
                   settings_data->value->GetString()));
        } else if (proto_setting.type() == proto::SETTING_TYPE_INTEGER) {
          return (proto_setting.value().has_int_val() &&
                  (proto_setting.value().int_val() ==
                   settings_data->value->GetInt()));
        } else if (proto_setting.type() == proto::SETTING_TYPE_DOUBLE) {
          EXPECT_DOUBLE_EQ(proto_setting.value().double_val(),
                           settings_data->value->GetDouble());
          return true;
        }
      }
    }
    return false;  // Did not find the setting.
  }
};

TEST_F(SparkyUtilTest, AddSettingsProto) {
  auto current_prefs = SparkyDelegate::SettingsDataList();
  current_prefs["ash.dark_mode.enabled"] = std::make_unique<SettingsData>(
      "ash.dark_mode.enabled", PrefType::kBoolean,
      std::make_optional<base::Value>(true));
  current_prefs["string_pref"] = std::make_unique<SettingsData>(
      "string_pref", PrefType::kString,
      std::make_optional<base::Value>("my string"));
  current_prefs["int_pref"] = std::make_unique<SettingsData>(
      "int_pref", PrefType::kInt, std::make_optional<base::Value>(1));
  current_prefs["ash.night_light.enabled"] = std::make_unique<SettingsData>(
      "ash.night_light.enabled", PrefType::kBoolean,
      std::make_optional<base::Value>(false));
  current_prefs["ash.night_light.color_temperature"] =
      std::make_unique<SettingsData>("ash.night_light.color_temperature",
                                     PrefType::kDouble,
                                     std::make_optional<base::Value>(0.1));
  proto::SparkyContextData sparky_context_data;
  manta::proto::SettingsData* settings_data =
      sparky_context_data.mutable_settings_data();
  AddSettingsProto(current_prefs, settings_data);
  auto settings = settings_data->setting();
  ASSERT_EQ(settings_data->setting_size(), 5);
  ASSERT_TRUE(
      ContainsSetting(settings, current_prefs["ash.dark_mode.enabled"].get()));
  ASSERT_TRUE(ContainsSetting(settings, current_prefs["string_pref"].get()));
  ASSERT_TRUE(ContainsSetting(settings,
                              current_prefs["ash.night_light.enabled"].get()));
  ASSERT_TRUE(ContainsSetting(
      settings, current_prefs["ash.night_light.color_temperature"].get()));
}

}  // namespace manta
