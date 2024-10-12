// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/public/common/interest_group/interest_group.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace blink {

TEST(InterestGroupTest, DEPRECATED_KAnonKeyForAdNameReporting) {
  // Make sure that DEPRECATED_KAnonKeyForAdNameReporting properly prioritizes
  // and incorporates various kinds of reporting IDs.
  InterestGroup ig;
  ig.owner = url::Origin::Create(GURL("https://example.org"));
  ig.name = "ig_one";
  ig.bidding_url = GURL("https://example.org/bid.js");
  ig.ads = {{{/*render_url=*/GURL("https://ad1.com"),
              /*metadata=*/std::nullopt, /*size_group=*/std::nullopt,
              /*buyer_reporting_id=*/std::nullopt,
              /*buyer_and_seller_reporting_id=*/std::nullopt},
             {/*render_url=*/GURL("https://ad2.com"),
              /*metadata=*/std::nullopt, /*size_group=*/std::nullopt,
              /*buyer_reporting_id=*/"bid",
              /*buyer_and_seller_reporting_id=*/std::nullopt},
             {/*render_url=*/GURL("https://ad3.com"),
              /*metadata=*/std::nullopt, /*size_group=*/std::nullopt,
              /*buyer_reporting_id=*/"bid",
              /*buyer_and_seller_reporting_id=*/"bsid"}}};
  EXPECT_EQ(
      "NameReport\nhttps://example.org/\nhttps://example.org/bid.js\n"
      "https://ad1.com/\nig_one",
      DEPRECATED_KAnonKeyForAdNameReporting(ig, ig.ads->at(0)));
  EXPECT_EQ(
      "BuyerReportId\nhttps://example.org/\nhttps://example.org/bid.js\n"
      "https://ad2.com/\nbid",
      DEPRECATED_KAnonKeyForAdNameReporting(ig, ig.ads->at(1)));
  EXPECT_EQ(
      "BuyerAndSellerReportId\nhttps://example.org/\n"
      "https://example.org/bid.js\nhttps://ad3.com/\nbsid",
      DEPRECATED_KAnonKeyForAdNameReporting(ig, ig.ads->at(2)));
}

// Test ParseTrustedBiddingSignalsSlotSizeMode() and
// TrustedBiddingSignalsSlotSizeModeToString().
TEST(InterestGroupTest, TrustedBiddingSignalsSlotSizeMode) {
  EXPECT_EQ(InterestGroup::TrustedBiddingSignalsSlotSizeMode::kNone,
            InterestGroup::ParseTrustedBiddingSignalsSlotSizeMode("none"));
  EXPECT_EQ("none",
            InterestGroup::TrustedBiddingSignalsSlotSizeModeToString(
                InterestGroup::TrustedBiddingSignalsSlotSizeMode::kNone));

  EXPECT_EQ(InterestGroup::TrustedBiddingSignalsSlotSizeMode::kSlotSize,
            InterestGroup::ParseTrustedBiddingSignalsSlotSizeMode("slot-size"));
  EXPECT_EQ("slot-size",
            InterestGroup::TrustedBiddingSignalsSlotSizeModeToString(
                InterestGroup::TrustedBiddingSignalsSlotSizeMode::kSlotSize));

  EXPECT_EQ(
      InterestGroup::TrustedBiddingSignalsSlotSizeMode::kAllSlotsRequestedSizes,
      InterestGroup::ParseTrustedBiddingSignalsSlotSizeMode(
          "all-slots-requested-sizes"));
  EXPECT_EQ("all-slots-requested-sizes",
            InterestGroup::TrustedBiddingSignalsSlotSizeModeToString(
                InterestGroup::TrustedBiddingSignalsSlotSizeMode::
                    kAllSlotsRequestedSizes));

  EXPECT_EQ(InterestGroup::TrustedBiddingSignalsSlotSizeMode::kNone,
            InterestGroup::ParseTrustedBiddingSignalsSlotSizeMode(
                "not-a-valid-mode"));
}

}  // namespace blink
