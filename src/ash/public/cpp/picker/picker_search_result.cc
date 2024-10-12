// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/public/cpp/picker/picker_search_result.h"

#include <string>
#include <string_view>
#include <utility>

#include "ui/base/models/image_model.h"
#include "ui/gfx/geometry/size.h"
#include "url/gurl.h"

namespace ash {

PickerSearchResult::TextData::TextData(std::u16string primary_text,
                                       std::u16string secondary_text,
                                       ui::ImageModel icon,
                                       Source source)
    : primary_text(std::move(primary_text)),
      secondary_text(std::move(secondary_text)),
      icon(std::move(icon)),
      source(source) {}

PickerSearchResult::TextData::TextData(const PickerSearchResult::TextData&) =
    default;

PickerSearchResult::TextData& PickerSearchResult::TextData::operator=(
    const PickerSearchResult::TextData&) = default;

PickerSearchResult::TextData::~TextData() = default;

bool PickerSearchResult::TextData::operator==(
    const PickerSearchResult::TextData&) const = default;

bool PickerSearchResult::SearchRequestData::operator==(
    const PickerSearchResult::SearchRequestData&) const = default;

bool PickerSearchResult::EmojiData::operator==(
    const PickerSearchResult::EmojiData&) const = default;

bool PickerSearchResult::SymbolData::operator==(
    const PickerSearchResult::SymbolData&) const = default;

bool PickerSearchResult::EmoticonData::operator==(
    const PickerSearchResult::EmoticonData&) const = default;

PickerSearchResult::ClipboardData::ClipboardData(
    base::UnguessableToken item_id,
    DisplayFormat display_format,
    std::u16string display_text,
    std::optional<ui::ImageModel> display_image)
    : item_id(item_id),
      display_format(display_format),
      display_text(std::move(display_text)),
      display_image(std::move(display_image)) {}

PickerSearchResult::ClipboardData::ClipboardData(
    const PickerSearchResult::ClipboardData&) = default;

PickerSearchResult::ClipboardData& PickerSearchResult::ClipboardData::operator=(
    const PickerSearchResult::ClipboardData&) = default;

PickerSearchResult::ClipboardData::~ClipboardData() = default;

bool PickerSearchResult::ClipboardData::operator==(
    const PickerSearchResult::ClipboardData&) const = default;

PickerSearchResult::GifData::GifData(const GURL& preview_url,
                                     const GURL& preview_image_url,
                                     const gfx::Size& preview_dimensions,
                                     const GURL& full_url,
                                     const gfx::Size& full_dimensions,
                                     std::u16string content_description)
    : preview_url(preview_url),
      preview_image_url(preview_image_url),
      preview_dimensions(preview_dimensions),
      full_url(full_url),
      full_dimensions(full_dimensions),
      content_description(std::move(content_description)) {}

PickerSearchResult::GifData::GifData(const PickerSearchResult::GifData&) =
    default;

PickerSearchResult::GifData& PickerSearchResult::GifData::operator=(
    const PickerSearchResult::GifData&) = default;

PickerSearchResult::GifData::~GifData() = default;

bool PickerSearchResult::GifData::operator==(
    const PickerSearchResult::GifData&) const = default;

bool PickerSearchResult::LocalFileData::operator==(const LocalFileData&) const =
    default;

bool PickerSearchResult::DriveFileData::operator==(const DriveFileData&) const =
    default;

bool PickerSearchResult::BrowsingHistoryData::operator==(
    const PickerSearchResult::BrowsingHistoryData&) const = default;

bool PickerSearchResult::CategoryData::operator==(const CategoryData&) const =
    default;

PickerSearchResult::EditorData::EditorData(
    Mode mode,
    std::u16string display_name,
    std::optional<chromeos::editor_menu::PresetQueryCategory> category,
    std::optional<std::string> preset_query_id,
    std::optional<std::string> freeform_text)
    : mode(mode),
      display_name(std::move(display_name)),
      category(std::move(category)),
      preset_query_id(std::move(preset_query_id)),
      freeform_text(std::move(freeform_text)) {}

PickerSearchResult::EditorData::EditorData(
    const PickerSearchResult::EditorData&) = default;

PickerSearchResult::EditorData& PickerSearchResult::EditorData::operator=(
    const PickerSearchResult::EditorData&) = default;

PickerSearchResult::EditorData::~EditorData() = default;

bool PickerSearchResult::EditorData::operator==(const EditorData&) const =
    default;

PickerSearchResult::~PickerSearchResult() = default;

PickerSearchResult::PickerSearchResult(const PickerSearchResult&) = default;

PickerSearchResult& PickerSearchResult::operator=(const PickerSearchResult&) =
    default;

PickerSearchResult::PickerSearchResult(PickerSearchResult&&) = default;

PickerSearchResult& PickerSearchResult::operator=(PickerSearchResult&&) =
    default;

PickerSearchResult PickerSearchResult::Text(std::u16string_view text,
                                            TextData::Source source) {
  return PickerSearchResult(
      TextData(std::u16string(text), u"", ui::ImageModel(), source));
}

PickerSearchResult PickerSearchResult::Text(std::u16string_view primary_text,
                                            std::u16string_view secondary_text,
                                            ui::ImageModel icon,
                                            TextData::Source source) {
  return PickerSearchResult(TextData(std::u16string(primary_text),
                                     std::u16string(secondary_text),
                                     std::move(icon), source));
}

PickerSearchResult PickerSearchResult::SearchRequest(std::u16string_view text,
                                                     ui::ImageModel icon) {
  return PickerSearchResult(
      SearchRequestData{.text = std::u16string(text), .icon = std::move(icon)});
}

PickerSearchResult PickerSearchResult::Emoji(std::u16string_view emoji) {
  return PickerSearchResult(EmojiData{.emoji = std::u16string(emoji)});
}

PickerSearchResult PickerSearchResult::Symbol(std::u16string_view symbol) {
  return PickerSearchResult(SymbolData{.symbol = std::u16string(symbol)});
}

PickerSearchResult PickerSearchResult::Emoticon(std::u16string_view emoticon) {
  return PickerSearchResult(EmoticonData{.emoticon = std::u16string(emoticon)});
}

PickerSearchResult PickerSearchResult::Clipboard(
    base::UnguessableToken item_id,
    ClipboardData::DisplayFormat display_format,
    std::u16string display_text,
    std::optional<ui::ImageModel> display_image) {
  return PickerSearchResult(ClipboardData(item_id, display_format,
                                          std::move(display_text),
                                          std::move(display_image)));
}

PickerSearchResult PickerSearchResult::Gif(const GURL& preview_url,
                                           const GURL& preview_image_url,
                                           const gfx::Size& preview_dimensions,
                                           const GURL& full_url,
                                           const gfx::Size& full_dimensions,
                                           std::u16string content_description) {
  return PickerSearchResult(
      GifData(preview_url, preview_image_url, preview_dimensions, full_url,
              full_dimensions, std::move(content_description)));
}

PickerSearchResult PickerSearchResult::BrowsingHistory(const GURL& url,
                                                       std::u16string title,
                                                       ui::ImageModel icon) {
  return PickerSearchResult(BrowsingHistoryData{
      .url = url, .title = std::move(title), .icon = std::move(icon)});
}

PickerSearchResult PickerSearchResult::LocalFile(std::u16string title,
                                                 base::FilePath file_path) {
  return PickerSearchResult(LocalFileData{.file_path = std::move(file_path),
                                          .title = std::move(title)});
}

PickerSearchResult PickerSearchResult::DriveFile(std::u16string title,
                                                 const GURL& url,
                                                 ui::ImageModel icon) {
  return PickerSearchResult(DriveFileData{
      .title = std::move(title), .url = url, .icon = std::move(icon)});
}

PickerSearchResult PickerSearchResult::Category(PickerCategory category) {
  return PickerSearchResult(CategoryData{.category = category});
}

PickerSearchResult PickerSearchResult::Editor(
    PickerSearchResult::EditorData::Mode mode,
    std::u16string display_name,
    std::optional<chromeos::editor_menu::PresetQueryCategory> category,
    std::optional<std::string> text_query_id,
    std::optional<std::string> freeform_text) {
  return PickerSearchResult(
      EditorData(mode, std::move(display_name), std::move(category),
                 std::move(text_query_id), std::move(freeform_text)));
}

bool PickerSearchResult::operator==(const PickerSearchResult&) const = default;

const PickerSearchResult::Data& PickerSearchResult::data() const {
  return data_;
}

PickerSearchResult::PickerSearchResult(Data data) : data_(std::move(data)) {}

}  // namespace ash
