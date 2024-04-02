// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ash/mahi/mahi_cache_manager.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace ash {

class MahiCacheManagerTest : public testing::Test {
 public:
  MahiCacheManagerTest() = default;

  MahiCacheManagerTest(const MahiCacheManagerTest&) = delete;
  MahiCacheManagerTest& operator=(const MahiCacheManagerTest&) = delete;

  ~MahiCacheManagerTest() override = default;

  MahiCacheManager* GetMahiCacheManager() { return mahi_cache_manager_.get(); }

  std::map<GURL, MahiCacheManager::MahiData> GetPageCache() {
    return mahi_cache_manager_->page_cache_;
  }

  // testing::Test:
  void SetUp() override {
    mahi_cache_manager_ = std::make_unique<MahiCacheManager>();
    mahi_cache_manager_->page_cache_[GURL("http://url1.com/")] =
        MahiCacheManager::MahiData(
            "http://url1.com", u"title 1", std::nullopt, u"page content 1",
            u"summary 1",
            {{u"Question 1", u"Answer 1"}, {u"Question 2", u"Answer 2"}});
    mahi_cache_manager_->page_cache_[GURL("http://url2.com/")] =
        MahiCacheManager::MahiData("http://url2.com", u"title 2", std::nullopt,
                                   u"page content 2", u"summary 2",
                                   {{u"question 1", u"answer 1"}});
  }

  void TearDown() override { mahi_cache_manager_.reset(); }

 private:
  std::unique_ptr<MahiCacheManager> mahi_cache_manager_;
};

TEST_F(MahiCacheManagerTest, AddNewURL) {
  EXPECT_EQ(GetPageCache().size(), 2u);
  GetMahiCacheManager()->AddCacheForUrl("http://url3.com",
                                        {"http://url3.com",
                                         u"title 3",
                                         std::nullopt,
                                         u"page content 3",
                                         u"summary 3",
                                         {{u"new question", u"new answer"}}});
  EXPECT_EQ(GetPageCache().size(), 3u);
}

TEST_F(MahiCacheManagerTest, ReplacingExistingURLWithNewContent) {
  MahiCacheManager::MahiData new_data(
      "http://url1.com", u"New title", std::nullopt, u"New content",
      u"New summary", {{u"new question", u"new answer"}});
  GetMahiCacheManager()->AddCacheForUrl("http://url1.com", new_data);
  EXPECT_EQ(GetPageCache().size(), 2u);
  EXPECT_EQ(GetPageCache().at(GURL("http://url1.com")).url, new_data.url);
  EXPECT_EQ(GetPageCache().at(GURL("http://url1.com")).title, new_data.title);
  EXPECT_EQ(GetPageCache().at(GURL("http://url1.com")).page_content,
            new_data.page_content);
  EXPECT_EQ(GetPageCache().at(GURL("http://url1.com")).summary,
            new_data.summary);
  EXPECT_EQ(GetPageCache().at(GURL("http://url1.com")).previous_qa.size(), 1u);
}

TEST_F(MahiCacheManagerTest, GetSummaryFromTheCacheSameURL) {
  auto result = GetMahiCacheManager()->GetSummaryForUrl("http://url1.com");
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), u"summary 1");
}

TEST_F(MahiCacheManagerTest, GetSummaryFromTheCacheDifferentURL) {
  auto result =
      GetMahiCacheManager()->GetSummaryForUrl("http://url1.com/search");
  EXPECT_FALSE(result.has_value());
}

TEST_F(MahiCacheManagerTest, GetSummaryFromTheCacheWithRef) {
  auto result = GetMahiCacheManager()->GetSummaryForUrl("http://url1.com/#ref");
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), u"summary 1");
}

TEST_F(MahiCacheManagerTest, GetQAFromCacheSameURL) {
  auto result = GetMahiCacheManager()->GetQAForUrl("http://url1.com");
  EXPECT_EQ(result.size(), 2u);
  EXPECT_EQ(result[0].question, u"Question 1");
  EXPECT_EQ(result[0].answer, u"Answer 1");
  EXPECT_EQ(result[1].question, u"Question 2");
  EXPECT_EQ(result[1].answer, u"Answer 2");
}

TEST_F(MahiCacheManagerTest, GetQAFromCacheURLWithRef) {
  auto result = GetMahiCacheManager()->GetQAForUrl("http://url1.com/#ref");
  EXPECT_EQ(result.size(), 2u);
  EXPECT_EQ(result[0].question, u"Question 1");
  EXPECT_EQ(result[0].answer, u"Answer 1");
  EXPECT_EQ(result[1].question, u"Question 2");
  EXPECT_EQ(result[1].answer, u"Answer 2");
}

TEST_F(MahiCacheManagerTest, ClearCacheSuccessfully) {
  GetMahiCacheManager()->ClearCache();
  EXPECT_EQ(GetPageCache().size(), 0u);
}

}  // namespace ash
