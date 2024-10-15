/* Copyright (c) 2019 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "chrome/browser/ui/webui/wootz_webui_source.h"

#include <string_view>
#include <vector>

#include "base/containers/flat_map.h"
#include "base/strings/utf_string_conversions.h"
#include "components/constants/url_constants.h"
#include "components/webui/webui_resources.h"
#include "build/build_config.h"
#include "chrome/browser/profiles/profile.h"
#include "components/grit/wootz_components_resources.h"
#include "components/grit/wootz_components_strings.h"
#include "components/grit/components_resources.h"
#include "content/public/browser/web_ui_data_source.h"
#include "services/network/public/mojom/content_security_policy.mojom.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/webui/resource_path.h"
#include "ui/base/webui/web_ui_util.h"


namespace {

void CustomizeWebUIHTMLSource(content::WebUI* web_ui,
                              const std::string& name,
                              content::WebUIDataSource* source) {

  source->AddResourcePaths(wootz::GetWebUIResources(name));
  source->AddLocalizedStrings(wootz::GetWebUILocalizedStrings(name));
}

content::WebUIDataSource* CreateWebUIDataSource(
    content::WebUI* web_ui,
    const std::string& name,
    const webui::ResourcePath* resource_map,
    size_t resource_map_size,
    int html_resource_id,
    bool disable_trusted_types_csp) {
  content::WebUIDataSource* source =
      content::WebUIDataSource::CreateAndAdd(Profile::FromWebUI(web_ui), name);
  // Some parts of Brave's UI pages are not yet migrated to work without doing
  // assignments of strings directly into |innerHTML| elements (i.e. see usage
  // of |dangerouslySetInnerHTML| in .tsx files). This will break Brave due to
  // committing a Trusted Types related violation now that Trusted Types are
  // enforced on WebUI pages (see crrev.com/c/2234238 and crrev.com/c/2353547).
  // We should migrate those pages not to require using |innerHTML|, but for now
  // we just restore pre-Cromium 87 behaviour for pages that are not ready yet.
  if (disable_trusted_types_csp) {
    source->DisableTrustedTypesCSP();
  } else {
    // Allow a policy to be created so that we
    // can allow trusted HTML and trusted lazy-load script sources.
    source->OverrideContentSecurityPolicy(
        network::mojom::CSPDirectiveName::TrustedTypes,
        "trusted-types default;");
  }

  source->UseStringsJs();
  source->SetDefaultResource(html_resource_id);
  // Add generated resource paths
  for (size_t i = 0; i < resource_map_size; ++i) {
    source->AddResourcePath(resource_map[i].path, resource_map[i].id);
  }
  CustomizeWebUIHTMLSource(web_ui, name, source);
  return source;
}

}  // namespace

content::WebUIDataSource* CreateAndAddWebUIDataSource(
    content::WebUI* web_ui,
    const std::string& name,
    const webui::ResourcePath* resource_map,
    size_t resource_map_size,
    int html_resource_id,
    bool disable_trusted_types_csp) {
  content::WebUIDataSource* data_source =
      CreateWebUIDataSource(web_ui, name, resource_map, resource_map_size,
                            html_resource_id, disable_trusted_types_csp);
  return data_source;
}

