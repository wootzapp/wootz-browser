/* Copyright (c) 2019 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_CONSTANTS_WOOTZ_PATHS_H_
#define COMPONENTS_CONSTANTS_WOOTZ_PATHS_H_

// This file declares path keys for the wootz module.  These can be used
// with the PathService to access various special directories and files.

namespace wootz {

enum {
  PATH_START = 20000,

  DIR_TEST_DATA,  // Directory where unit test data resides.
  PATH_END
};

}  // namespace wootz

#endif  // COMPONENTS_CONSTANTS_WOOTZ_PATHS_H_
