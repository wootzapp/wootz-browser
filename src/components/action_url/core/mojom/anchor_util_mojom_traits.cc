// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/action_url/core/mojom/anchor_util_mojom_traits.h"

#include "mojo/public/cpp/base/string16_mojom_traits.h"
#include "url/mojom/url_gurl_mojom_traits.h"

namespace mojo {

// static
bool StructTraits<
    action_url::mojom::AnchorDataDataView,
    action_url::AnchorData>::Read(action_url::mojom::AnchorDataDataView data,
                                  action_url::AnchorData* out) {
  if (!data.ReadIdAttribute(&out->id_attribute)) {
    return false;
  }
  if (!data.ReadUrl(&out->url)) {
    return false;
  }
  out->renderer_id = data.renderer_id();
  return true;
}
}  // namespace mojo
