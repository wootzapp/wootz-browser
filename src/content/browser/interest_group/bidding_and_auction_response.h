// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_INTEREST_GROUP_BIDDING_AND_AUCTION_RESPONSE_H_
#define CONTENT_BROWSER_INTEREST_GROUP_BIDDING_AND_AUCTION_RESPONSE_H_

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "base/containers/flat_map.h"
#include "base/containers/span.h"
#include "base/values.h"
#include "content/browser/interest_group/auction_result.h"
#include "content/common/content_export.h"
#include "third_party/blink/public/common/interest_group/ad_auction_currencies.h"
#include "third_party/blink/public/common/interest_group/interest_group.h"
#include "url/gurl.h"
#include "url/origin.h"

namespace content {

std::optional<base::span<const uint8_t>> CONTENT_EXPORT
ExtractCompressedBiddingAndAuctionResponse(
    base::span<const uint8_t> decrypted_data);

struct CONTENT_EXPORT BiddingAndAuctionResponse {
  BiddingAndAuctionResponse();
  ~BiddingAndAuctionResponse();

  BiddingAndAuctionResponse(BiddingAndAuctionResponse&& other);
  BiddingAndAuctionResponse& operator=(BiddingAndAuctionResponse&& other);

  static std::optional<BiddingAndAuctionResponse> TryParse(
      base::Value input,
      const base::flat_map<url::Origin, std::vector<std::string>>& group_names);

  struct CONTENT_EXPORT ReportingURLs {
    ReportingURLs();
    ~ReportingURLs();

    ReportingURLs(ReportingURLs&& other);
    ReportingURLs& operator=(ReportingURLs&& other);

    static std::optional<ReportingURLs> TryParse(base::Value::Dict* input_dict);

    std::optional<GURL> reporting_url;
    base::flat_map<std::string, GURL> beacon_urls;
  };

  // This is not part of the message from the server, but is a convenient place
  // to store the outcome if we finish parsing the response before the component
  // auctions start the bidding phase.
  AuctionResult result = AuctionResult::kInvalidServerResponse;

  bool is_chaff = false;  // indicates this response should be ignored.
  // TODO(behamilton): Add support for creative dimensions to the response from
  // the Bidding and Auction server.
  GURL ad_render_url;
  std::vector<GURL> ad_components;
  std::string interest_group_name;
  url::Origin interest_group_owner;
  std::vector<blink::InterestGroupKey> bidding_groups;
  std::optional<double> score, bid;
  std::optional<blink::AdCurrency> bid_currency;
  std::optional<url::Origin> top_level_seller;
  std::optional<std::string> ad_metadata;
  std::optional<std::string> buyer_reporting_id;
  std::optional<std::string> buyer_and_seller_reporting_id;

  std::optional<std::string> error;
  // The Bidding and Auction server uses the top_level_seller_reporting field
  // for single-level auctions.
  std::optional<ReportingURLs> buyer_reporting, top_level_seller_reporting,
      component_seller_reporting;
};

}  // namespace content

#endif  // CONTENT_BROWSER_INTEREST_GROUP_BIDDING_AND_AUCTION_RESPONSE_H_
