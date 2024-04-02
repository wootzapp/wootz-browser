// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_PUBLIC_CPP_PICKER_PICKER_CATEGORY_H_
#define ASH_PUBLIC_CPP_PICKER_PICKER_CATEGORY_H_

#include "ash/public/cpp/ash_public_export.h"

namespace ash {

// A category specifies a type of data that can be searched for.
enum class ASH_PUBLIC_EXPORT PickerCategory {
  kEmojis,
  kSymbols,
  kEmoticons,
  kGifs,
  kOpenTabs,
  kBrowsingHistory,
  kBookmarks,
  kDriveFiles,
  kLocalFiles
};

}  // namespace ash

#endif  // ASH_PUBLIC_CPP_PICKER_PICKER_CATEGORY_H_
