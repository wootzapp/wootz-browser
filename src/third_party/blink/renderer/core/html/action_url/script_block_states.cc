// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/html/action_url/script_block_states.h"

namespace blink {

// static
ScriptBlockStates& ScriptBlockStates::GetInstance() {
  static blink::ScriptBlockStates* script = new ScriptBlockStates();
  return *script;
}

ScriptBlockStates::ScriptBlockStates() {}

void ScriptBlockStates::SetInputHandlerScriptState() {
  should_add_input_handler_script = true;
}

void ScriptBlockStates::SetRadioButtonHandlerScriptState() {
  should_add_radio_button_handler_script = true;
}

void ScriptBlockStates::SetCheckBoxHandlerScriptState() {
  should_add_check_box_handler_script = true;
}

void ScriptBlockStates::SetButtonHandlerScriptState() {
    should_add_button_handler_script = true;
}

void ScriptBlockStates::SetSelectHandlerScriptState() {
    should_add_select_handler_script = true;
}

void ScriptBlockStates::SetFormHandlerScriptState() {
    should_add_form_handler_script = true;
}

void ScriptBlockStates::ResetScriptState() {
    should_add_input_handler_script = false;
    should_add_radio_button_handler_script = false;
    should_add_check_box_handler_script = false;
    should_add_button_handler_script = false;
    should_add_select_handler_script = false;
    should_add_form_handler_script = false;
    input_script_added = false;
    radio_script_added = false;
    checkbox_script_added = false;
    button_script_added = false;
    select_script_added = false;
    form_script_added = false;
    validiation_script_added = false;
}

String ScriptBlockStates::GetCssScriptsToAdd() {
  String css_script = (R"HTML(

    async function modifyCss(cssUrl, divSelector) {
    try {
        // Fetch the CSS file
        var response = await fetch(cssUrl);

        // Check if the fetch was successful
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }

        // Get the CSS text
        var cssText = await response.text();

        // Process the CSS
        var processedCss = processCssRules(cssText, divSelector);

        // Create a style element
        var styleElement = document.createElement('style');

        // Set the processed CSS
        styleElement.textContent = processedCss;

        // Append to document head
        document.head.appendChild(styleElement);

    } catch (error) {
        console.error('Error modifying CSS:', error);
    }
}

function processCssRules(cssText, divSelector) {
    // Regex to match CSS rules while preserving @rules and comments
    var ruleRegex = /([^{]*){([^}]*)}/g;

    // Replace function to modify selectors
    return cssText.replace(ruleRegex, (match, selectors, body) => {
        // Skip @rules and comments
        if (selectors.trim().startsWith('@') || selectors.trim().startsWith('/*')) {
            return match;
        }

        // Split multiple selectors and prepend div selector
        var modifiedSelectors = selectors
            .split(',')
            .map(selector => `${divSelector} ${selector.trim()}`)
            .join(', ');

        // Return modified rule
        return `${modifiedSelectors} {${body}}`;
    });
}


        modifyCss('https://dial.to/_next/static/css/d4d01f288ee49a49.css', '.action-block-div');
        modifyCss('https://dial.to/_next/static/css/3f18034bace3b462.css', '.action-block-div');
  )HTML");
  return css_script;
}

String ScriptBlockStates::GetScriptsToAdd() {
  String scripts = "";
  LOG(INFO) << "Unfurling :: " << __func__;
  if (!input_script_added || !radio_script_added || !button_script_added ||
      !checkbox_script_added || !form_script_added) {
    if (!validiation_script_added && (should_add_input_handler_script ||
                                      should_add_radio_button_handler_script ||
                                      should_add_select_handler_script ||
                                      should_add_check_box_handler_script ||
                                      should_add_form_handler_script)) {
      scripts = scripts + ValidationBlock();
      validiation_script_added = true;
    }
    if (!input_script_added && should_add_input_handler_script) {
      scripts = scripts + InputEventListener();
      input_script_added = true;
    }
    if (!radio_script_added && should_add_radio_button_handler_script) {
      scripts = scripts + RadioButtonEventListener();
      radio_script_added = true;
    }
    if (!checkbox_script_added && should_add_check_box_handler_script) {
      scripts = scripts + CheckboxEventListener();
      checkbox_script_added = true;
    }
    if (!button_script_added && should_add_button_handler_script) {
      scripts = scripts + ButtonEventListner();
      button_script_added = true;
    }
    if (!select_script_added && should_add_select_handler_script) {
      scripts = scripts + SelectInputEventListener();
      select_script_added = true;
    }
    if (!form_script_added && should_add_form_handler_script) {
      scripts = scripts + FormSubmitEventListener();
      form_script_added = true;
    }
  }
  return scripts;
}

