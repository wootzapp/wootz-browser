// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/page_content_annotations/core/page_content_annotations_service.h"

#include "base/test/gmock_callback_support.h"
#include "base/test/gmock_move_support.h"
#include "base/test/metrics/histogram_tester.h"
#include "base/test/scoped_feature_list.h"
#include "base/test/task_environment.h"
#include "components/history/core/browser/history_service.h"
#include "components/optimization_guide/core/optimization_guide_features.h"
#include "components/optimization_guide/core/test_optimization_guide_decider.h"
#include "components/optimization_guide/core/test_optimization_guide_model_provider.h"
#include "components/page_content_annotations/core/page_content_annotations_features.h"
#include "components/page_content_annotations/core/test_page_content_annotator.h"
#include "components/search_engines/template_url_service.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace page_content_annotations {

namespace {

using ::testing::_;

const TemplateURLService::Initializer kTemplateURLData[] = {
    {"default-engine.com", "http://default-engine.com/search?q={searchTerms}",
     "Default"},
    {"non-default-engine.com", "http://non-default-engine.com?q={searchTerms}",
     "Not Default"},
};

class MockHistoryService : public history::HistoryService {
 public:
  MockHistoryService() = default;

  MOCK_METHOD(void,
              AddContentModelAnnotationsForVisit,
              (const history::VisitContentModelAnnotations&, history::VisitID),
              (override));

  MOCK_METHOD(void,
              AddSearchMetadataForVisit,
              (const GURL&, const std::u16string&, history::VisitID),
              (override));

  MOCK_METHOD(void,
              AddPageMetadataForVisit,
              (const std::string&, history::VisitID),
              (override));

  MOCK_METHOD(void,
              SetHasUrlKeyedImageForVisit,
              (bool, history::VisitID),
              (override));
};

class FakeOptimizationGuideDecider
    : public optimization_guide::TestOptimizationGuideDecider {
 public:
  void RegisterOptimizationTypes(
      const std::vector<optimization_guide::proto::OptimizationType>&
          optimization_types) override {
    registered_optimization_types_ = optimization_types;
  }

  std::vector<optimization_guide::proto::OptimizationType>
  registered_optimization_types() {
    return registered_optimization_types_;
  }

  void CanApplyOptimization(
      const GURL& url,
      optimization_guide::proto::OptimizationType optimization_type,
      optimization_guide::OptimizationGuideDecisionCallback callback) override {
    std::string url_spec = url.spec();
    if (optimization_type == optimization_guide::proto::PAGE_ENTITIES &&
        url == GURL("http://hasmetadata.com/")) {
      optimization_guide::proto::PageEntitiesMetadata page_entities_metadata;
      page_entities_metadata.set_alternative_title("alternative title");

      optimization_guide::OptimizationMetadata metadata;
      metadata.SetAnyMetadataForTesting(page_entities_metadata);
      std::move(callback).Run(
          optimization_guide::OptimizationGuideDecision::kTrue, metadata);
      return;
    }
    if (optimization_type == optimization_guide::proto::SALIENT_IMAGE &&
        url == GURL("http://hasimageurl.com")) {
      optimization_guide::proto::SalientImageMetadata salient_image_metadata;
      salient_image_metadata.add_thumbnails()->set_image_url(
          "http://gstatic.com/image");

      optimization_guide::OptimizationMetadata metadata;
      metadata.SetAnyMetadataForTesting(salient_image_metadata);
      std::move(callback).Run(
          optimization_guide::OptimizationGuideDecision::kTrue, metadata);
      return;
    }
    if (url == GURL("http://wrongmetadata.com/")) {
      optimization_guide::OptimizationMetadata metadata;
      optimization_guide::proto::Entity entity;
      metadata.SetAnyMetadataForTesting(entity);
      std::move(callback).Run(
          optimization_guide::OptimizationGuideDecision::kTrue, metadata);
      return;
    }
    std::move(callback).Run(
        optimization_guide::OptimizationGuideDecision::kFalse, {});
  }

  optimization_guide::OptimizationGuideDecision CanApplyOptimization(
      const GURL& url,
      optimization_guide::proto::OptimizationType optimization_type,
      optimization_guide::OptimizationMetadata* optimization_metadata)
      override {
    NOTREACHED();
    return optimization_guide::OptimizationGuideDecision::kFalse;
  }

 private:
  std::vector<optimization_guide::proto::OptimizationType>
      registered_optimization_types_;
};

}  // namespace

class PageContentAnnotationsServiceTest : public testing::Test {
 public:
  PageContentAnnotationsServiceTest() {
    scoped_feature_list_.InitWithFeaturesAndParameters(
        {{features::kPageContentAnnotations,
          {
              {"write_to_history_service", "true"},
              {"pca_service_wait_for_title_delay_in_milliseconds", "4999"},
          }},
         {features::kPageVisibilityPageContentAnnotations, {}}},
        /*disabled_features=*/{
            optimization_guide::features::kPreventLongRunningPredictionModels});
  }
  ~PageContentAnnotationsServiceTest() override = default;

