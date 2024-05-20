// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/content/renderer/form_cache.h"

#include <optional>
#include <string_view>

#include "base/memory/raw_ptr.h"
#include "base/memory/raw_ref.h"
#include "base/ranges/algorithm.h"
#include "base/strings/utf_string_conversions.h"
#include "base/test/scoped_feature_list.h"
#include "build/build_config.h"
#include "components/autofill/content/renderer/focus_test_utils.h"
#include "components/autofill/content/renderer/form_autofill_util.h"
#include "components/autofill/content/renderer/form_cache_test_api.h"
#include "components/autofill/content/renderer/test_utils.h"
#include "components/autofill/core/common/autofill_features.h"
#include "components/autofill/core/common/field_data_manager.h"
#include "components/autofill/core/common/form_field_data.h"
#include "components/autofill/core/common/mojom/autofill_types.mojom-shared.h"
#include "content/public/test/render_view_test.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/blink/public/web/web_document.h"
#include "third_party/blink/public/web/web_input_element.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "third_party/blink/public/web/web_remote_frame.h"
#include "third_party/blink/public/web/web_select_element.h"

using base::ASCIIToUTF16;
using blink::WebDocument;
using blink::WebElement;
using blink::WebInputElement;
using blink::WebSelectElement;
using blink::WebString;
using testing::AllOf;
using testing::ElementsAre;
using testing::Field;
using testing::UnorderedElementsAre;

namespace autofill {
using CheckStatus = FormFieldData::CheckStatus;

auto HasId(FormRendererId expected_id) {
  return Field("renderer_id", &FormData::renderer_id, expected_id);
}

auto HasName(std::string_view expected_name) {
  return Field("name", &FormData::name, base::ASCIIToUTF16(expected_name));
}

auto IsToken(FrameToken expected_token, int expected_predecessor) {
  return AllOf(
      Field(&FrameTokenWithPredecessor::token, expected_token),
      Field(&FrameTokenWithPredecessor::predecessor, expected_predecessor));
}

const FormData* GetFormByName(const std::vector<FormData>& forms,
                              std::string_view name) {
  for (const FormData& form : forms) {
    if (form.name == ASCIIToUTF16(name))
      return &form;
  }
  return nullptr;
}

FormCache::UpdateFormCacheResult UpdateFormCache(FormCache& form_cache) {
  return form_cache.UpdateFormCache(*base::MakeRefCounted<FieldDataManager>());
}

class FormCacheBrowserTest : public content::RenderViewTest {
 public:
  FormCacheBrowserTest() {
    focus_test_utils_ = std::make_unique<test::FocusTestUtils>(
        base::BindRepeating(&FormCacheBrowserTest::ExecuteJavaScriptForTests,
                            base::Unretained(this)));
    scoped_feature_list_.InitAndEnableFeature(
        features::kAutofillEnableSelectList);
    field_data_manager_ = base::MakeRefCounted<FieldDataManager>();
  }
  ~FormCacheBrowserTest() override = default;
  FormCacheBrowserTest(const FormCacheBrowserTest&) = delete;
  FormCacheBrowserTest& operator=(const FormCacheBrowserTest&) = delete;

  FormCache::UpdateFormCacheResult UpdateFormCache(FormCache& form_cache) {
    return form_cache.UpdateFormCache(GetFieldDataManager());
  }

 protected:
  std::string GetFocusLog() {
    return focus_test_utils_->GetFocusLog(GetMainFrame()->GetDocument());
  }

  FieldDataManager& GetFieldDataManager() const {
    return *field_data_manager_.get();
  }

  std::unique_ptr<test::FocusTestUtils> focus_test_utils_;