String ScriptBlockStates::FormSubmitEventListener() {
  String form_submission_script = (R"HTML(
    function handleFormSubmission(event, form, href) {
      event.preventDefault(); // Prevent default form submission
      console.log("Given hef: "+href);
      // Collect form data
      var formData = new FormData(form);
      var params = new URLSearchParams();

      // Include all input elements, including checkboxes
      var inputs = form.querySelectorAll('input, select, textarea');
      inputs.forEach(input => {
        if (input.type === 'checkbox') {
              params.append(input.name, input.checked ? input.value : '');
        } else if (input.type === 'radio') {
              if (input.checked) {
                  params.append(input.name, input.value);
              }
        } else {
              params.append(input.name, input.value);
        }
      });

      // Send the data to the specified endpoint with URL parameters
      var urlWithParams = `${href}${params.toString()}`;
      console.log("urlWithParam: "+urlWithParams);
      fetch(urlWithParams, {
                method: "POST",
                body: JSON.stringify
                ({
                  "account":"3YKGasCtfeMHNR5CrFB4Y5sL6b5ukvzSoTpcUGpFJs36",
                }),
                headers: {
                  "Content-type": "application/json",
                },
        }).then((response) => response.json())
          .then((json) => console.log(json))
          .catch(error => console.error('Error:', error));
    }
  )HTML");
  return form_submission_script;
}

String ScriptBlockStates::ButtonEventListner() {
  String button_listner = (R"HTML(
      function handleButtonClick(href) {
        console.log("URL: "+href);
        // var corsProxyUrl = 'https://cors-anywhere.herokuapp.com/';
        // fetch(`${corsProxyUrl}${href}`, {
        fetch(href, {
                method: "POST",
                body: JSON.stringify
                ({
                  "account":"3YKGasCtfeMHNR5CrFB4Y5sL6b5ukvzSoTpcUGpFJs36",
                }),
                headers: {
                  "Content-type": "application/json",
                },
        }).then((response) => response.json())
          .then((json) => console.log(json))
          .catch(error => console.error('Error:', error));
      }
  )HTML");
  return button_listner;
}

String ScriptBlockStates::InputEventListener() {
  String input_event_listner = (R"HTML(
      function onInputListener(element) {
         form = element.closest('form');
         if(element.required) {
            validateFields(form);
         }
      }
  )HTML");
  return input_event_listner;
}

String ScriptBlockStates::SelectInputEventListener() {
  String input_event_listner = (R"HTML(
      function OnSelectInputListener(element) {
         form = element.closest('form');
         if(element.required) {
            validateFields(form);
         }
      }
  )HTML");
  return input_event_listner;
}

String ScriptBlockStates::RadioButtonEventListener() {
  String radio_button_event_listner = (R"HTML(
      function handleRadioSelection(element) {
         // If clicked on text, find the associated radio button
         var form = element.closest('form');
         var parentContainer = element.closest('div[class="py-1.5"]');

         if (!parentContainer) return;
         var radio = element.hasAttribute('role') && element.getAttribute('role') === 'radio' 
            ? element 
            : parentContainer.querySelector(`[role="radio"][aria-labelledby="${element.id}"]`);
         
         if (!radio) {
            return;
         }

         // Find all radio buttons
         var radioButtons = parentContainer.querySelectorAll('[role="radio"]');
         
         // Remove selected styling from all radio buttons
         radioButtons.forEach(radio => {
            radio.classList.remove('bg-input-bg-selected');
            radio.classList.add('bg-input-bg');
            radio.classList.remove('border-input-stroke-selected');
            radio.classList.add('border-input-stroke');
            
            // Find the inner circle and make it invisible
            var innerCircle = radio.querySelector('.bg-input-bg');
            if (innerCircle) {
               innerCircle.classList.add('invisible');
            }
            radio.previousElementSibling.checked=false;
         });
         
         // Add selected styling to clicked radio button
         radio.classList.add('bg-input-bg-selected');
         radio.classList.remove('bg-input-bg');
         radio.classList.add('border-input-stroke-selected');
         radio.classList.remove('border-input-stroke');
         
         // Find the inner circle and make it visible
         var clickedInnerCircle = radio.querySelector('.bg-input-bg');
         if (clickedInnerCircle) {
            clickedInnerCircle.classList.remove('invisible');
         }
         radio.previousElementSibling.checked=true;

         event.preventDefault(); // Prevent default form submission
         validateFields(form);
      }
  )HTML");
  return radio_button_event_listner;
}

