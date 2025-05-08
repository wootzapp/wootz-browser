// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_ACTION_URL_CORE_MOJOM_ANCHOR_UTIL_MOJOM_TRAITS_H_
#define COMPONENTS_ACTION_URL_CORE_MOJOM_ANCHOR_UTIL_MOJOM_TRAITS_H_

#include <string>

#include "components/action_url/core/anchor_data.h"
#include "components/action_url/core/mojom/anchor_util.mojom-shared.h"
#include "mojo/public/cpp/bindings/struct_traits.h"

namespace mojo {

template <>
struct StructTraits<action_url::mojom::AnchorDataDataView,
                    action_url::AnchorData> {
  static const std::u16string& id_attribute(const action_url::AnchorData& r) {
    return r.id_attribute;
  }

  static const GURL& url(const action_url::AnchorData& r) { return r.url; }

  static uint64_t renderer_id(const action_url::AnchorData& r) {
    return r.renderer_id;
  }

  static bool Read(action_url::mojom::AnchorDataDataView data,
                   action_url::AnchorData* out);
};

}  // namespace mojo

#endif  // COMPONENTS_ACTION_URL_CORE_MOJOM_ANCHOR_UTIL_MOJOM_TRAITS_H_