  void SetUp() override {
    optimization_guide_model_provider_ = std::make_unique<
        optimization_guide::TestOptimizationGuideModelProvider>();
    history_service_ =
        std::make_unique<testing::StrictMock<MockHistoryService>>();

    template_url_service_ = std::make_unique<TemplateURLService>(
        kTemplateURLData, std::size(kTemplateURLData));

    optimization_guide_decider_ =
        std::make_unique<FakeOptimizationGuideDecider>();

    // Instantiate service.
    service_ = std::make_unique<PageContentAnnotationsService>(
        /*autocomplete_provider_client=*/nullptr, "en-US", "us",
        optimization_guide_model_provider_.get(), history_service_.get(),
        template_url_service_.get(),
        /*zero_suggest_cache_service=*/nullptr,
        /*database_provider=*/nullptr,
        /*database_dir=*/base::FilePath(),
        /*optimization_guide_logger=*/nullptr,
        optimization_guide_decider_.get(),
        /*background_task_runner=*/nullptr);

#if BUILDFLAG(BUILD_WITH_TFLITE_LIB)
    test_annotator_ = std::make_unique<TestPageContentAnnotator>();
    test_annotator_->UseVisibilityScores(/*model_info=*/std::nullopt,
                                         {{"test", 0.5}});
    service_->OverridePageContentAnnotatorForTesting(test_annotator_.get());
#endif
  }

  // Simulates a visit to URL.
  void VisitURL(const GURL& url,
                const std::u16string& title,
                history::VisitID visit_id,
                std::optional<int64_t> local_navigation_id,
                bool is_synced_visit = false) {
    history::URLRow url_row(url);
    url_row.set_title(title);
    history::VisitRow new_visit;
    new_visit.visit_id = visit_id;
    new_visit.originator_cache_guid = is_synced_visit ? "otherdevice" : "";
    service_->OnURLVisitedWithNavigationId(history_service_.get(), url_row,
                                           new_visit, local_navigation_id);
  }

  FakeOptimizationGuideDecider* optimization_guide_decider() {
    return optimization_guide_decider_.get();
  }

  base::test::TaskEnvironment task_environment_{
      base::test::TaskEnvironment::TimeSource::MOCK_TIME};

  PageContentAnnotationsService* service() { return service_.get(); }

 protected:
  std::unique_ptr<MockHistoryService> history_service_;

 private:
  base::test::ScopedFeatureList scoped_feature_list_;

  std::unique_ptr<optimization_guide::TestOptimizationGuideModelProvider>
      optimization_guide_model_provider_;
  std::unique_ptr<TemplateURLService> template_url_service_;
  std::unique_ptr<TestPageContentAnnotator> test_annotator_;
  std::unique_ptr<FakeOptimizationGuideDecider> optimization_guide_decider_;
  std::unique_ptr<PageContentAnnotationsService> service_;
};

TEST_F(PageContentAnnotationsServiceTest, ObserveLocalVisitNonSearch) {
  history::VisitID visit_id = 1;

#if BUILDFLAG(BUILD_WITH_TFLITE_LIB)
  EXPECT_CALL(*history_service_,
              AddContentModelAnnotationsForVisit(_, visit_id));
#endif

  VisitURL(GURL("https://example.com"), u"test", visit_id,
           /*local_navigation_id=*/1,
           /*is_synced_visit=*/false);

  task_environment_.FastForwardBy(base::Seconds(5));
}

TEST_F(PageContentAnnotationsServiceTest, NonHTTPUrlIgnored) {
  history::VisitID visit_id = 1;

#if BUILDFLAG(BUILD_WITH_TFLITE_LIB)
  EXPECT_CALL(*history_service_,
              AddContentModelAnnotationsForVisit(_, visit_id))
      .Times(0);
#endif

  VisitURL(GURL("data:,"), u"test", visit_id,
           /*local_navigation_id=*/1,
           /*is_synced_visit=*/false);

  task_environment_.FastForwardBy(base::Seconds(5));
}

TEST_F(PageContentAnnotationsServiceTest, ObserveSyncedVisitsNonSearch) {
  history::VisitID visit_id = 1;

#if BUILDFLAG(BUILD_WITH_TFLITE_LIB)
  EXPECT_CALL(*history_service_,
              AddContentModelAnnotationsForVisit(_, visit_id));
#endif

  VisitURL(GURL("https://example.com"), u"test", visit_id,
           /*local_navigation_id=*/1,
           /*is_synced_visit=*/true);

  task_environment_.FastForwardBy(base::Seconds(5));
}

