// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/supervised_user/test_support/kids_chrome_management_test_utils.h"

#include <string>

#include "base/strings/strcat.h"
#include "base/strings/string_piece.h"
#include "components/signin/public/identity_manager/identity_test_utils.h"
#include "components/supervised_user/core/browser/proto/kidschromemanagement_messages.pb.h"

namespace supervised_user {

void SetFamilyMemberAttributesForTesting(
    kids_chrome_management::FamilyMember* mutable_member,
    kids_chrome_management::FamilyRole role,
    base::StringPiece username) {
  mutable_member->mutable_profile()->set_display_name(std::string(username));
  const std::string email = base::StrCat({username, "@gmail.com"});
  mutable_member->mutable_profile()->set_email(email);
  mutable_member->mutable_profile()->set_profile_url(
      base::StrCat({"http://profile.url/", username}));
  mutable_member->mutable_profile()->set_profile_image_url(
      base::StrCat({"http://image.url/", username}));
  mutable_member->set_role(role);
  mutable_member->set_user_id(signin::GetTestGaiaIdForEmail(email));
}

}  // namespace supervised_user
