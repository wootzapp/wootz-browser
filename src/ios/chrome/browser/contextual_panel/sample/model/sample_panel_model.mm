// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/contextual_panel/sample/model/sample_panel_model.h"

#import "ios/chrome/browser/contextual_panel/sample/model/sample_panel_item_configuration.h"
#import "ios/chrome/browser/shared/model/url/url_util.h"
#import "ios/web/public/web_state.h"

SamplePanelModel::SamplePanelModel() {}

SamplePanelModel::~SamplePanelModel() {}

void SamplePanelModel::FetchConfigurationForWebState(
    web::WebState* web_state,
    FetchConfigurationForWebStateCallback callback) {
  // No data to show for the current webstate.
  if (IsUrlNtp(web_state->GetVisibleURL())) {
    base::SequencedTaskRunner::GetCurrentDefault()->PostTask(
        FROM_HERE, base::BindOnce(std::move(callback), std::move(nullptr)));
    return;
  }

  std::unique_ptr<SamplePanelItemConfiguration> item_configuration =
      std::make_unique<SamplePanelItemConfiguration>();

  // Happy path, there is data to show for the current webstate.
  item_configuration->sample_name = "sample_config";
  item_configuration->accessibility_label = "Sample entrypoint";
  item_configuration->entrypoint_message = "Large entrypoint";
  item_configuration->entrypoint_image_name = "chrome_product";
  item_configuration->image_type =
      ContextualPanelItemConfiguration::EntrypointImageType::SFSymbol;
  item_configuration->relevance =
      ContextualPanelItemConfiguration::high_relevance;
  base::SequencedTaskRunner::GetCurrentDefault()->PostTask(
      FROM_HERE,
      base::BindOnce(std::move(callback), std::move(item_configuration)));
}
