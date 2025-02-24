// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_REPLACE_ELEMENT_CORE_REPLACE_ELEMENT_DRIVER_H_
#define COMPONENTS_REPLACE_ELEMENT_CORE_REPLACE_ELEMENT_DRIVER_H_

namespace replace_element {

class ReplaceElementDriver {
 public:
  ReplaceElementDriver() = default;

  ReplaceElementDriver(const ReplaceElementDriver&) = delete;
  ReplaceElementDriver& operator=(const ReplaceElementDriver&) = delete;

  virtual ~ReplaceElementDriver() = default;

  // Returns driver id which is unique in the current tab.
  virtual int GetId() const = 0;
  virtual void ReplaceElement(std::string element, std::string json_data) = 0;
};

}  // namespace replace_element

#endif  // COMPONENTS_REPLACE_ELEMENT_CORE_REPLACE_ELEMENT_DRIVER_H_