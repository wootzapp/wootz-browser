// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_ACTION_URL_CORE_ANCHOR_DATA_H_
#define COMPONENTS_ACTION_URL_CORE_ANCHOR_DATA_H_

#include <vector>

#include "build/build_config.h"
#include "third_party/blink/public/common/tokens/tokens.h"
#include "url/gurl.h"

using blink::LocalFrameToken;
namespace action_url {

struct AnchorData {
  AnchorData();
  AnchorData(const AnchorData&);
  AnchorData& operator=(const AnchorData&);
  AnchorData(AnchorData&&);
  AnchorData& operator=(AnchorData&&);
  ~AnchorData();

  bool SameAnchorAs(const AnchorData& other) const;

  GURL url;

  std::u16string id_attribute;

  int driver_id;  // unique in the current tab

  int process_id;  // RenderProcessHost id

  LocalFrameToken host_frame;  // unique id for each frame

  // Uniquely identifies the DOM element that this form represents among the
  // form DOM elements in the same frame.
  uint64_t renderer_id = 0;
};

}  // namespace action_url

#endif  // COMPONENTS_ACTION_URL_CORE_ANCHOR_DATA_H_