String ScriptBlockStates::CheckboxEventListener() {
  String radio_button_event_listner = (R"HTML(
      function handleCheckboxSelection(element) {
          var form = element.closest('form');
          var div_element = element.closest('div[class="py-1.5"]');

          var checkbox = element.hasAttribute('role') && element.getAttribute('role') === 'checkbox'
              ? element
              : div_element.querySelector(`[role="checkbox"][aria-labelledby="${element.id}"]`);

          if (!checkbox) {
              console.log("Not checkbox");
              return;
          }

          // Toggle the checkbox state
          var isSelected = checkbox.classList.contains('border-input-stroke-selected');

          // Find the SVG elements within the checkbox
          var svgElement = checkbox.querySelector('svg');

          if (isSelected) {
              // Deselect the checkbox
              checkbox.classList.remove('border-input-stroke-selected', 'bg-input-bg-selected');
              checkbox.classList.add('border-input-stroke', 'bg-input-bg');
              checkbox.previousElementSibling.checked=false;

              // Hide the checkmark SVG and show the unchecked SVG
              svgElement.classList.remove('block');
              svgElement.classList.add('hidden');
          } else {
              // Select the checkbox
              checkbox.classList.remove('border-input-stroke', 'bg-input-bg');
              checkbox.classList.add('border-input-stroke-selected', 'bg-input-bg-selected');
              checkbox.previousElementSibling.checked=true;

              // Show the checkmark SVG and hide the unchecked SVG
              svgElement.classList.remove('hidden');
              svgElement.classList.add('block');
          }

          event.preventDefault(); // Prevent default form submission
          validateFields(form);
      }
  )HTML");
  return radio_button_event_listner;
}

String ScriptBlockStates::ValidationBlock() {
    String validaion_block = (R"HTML(
        function validateFields(form) {
            // Find the submit button
            var submitButton = form.querySelector('button[type="submit"]');
            if(!submitButton) return;


            // Check if a radioButton is selected in each radio block
            var allBlockRadioOrCheckboxSelected = true;
            var radioOrCheckboxDivs = form.querySelectorAll('div[class="py-1.5"]');
            if(radioOrCheckboxDivs){
                // console.log("radioOrCheckboxDivs length: "+radioOrCheckboxDivs.length);
                for (var i = 0, len = radioOrCheckboxDivs.length; i < len; i++) {
                    // console.log("Radio or checkbox div no: "+i);
                    var radiosOrCheckboxes = radioOrCheckboxDivs[i].querySelectorAll('div[role="radio"], span[role="checkbox"]');

                    var radioOrCheckboxSelected = Array.from(radiosOrCheckboxes).some(radioOrCheckbox =>
                        radioOrCheckbox.classList.contains('bg-input-bg-selected')
                    );
                    if(!radioOrCheckboxSelected) {
                        allBlockRadioOrCheckboxSelected = false;
                        break;
                    }
                }
            }
            // console.log("allBlockRadioOrCheckboxSelected: "+allBlockRadioOrCheckboxSelected);


            // Select all required input fields
            var inputFields = form.querySelectorAll('.border-input-stroke input[required]');
            var allFieldsFilled = true;
            if(inputFields) {
                // Check if all input fields have a value
                allFieldsFilled = Array.from(inputFields).every(input => 
                    input.value.trim() !== ''
                );
            }

            // Select all required textArea fields
            var textAreaFields = form.querySelectorAll('.border-input-stroke textarea[required]');
            var alltextAreaFieldsFilled = true;
            if(textAreaFields) {
                // Check if all textArea fields have a value
                alltextAreaFieldsFilled = Array.from(textAreaFields).every(textArea =>
                    textArea.value.trim() !== ''
                );
            }

            var selectInputs = form.querySelectorAll('select[required]');
            var allSelectOptionSelected = true;
            if(selectInputs) {
                allSelectOptionSelected = Array.from(selectInputs).every(selectInput => 
                    selectInput.value !== ':r0:'
                );
            }

            // Enable or disable the button based on validation
            if (allBlockRadioOrCheckboxSelected && allFieldsFilled && allSelectOptionSelected && alltextAreaFieldsFilled) {
                console.log("All fields are selected properly");
                submitButton.removeAttribute("disabled");
                submitButton.classList.remove('bg-button-disabled', 'text-text-button-disabled');
                submitButton.classList.add('bg-button', 'text-text-button', 'hover:bg-button-hover');
                submitButton.disabled = false;
            }
            else {
                submitButton.setAttribute("disabled", "");
                submitButton.classList.remove('bg-button', 'text-text-button', 'hover:bg-button-hover');
                submitButton.classList.add('bg-button-disabled', 'text-text-button-disabled');
                submitButton.disabled = true;
            }

        }
    )HTML");
    return validaion_block;
}




}  // namespace webrtc
