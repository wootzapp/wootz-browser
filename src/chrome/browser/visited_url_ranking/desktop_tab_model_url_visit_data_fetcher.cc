// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/visited_url_ranking/desktop_tab_model_url_visit_data_fetcher.h"

#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_list.h"
#include "chrome/browser/ui/tabs/tab_renderer_data.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "components/sessions/content/session_tab_helper.h"
#include "components/visited_url_ranking/internal/url_visit_util.h"
#include "components/visited_url_ranking/public/fetch_options.h"
#include "components/visited_url_ranking/public/fetch_result.h"
#include "components/visited_url_ranking/public/url_visit.h"
#include "components/visited_url_ranking/public/url_visit_data_fetcher.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/web_contents.h"

namespace visited_url_ranking {

using Source = URLVisit::Source;
using URLVisitVariant = URLVisitAggregate::URLVisitVariant;

namespace {

URLVisitAggregate::Tab MakeAggregateTabFromWebContents(
    content::WebContents* web_contents) {
  auto* last_entry = web_contents->GetController().GetLastCommittedEntry();
  URLVisitAggregate::Tab tab(
      sessions::SessionTabHelper::IdForTab(web_contents).id(),
      URLVisit(web_contents->GetURL(), web_contents->GetTitle(),
               last_entry->GetTimestamp(),
               syncer::DeviceInfo::FormFactor::kDesktop, Source::kLocal));
  return tab;
}

}  // namespace

DesktopTabModelURLVisitDataFetcher::DesktopTabModelURLVisitDataFetcher(
    Profile* profile)
    : profile_(profile) {}

DesktopTabModelURLVisitDataFetcher::~DesktopTabModelURLVisitDataFetcher() =
    default;

void DesktopTabModelURLVisitDataFetcher::FetchURLVisitData(
    const FetchOptions& options,
    FetchResultCallback callback) {
  std::map<URLMergeKey, URLVisitAggregate::TabData> url_visit_tab_data_map;
  const BrowserList* browser_list = BrowserList::GetInstance();
  for (const Browser* browser : *browser_list) {
    if (browser->profile() != profile_) {
      continue;
    }

    TabStripModel* tab_strip_model = browser->tab_strip_model();
    for (int i = 0; i < tab_strip_model->GetTabCount(); ++i) {
      auto* web_contents = tab_strip_model->GetWebContentsAt(i);
      auto* last_entry = web_contents->GetController().GetLastCommittedEntry();
      if (!last_entry) {
        continue;
      }

      auto url_key = ComputeURLMergeKey(web_contents->GetURL());
      auto it = url_visit_tab_data_map.find(url_key);
      if ((it == url_visit_tab_data_map.end()) ||
          (it->second.last_active_tab.visit.last_modified <
           last_entry->GetTimestamp())) {
        url_visit_tab_data_map.emplace(
            url_key, MakeAggregateTabFromWebContents(web_contents));
      }

      auto& tab_data = url_visit_tab_data_map.at(url_key);
      tab_data.tab_count += 1;
      TabRendererData tab_renderer_data =
          TabRendererData::FromTabInModel(tab_strip_model, i);
      tab_data.pinned = tab_data.pinned || tab_renderer_data.pinned;
      tab_data.in_group =
          tab_data.in_group ||
          (tab_strip_model->GetTabGroupForTab(i) != std::nullopt);
    }
  }

  std::map<URLMergeKey, URLVisitVariant> url_visit_variant_map;
  std::transform(
      std::make_move_iterator(url_visit_tab_data_map.begin()),
      std::make_move_iterator(url_visit_tab_data_map.end()),
      std::inserter(url_visit_variant_map, url_visit_variant_map.end()),
      [](auto kv) { return std::make_pair(kv.first, std::move(kv.second)); });

  std::move(callback).Run(
      {FetchResult::Status::kSuccess, std::move(url_visit_variant_map)});
}

}  // namespace visited_url_ranking