 private:
  base::test::ScopedFeatureList scoped_feature_list_;
  scoped_refptr<FieldDataManager> field_data_manager_;
};

TEST_F(FormCacheBrowserTest, UpdatedForms) {
  LoadHTML(R"(
    <form id="form1">
      <input type="text" name="foo1">
      <input type="text" name="foo2">
      <input type="text" name="foo3">
    </form>
    <input type="text" name="unowned_element">
  )");

  FormCache form_cache(GetMainFrame());
  FormCache::UpdateFormCacheResult forms = UpdateFormCache(form_cache);

  EXPECT_THAT(forms.updated_forms,
              UnorderedElementsAre(HasId(FormRendererId()), HasName("form1")));
  EXPECT_TRUE(forms.removed_forms.empty());

  const FormData* form1 = GetFormByName(forms.updated_forms, "form1");
  ASSERT_TRUE(form1);
  EXPECT_EQ(3u, form1->fields.size());
  EXPECT_TRUE(form1->child_frames.empty());

  const FormData* unowned_form = GetFormByName(forms.updated_forms, "");
  ASSERT_TRUE(unowned_form);
  EXPECT_EQ(1u, unowned_form->fields.size());
  EXPECT_TRUE(unowned_form->child_frames.empty());
}

TEST_F(FormCacheBrowserTest, RemovedForms) {
  LoadHTML(R"(
    <form id="form1">
      <input type="text" name="foo1">
      <input type="text" name="foo2">
      <input type="text" name="foo3">
    </form>
    <form id="form2">
      <input type="text" name="foo1">
      <input type="text" name="foo2">
      <input type="text" name="foo3">
    </form>
    <input type="text" id="unowned_element">
  )");

  FormCache form_cache(GetMainFrame());
  FormCache::UpdateFormCacheResult forms = UpdateFormCache(form_cache);

  EXPECT_THAT(forms.updated_forms,
              UnorderedElementsAre(HasId(FormRendererId()), HasName("form1"),
                                   HasName("form2")));
  EXPECT_TRUE(forms.removed_forms.empty());

  std::vector<FormRendererId> forms_renderer_id;
  for (const FormData& form : forms.updated_forms) {
    if (form.renderer_id != FormRendererId()) {
      forms_renderer_id.push_back(form.renderer_id);
    }
  }
  ASSERT_EQ(forms_renderer_id.size(), 2u);

  ExecuteJavaScriptForTests(R"(
    document.getElementById("form1").remove();
    document.getElementById("form2").innerHTML = "";
  )");

  forms = UpdateFormCache(form_cache);

  EXPECT_TRUE(forms.updated_forms.empty());
  EXPECT_THAT(forms.removed_forms,
              UnorderedElementsAre(forms_renderer_id[0], forms_renderer_id[1]));

  ExecuteJavaScriptForTests(R"(
    document.getElementById("unowned_element").remove();
  )");

  forms = UpdateFormCache(form_cache);

  EXPECT_TRUE(forms.updated_forms.empty());
  EXPECT_THAT(forms.removed_forms, ElementsAre(FormRendererId()));

  ExecuteJavaScriptForTests(R"(
    document.getElementById("form2").innerHTML = `
      <input type="text" name="foo1">
      <input type="text" name="foo2">
      <input type="text" name="foo3">
    `;
  )");

  forms = UpdateFormCache(form_cache);

  EXPECT_THAT(forms.updated_forms, ElementsAre(HasName("form2")));
  EXPECT_TRUE(forms.removed_forms.empty());

  ExecuteJavaScriptForTests(R"(
    document.getElementById("form2").innerHTML = `
      <input type="text" name="foo1">
      <input type="text" name="foo2">
      <input type="text" name="foo3">
      <input type="text" name="foo4">
    `;
  )");

  forms = UpdateFormCache(form_cache);

  EXPECT_THAT(forms.updated_forms, ElementsAre(HasName("form2")));
  EXPECT_TRUE(forms.removed_forms.empty());
}

// Test if the form gets re-extracted after a label change.
TEST_F(FormCacheBrowserTest, ExtractFormAfterDynamicFieldChange) {
  LoadHTML(R"(
    <form id="f"><input></form>
    <form id="g"> <label id="label">Name</label><input></form>
  )");

  FormCache form_cache(GetMainFrame());
  FormCache::UpdateFormCacheResult forms = UpdateFormCache(form_cache);
  EXPECT_THAT(forms.updated_forms,
              UnorderedElementsAre(HasName("f"), HasName("g")));
  EXPECT_TRUE(forms.removed_forms.empty());

  ExecuteJavaScriptForTests(R"(
    document.getElementById("label").innerHTML = "Last Name";
  )");

  forms = UpdateFormCache(form_cache);
  EXPECT_THAT(forms.updated_forms, ElementsAre(HasName("g")));
  EXPECT_TRUE(forms.removed_forms.empty());
}

TEST_F(FormCacheBrowserTest, ExtractFrames) {
  LoadHTML(R"(
    <form id="form1">
      <iframe id="frame1"></iframe>
    </form>
    <iframe id="frame2"></iframe>
  )");

  FrameToken frame1_token =
      GetFrameToken(GetMainFrame()->GetDocument(), "frame1");
  FrameToken frame2_token =
      GetFrameToken(GetMainFrame()->GetDocument(), "frame2");

  FormCache form_cache(GetMainFrame());
  FormCache::UpdateFormCacheResult forms = UpdateFormCache(form_cache);

  EXPECT_THAT(forms.updated_forms,
              UnorderedElementsAre(HasId(FormRendererId()), HasName("form1")));
  EXPECT_TRUE(forms.removed_forms.empty());

  const FormData* form1 = GetFormByName(forms.updated_forms, "form1");
  ASSERT_TRUE(form1);
  EXPECT_TRUE(form1->fields.empty());
  EXPECT_THAT(form1->child_frames, ElementsAre(IsToken(frame1_token, -1)));

  const FormData* unowned_form = GetFormByName(forms.updated_forms, "");
  ASSERT_TRUE(unowned_form);
  EXPECT_TRUE(unowned_form->fields.empty());
  EXPECT_THAT(unowned_form->child_frames,
              ElementsAre(AllOf(IsToken(frame2_token, -1))));
}

TEST_F(FormCacheBrowserTest, ExtractFormsTwice) {
  LoadHTML(R"(
    <form id="form1">
      <input type="text" name="foo1">
      <input type="text" name="foo2">
      <input type="text" name="foo3">
    </form>
    <input type="text" name="unowned_element">
  )");

  FormCache form_cache(GetMainFrame());
  FormCache::UpdateFormCacheResult forms = UpdateFormCache(form_cache);

  EXPECT_THAT(forms.updated_forms,
              UnorderedElementsAre(HasId(FormRendererId()), HasName("form1")));
  EXPECT_TRUE(forms.removed_forms.empty());

  forms = UpdateFormCache(form_cache);
  // As nothing has changed, there are no new or removed forms.
  EXPECT_TRUE(forms.updated_forms.empty());
  EXPECT_TRUE(forms.removed_forms.empty());
}

TEST_F(FormCacheBrowserTest, ExtractFramesTwice) {
  LoadHTML(R"(
    <form id="form1">
      <iframe></iframe>
    </form>
    <iframe></iframe>
  )");

  FormCache form_cache(GetMainFrame());
  FormCache::UpdateFormCacheResult forms = UpdateFormCache(form_cache);

  EXPECT_THAT(forms.updated_forms,
              UnorderedElementsAre(HasId(FormRendererId()), HasName("form1")));
  EXPECT_TRUE(forms.removed_forms.empty());

  forms = UpdateFormCache(form_cache);
  // As nothing has changed, there are no new or removed forms.
  EXPECT_TRUE(forms.updated_forms.empty());
  EXPECT_TRUE(forms.removed_forms.empty());
}

// TODO(crbug.com/40144964) Adjust expectations when we omit invisible iframes.
TEST_F(FormCacheBrowserTest, ExtractFramesAfterVisibilityChange) {
  LoadHTML(R"(
    <form id="form1">
      <iframe id="frame1" style="display: none;"></iframe>
      <iframe id="frame2" style="display: none;"></iframe>
    </form>
    <iframe id="frame3" style="display: none;"></iframe>
  )");

  WebElement iframe1 = GetElementById(GetMainFrame()->GetDocument(), "frame1");
  WebElement iframe2 = GetElementById(GetMainFrame()->GetDocument(), "frame2");
  WebElement iframe3 = GetElementById(GetMainFrame()->GetDocument(), "frame3");

  auto GetSize = [](const WebElement& element) {
    gfx::Rect bounds = element.BoundsInWidget();
    return bounds.width() * bounds.height();
  };

  ASSERT_LE(GetSize(iframe1), 0);
  ASSERT_LE(GetSize(iframe2), 0);
  ASSERT_LE(GetSize(iframe3), 0);

  FormCache form_cache(GetMainFrame());
  FormCache::UpdateFormCacheResult forms = UpdateFormCache(form_cache);
  EXPECT_THAT(forms.updated_forms,
              UnorderedElementsAre(HasId(FormRendererId()), HasName("form1")));
  EXPECT_TRUE(forms.removed_forms.empty());

  iframe1.SetAttribute("style", "display: block;");
  iframe2.SetAttribute("style", "display: block;");
  iframe3.SetAttribute("style", "display: block;");

  ASSERT_GT(GetSize(iframe1), 0);
  ASSERT_GT(GetSize(iframe2), 0);
  ASSERT_GT(GetSize(iframe3), 0);

  forms = UpdateFormCache(form_cache);
  EXPECT_TRUE(forms.updated_forms.empty());
  EXPECT_TRUE(forms.removed_forms.empty());

  iframe2.SetAttribute("style", "display: none;");
  iframe3.SetAttribute("style", "display: none;");

  ASSERT_GT(GetSize(iframe1), 0);
  ASSERT_LE(GetSize(iframe2), 0);
  ASSERT_LE(GetSize(iframe3), 0);

  forms = UpdateFormCache(form_cache);
  EXPECT_TRUE(forms.updated_forms.empty());
  EXPECT_TRUE(forms.removed_forms.empty());
}

TEST_F(FormCacheBrowserTest, ExtractFormsAfterModification) {
  LoadHTML(R"(
    <form id="form1">
      <input type="text" name="foo1">
      <input type="text" name="foo2">
      <input type="text" name="foo3">
    </form>
    <input type="text" name="unowned_element">
  )");

  FormCache form_cache(GetMainFrame());
  FormCache::UpdateFormCacheResult forms = UpdateFormCache(form_cache);
  EXPECT_THAT(forms.updated_forms,
              UnorderedElementsAre(HasId(FormRendererId()), HasName("form1")));
  EXPECT_TRUE(forms.removed_forms.empty());

  // Append an input element to the form and to the list of unowned inputs.
  ExecuteJavaScriptForTests(R"(
    var new_input_1 = document.createElement("input");
    new_input_1.setAttribute("type", "text");
    new_input_1.setAttribute("name", "foo4");

    var form1 = document.getElementById("form1");
    form1.appendChild(new_input_1);

    var new_input_2 = document.createElement("input");
    new_input_2.setAttribute("type", "text");
    new_input_2.setAttribute("name", "unowned_element_2");
    document.body.appendChild(new_input_2);
  )");

  forms = UpdateFormCache(form_cache);
  EXPECT_THAT(forms.updated_forms,
              UnorderedElementsAre(HasId(FormRendererId()), HasName("form1")));
  EXPECT_TRUE(forms.removed_forms.empty());

  const FormData* form1 = GetFormByName(forms.updated_forms, "form1");
  ASSERT_TRUE(form1);
  EXPECT_EQ(4u, form1->fields.size());

  const FormData* unowned_form = GetFormByName(forms.updated_forms, "");
  ASSERT_TRUE(unowned_form);
  EXPECT_EQ(2u, unowned_form->fields.size());
}

struct FillElementData {
  const raw_ref<blink::WebFormControlElement> element;
  std::u16string value;
};

FormFieldData* FindFieldByName(FormData& form_data,
                               blink::WebString search_field_name) {
  auto it = base::ranges::find(form_data.fields, search_field_name.Utf16(),
                               &FormFieldData::name);
  return it != form_data.fields.end() ? &*it : nullptr;
}

// Fills the fields referenced in `form_fill_data`. Fills `checkbox_element`, if
// non-null.
void FillAndCheckState(
    const WebDocument& document,
    const FormData& form_data,
    FieldDataManager& field_data_manager,
    const std::vector<FillElementData>& form_to_fill,
    std::optional<blink::WebInputElement> checkbox_element = std::nullopt,
    CheckStatus fill_checkbox_check_status =
        CheckStatus::kCheckableButUnchecked) {
  FormData values_to_fill = form_data;
  for (const FillElementData& field_to_fill : form_to_fill) {
    FormFieldData* value_to_fill = FindFieldByName(
        values_to_fill, field_to_fill.element->NameForAutofill());
    ASSERT_TRUE(value_to_fill != nullptr);
    value_to_fill->set_value(field_to_fill.value);
    value_to_fill->set_is_autofilled(true);
  }

  if (checkbox_element) {
    FormFieldData* value_to_fill =
        FindFieldByName(values_to_fill, checkbox_element->NameForAutofill());
    ASSERT_TRUE(value_to_fill != nullptr);
    value_to_fill->set_check_status(fill_checkbox_check_status);
    value_to_fill->set_is_autofilled(true);
  }

  std::vector<FormFieldData::FillData> fields_to_fill;
  fields_to_fill.reserve(values_to_fill.fields.size());
  for (const FormFieldData& field : values_to_fill.fields) {
    fields_to_fill.emplace_back(field);
  }
  form_util::ApplyFieldsAction(
      document, fields_to_fill, mojom::FormActionType::kFill,
      mojom::ActionPersistence::kFill, field_data_manager);

  for (const FillElementData& field_to_fill : form_to_fill) {
    EXPECT_EQ(field_to_fill.value, field_to_fill.element->Value().Utf16());
  }

  if (checkbox_element) {
    bool expect_checked = (fill_checkbox_check_status == CheckStatus::kChecked);
    EXPECT_EQ(expect_checked, checkbox_element->IsChecked());
  }
}

// Tests that correct focus, change and blur events are emitted during the
// autofilling and clearing of the form with an initially focused element.
TEST_F(FormCacheBrowserTest,
       VerifyFocusAndBlurEventsAfterAutofillAndClearingWithFocusElement) {
  // Load a form.
  LoadHTML(
      "<html><form id='myForm'>"
      "<label>First Name:</label><input id='fname' name='0'><br>"
      "<label>Last Name:</label> <input id='lname' name='1'><br>"
      "</form></html>");

  focus_test_utils_->SetUpFocusLogging();
  focus_test_utils_->FocusElement("fname");

  FormCache form_cache(GetMainFrame());
  FormCache::UpdateFormCacheResult forms = UpdateFormCache(form_cache);

  EXPECT_THAT(forms.updated_forms,
              UnorderedElementsAre(HasId(FormRendererId()), HasName("myForm")));
  EXPECT_TRUE(forms.removed_forms.empty());

  std::vector<FormFieldData::FillData> values_to_fill;
  values_to_fill.reserve(forms.updated_forms[0].fields.size());
  for (const FormFieldData& field : forms.updated_forms[0].fields) {
    values_to_fill.emplace_back(field);
  }
  values_to_fill[0].value = u"John";
  values_to_fill[0].is_autofilled = true;
  values_to_fill[1].value = u"Smith";
  values_to_fill[1].is_autofilled = true;

  auto fname =
      GetFormControlElementById(GetMainFrame()->GetDocument(), "fname");

  // Simulate filling the form using Autofill.
  form_util::ApplyFieldsAction(GetMainFrame()->GetDocument(), values_to_fill,
                               mojom::FormActionType::kFill,
                               mojom::ActionPersistence::kFill,
                               GetFieldDataManager());

  // Expected Result in order:
  // - from filling
  //  * Change fname
  //  * Blur fname
  //  * Focus lname
  //  * Change lname
  //  * Blur lname
  //  * Focus fname
  EXPECT_EQ(GetFocusLog(), "c0b0f1c1b1f0");
}

// Test that the FormCache does not contain empty forms.
TEST_F(FormCacheBrowserTest, DoNotStoreEmptyForms) {
  LoadHTML(R"(<form></form>)");

  FormCache form_cache(GetMainFrame());
  FormCache::UpdateFormCacheResult forms = UpdateFormCache(form_cache);

  EXPECT_TRUE(forms.updated_forms.empty());
  EXPECT_TRUE(forms.removed_forms.empty());

  EXPECT_EQ(1u, GetMainFrame()->GetDocument().GetTopLevelForms().size());
  EXPECT_EQ(0u, test_api(form_cache).extracted_forms_size());
}

// Test that the FormCache never contains more than |kMaxExtractableFields|
// non-empty extracted forms.
TEST_F(FormCacheBrowserTest, FormCacheSizeUpperBound) {
  // Create a HTML page that contains `kMaxExtractableFields + 1` non-empty
  // forms.
  std::string html;
  for (unsigned int i = 0; i < kMaxExtractableFields + 1; i++) {
    html += "<form><input></form>";
  }
  LoadHTML(html.c_str());

  FormCache form_cache(GetMainFrame());
  FormCache::UpdateFormCacheResult forms = UpdateFormCache(form_cache);

  EXPECT_EQ(forms.updated_forms.size(), kMaxExtractableFields);
  EXPECT_TRUE(forms.removed_forms.empty());

  EXPECT_EQ(kMaxExtractableFields + 1,
            GetMainFrame()->GetDocument().GetTopLevelForms().size());
  EXPECT_EQ(kMaxExtractableFields, test_api(form_cache).extracted_forms_size());
}

// Test that FormCache::UpdateFormCache() limits the number of total fields by
// skipping any additional forms.
TEST_F(FormCacheBrowserTest, FieldLimit) {
  std::string html;
  for (unsigned int i = 0; i < kMaxExtractableFields + 1; i++) {
    html += "<form><input></form>";
  }
  LoadHTML(html.c_str());

  ASSERT_EQ(kMaxExtractableFields + 1,
            GetMainFrame()->GetDocument().GetTopLevelForms().size());

  FormCache form_cache(GetMainFrame());
  FormCache::UpdateFormCacheResult forms = UpdateFormCache(form_cache);

  EXPECT_EQ(kMaxExtractableFields, forms.updated_forms.size());
  EXPECT_TRUE(forms.removed_forms.empty());
}

// Test that FormCache::UpdateFormCache() limits the number of total frames by
// clearing their frames and skipping the then-empty forms.
TEST_F(FormCacheBrowserTest, FrameLimit) {
  std::string html;
  for (unsigned int i = 0; i < kMaxExtractableChildFrames + 1; i++) {
    html += "<form><iframe></iframe></form>";
  }
  LoadHTML(html.c_str());

  ASSERT_EQ(kMaxExtractableChildFrames + 1,
            GetMainFrame()->GetDocument().GetTopLevelForms().size());

  FormCache form_cache(GetMainFrame());
  FormCache::UpdateFormCacheResult forms = UpdateFormCache(form_cache);

  EXPECT_EQ(kMaxExtractableChildFrames, forms.updated_forms.size());
  EXPECT_TRUE(forms.removed_forms.empty());
}

// Test that FormCache::UpdateFormCache() limits the number of total fields and
// total frames:
// - the forms [0, kMaxExtractableChildFrames) should be unchanged,
// - the forms [kMaxExtractableChildFrames, kMaxExtractableFields) should have
//   empty FormData::child_frames,
// - the forms [kMaxExtractableFields, end) should be skipped.
// TODO(crbug.com/40816477): Flaky on android.
#if BUILDFLAG(IS_ANDROID)
#define MAYBE_FieldAndFrameLimit DISABLED_FieldAndFrameLimit
#else
#define MAYBE_FieldAndFrameLimit FieldAndFrameLimit
#endif
TEST_F(FormCacheBrowserTest, MAYBE_FieldAndFrameLimit) {
  ASSERT_LE(kMaxExtractableChildFrames, kMaxExtractableFields);

  std::string html;
  for (unsigned int i = 0; i < kMaxExtractableFields + 1; i++) {
    html += "<form><input><iframe></iframe></form>";
  }
  LoadHTML(html.c_str());

  ASSERT_EQ(kMaxExtractableFields + 1,
            GetMainFrame()->GetDocument().GetTopLevelForms().size());

  FormCache form_cache(GetMainFrame());
  FormCache::UpdateFormCacheResult forms = UpdateFormCache(form_cache);

  EXPECT_EQ(forms.updated_forms.size(), kMaxExtractableFields);
  EXPECT_TRUE(base::ranges::none_of(forms.updated_forms,
                                    &std::vector<FormFieldData>::empty,
                                    &FormData::fields));
  EXPECT_TRUE(base::ranges::none_of(
      base::make_span(forms.updated_forms).first(kMaxExtractableChildFrames),
      &std::vector<FrameTokenWithPredecessor>::empty, &FormData::child_frames));
  EXPECT_TRUE(base::ranges::all_of(
      base::make_span(forms.updated_forms).subspan(kMaxExtractableChildFrames),
      &std::vector<FrameTokenWithPredecessor>::empty, &FormData::child_frames));

  EXPECT_TRUE(forms.removed_forms.empty());
}

}  // namespace autofill
