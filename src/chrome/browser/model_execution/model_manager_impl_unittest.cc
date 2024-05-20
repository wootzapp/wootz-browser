// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/model_execution/model_manager_impl.h"

#include "base/test/mock_callback.h"
#include "chrome/browser/optimization_guide/mock_optimization_guide_keyed_service.h"
#include "chrome/browser/optimization_guide/optimization_guide_keyed_service_factory.h"
#include "chrome/test/base/chrome_render_view_host_test_harness.h"
#include "components/optimization_guide/core/optimization_guide_switches.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/blink/public/mojom/model_execution/model_manager.mojom.h"

using ::testing::AtMost;
using ::testing::NiceMock;

class ModelManagerImplTest : public ChromeRenderViewHostTestHarness {
 public:
  void SetUp() override {
    ChromeRenderViewHostTestHarness::SetUp();

    // Setting up MockOptimizationGuideKeyedService.
    OptimizationGuideKeyedServiceFactory::GetInstance()
        ->SetTestingFactoryAndUse(
            profile(),
            base::BindRepeating([](content::BrowserContext* context)
                                    -> std::unique_ptr<KeyedService> {
              return std::make_unique<
                  NiceMock<MockOptimizationGuideKeyedService>>();
            }));
  }

  void TearDown() override { ChromeRenderViewHostTestHarness::TearDown(); }
};

// Tests that involve invalid on-device model file paths should not crash when
// the associated RFH is destroyed.
TEST_F(ModelManagerImplTest, NoUAFWithInvalidOnDeviceModelPath) {
  auto* command_line = base::CommandLine::ForCurrentProcess();
  command_line->AppendSwitchASCII(
      optimization_guide::switches::kOnDeviceModelExecutionOverride,
      "invalid-on-device-model-file-path");

  base::MockCallback<
      blink::mojom::ModelManager::CanCreateGenericSessionCallback>
      callback;
  EXPECT_CALL(callback, Run(testing::_))
      .Times(AtMost(1))
      .WillOnce(
          testing::Invoke([&](bool can_create) { EXPECT_FALSE(can_create); }));

  ModelManagerImpl* model_manager =
      ModelManagerImpl::GetOrCreateForCurrentDocument(main_rfh());
  model_manager->CanCreateGenericSession(callback.Get());

  // The callback may still be pending, delete the WebContents and destroy the
  // associated RFH, which should not result in a UAF.
  DeleteContents();

  task_environment()->RunUntilIdle();
}