TEST_F(PageContentAnnotationsServiceTest, ObserveLocalVisitsSearch) {
  history::VisitID visit_id = 1;
  base::HistogramTester histogram_tester;

  EXPECT_CALL(*history_service_, AddSearchMetadataForVisit(_, _, visit_id));

#if BUILDFLAG(BUILD_WITH_TFLITE_LIB)
  EXPECT_CALL(*history_service_,
              AddContentModelAnnotationsForVisit(_, visit_id));
#endif

  VisitURL(GURL("http://www.google.com/search?q=test#frag"), u"Test Page",
           visit_id, /*local_navigation_id=*/1,
           /*is_synced_visit=*/false);

  task_environment_.FastForwardBy(base::Seconds(5));

  histogram_tester.ExpectUniqueSample(
      "OptimizationGuide.PageContentAnnotations.GoogleSearchMetadataExtracted",
      true, 1);
}

TEST_F(PageContentAnnotationsServiceTest, ObserveSyncedVisitsSearch) {
  history::VisitID visit_id = 1;

  EXPECT_CALL(*history_service_, AddSearchMetadataForVisit(_, _, visit_id));

#if BUILDFLAG(BUILD_WITH_TFLITE_LIB)
  EXPECT_CALL(*history_service_,
              AddContentModelAnnotationsForVisit(_, visit_id));
#endif

  VisitURL(GURL("https://default-engine.com/search?q=test#frag"), u"Test Page",
           visit_id, /*local_navigation_id=*/1,
           /*is_synced_visit=*/true);

  task_environment_.FastForwardBy(base::Seconds(5));
}

class PageContentAnnotationsServiceRemotePageMetadataTest
    : public PageContentAnnotationsServiceTest {
 public:
  PageContentAnnotationsServiceRemotePageMetadataTest() {
    scoped_feature_list_.InitAndEnableFeature(features::kRemotePageMetadata);
  }

 private:
  base::test::ScopedFeatureList scoped_feature_list_;
};

TEST_F(PageContentAnnotationsServiceRemotePageMetadataTest,
       RegistersTypeWhenFeatureEnabled) {
  std::vector<optimization_guide::proto::OptimizationType>
      registered_optimization_types =
          optimization_guide_decider()->registered_optimization_types();
  EXPECT_TRUE(base::Contains(registered_optimization_types,
                             optimization_guide::proto::PAGE_ENTITIES));
}

TEST_F(PageContentAnnotationsServiceRemotePageMetadataTest,
       DoesNotPersistIfServerHasNoData) {
  VisitURL(GURL("http://www.nohints.com"), u"sometitle", 13,
           /*local_navigation_id=*/1);
}

TEST_F(PageContentAnnotationsServiceRemotePageMetadataTest,
       DoesNotPersistIfServerReturnsWrongMetadata) {
  // Navigate.
  VisitURL(GURL("http://wrongmetadata.com"), u"sometitle", 13,
           /*local_navigation_id=*/1);
}

TEST_F(PageContentAnnotationsServiceRemotePageMetadataTest,
       RequestsToPersistIfHasPageMetadata) {
  EXPECT_CALL(*history_service_,
              AddPageMetadataForVisit("alternative title", 13));

  // Navigate.
  VisitURL(GURL("http://hasmetadata.com"), u"sometitle", 13,
           /*local_navigation_id=*/1);
}

class PageContentAnnotationsServiceSalientImageMetadataTest
    : public PageContentAnnotationsServiceTest {
 public:
  PageContentAnnotationsServiceSalientImageMetadataTest() {
    scoped_feature_list_.InitAndEnableFeature(
        features::kPageContentAnnotationsPersistSalientImageMetadata);
  }

 private:
  base::test::ScopedFeatureList scoped_feature_list_;
};

TEST_F(PageContentAnnotationsServiceSalientImageMetadataTest,
       RegistersTypeWhenFeatureEnabled) {
  std::vector<optimization_guide::proto::OptimizationType>
      registered_optimization_types =
          optimization_guide_decider()->registered_optimization_types();
  EXPECT_TRUE(base::Contains(registered_optimization_types,
                             optimization_guide::proto::SALIENT_IMAGE));
}

TEST_F(PageContentAnnotationsServiceSalientImageMetadataTest,
       DoesNotPersistIfServerHasNoData) {
  // Navigate.
  VisitURL(GURL("http://www.nohints.com"), u"sometitle", 13,
           /*local_navigation_id=*/1);
}

TEST_F(PageContentAnnotationsServiceSalientImageMetadataTest,
       DoesNotPersistIfServerReturnsWrongMetadata) {
  // Navigate.
  VisitURL(GURL("http://wrongmetadata.com"), u"sometitle", 13,
           /*local_navigation_id=*/1);
}

TEST_F(PageContentAnnotationsServiceSalientImageMetadataTest,
       RequestsToPersistIfHasSalientImageMetadata) {
  EXPECT_CALL(*history_service_, SetHasUrlKeyedImageForVisit(true, 13));

  // Navigate.
  VisitURL(GURL("http://hasimageurl.com"), u"sometitle", 13,
           /*local_navigation_id=*/1);
}

}  // namespace page_content_annotations
