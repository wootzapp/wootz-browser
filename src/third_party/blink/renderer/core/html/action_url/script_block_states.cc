// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/html/action_url/script_block_states.h"

namespace blink {

// static
ScriptBlockStates& ScriptBlockStates::GetInstance() {
  LOG(INFO) << "AMIT ScriptBlockStates::GetInstance";
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
  LOG(INFO) << "AMIT SetButtonHandlerScriptState 1"
            << should_add_button_handler_script;
  should_add_button_handler_script = true;
  LOG(INFO) << "AMIT SetButtonHandlerScriptState 2"
            << should_add_button_handler_script;
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

  // Add Twitter-specific styles
  String twitterCSS = R"CSS(
  /* Twitter dark theme */
  .blink.x-dark {
    --blink-bg-primary: #202327;
    --blink-button: #1d9bf0;
    --blink-button-disabled: #2f3336;
    --blink-button-hover: #3087da;
    --blink-button-success: #00ae661a;
    --blink-icon-error: #ff6565;
    --blink-icon-error-hover: #ff7a7a;
    --blink-icon-primary: #6e767d;
    --blink-icon-primary-hover: #949ca4;
    --blink-icon-warning: #ffb545;
    --blink-icon-warning-hover: #ffc875;
    --blink-input-bg: #202327;
    --blink-input-stroke: #3d4144;
    --blink-input-stroke-disabled: #2f3336;
    --blink-input-stroke-error: #ff6565;
    --blink-input-stroke-hover: #6e767d;
    --blink-input-stroke-selected: #1d9bf0;
    --blink-stroke-error: #ff6565;
    --blink-stroke-primary: #1d9bf0;
    --blink-stroke-secondary: #3d4144;
    --blink-stroke-warning: #ffb545;
    --blink-text-brand: #35aeff;
    --blink-text-button: #ffffff;
    --blink-text-button-disabled: #768088;
    --blink-text-button-success: #12dc88;
    --blink-text-error: #ff6565;
    --blink-text-error-hover: #ff7a7a;
    --blink-text-input: #ffffff;
    --blink-text-input-disabled: #566470;
    --blink-text-input-placeholder: #6e767d;
    --blink-text-link: #6e767d;
    --blink-text-link-hover: #949ca4;
    --blink-text-primary: #ffffff;
    --blink-text-secondary: #949ca4;
    --blink-text-success: #12dc88;
    --blink-text-warning: #ffb545;
    --blink-text-warning-hover: #ffc875;
    --blink-transparent-error: #aa00001a;
    --blink-transparent-grey: #6e767d1a;
    --blink-transparent-warning: #a966001a;
  }
  
  /* Apply Twitter styles to elements inside x-dark container */
  .blink.x-dark .bg-bg-primary {
    background-color: var(--blink-bg-primary);
  }
  .blink.x-dark .text-text-primary {
    color: var(--blink-text-primary);
  }
  .blink.x-dark .text-text-secondary {
    color: var(--blink-text-secondary);
  }
  .blink.x-dark .bg-button {
    background-color: var(--blink-button);
  }
  .blink.x-dark .bg-button-hover:hover {
    background-color: var(--blink-button-hover);
  }
  .blink.x-dark .text-text-button {
    color: var(--blink-text-button);
  }
  .blink.x-dark .border-stroke-primary {
    border-color: var(--blink-stroke-primary);
  }
  )CSS";

  // Return combined CSS
  return css_script + twitterCSS;
}

String ScriptBlockStates::GetScriptsToAdd() {
  LOG(INFO) << "AMIT get scripts to add";
  String scripts = "";
  LOG(INFO) << "Unfurling :: " << __func__;
  LOG(INFO) << "AMIT input_script_added: " << input_script_added;
  LOG(INFO) << "AMIT radio_script_added: " << radio_script_added;
  LOG(INFO) << "AMIT checkbox_script_added: " << checkbox_script_added;
  LOG(INFO) << "AMIT form_script_added: " << form_script_added;
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
    LOG(INFO) << "AMIT button_script_added: " << button_script_added;
    LOG(INFO) << "AMIT should_add_button_handler_script: "
              << should_add_button_handler_script;
    if (!button_script_added && should_add_button_handler_script) {
      LOG(INFO) << "AMIT adding button event listener button_script_added: ";
      scripts = scripts + ButtonEventListner();
      scripts = scripts + SolanaTransactionHandler();
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
  LOG(INFO) << "AMIT running buttononclick event";
  String button_listner = (R"HTML(
      async function handleButtonClick(href) {
        try {
          // Find the button that was clicked
          const clickedButton = event.currentTarget;
          // Find the containing action block
          const buttonContainer = clickedButton.closest('[id^="::action_block_"]');
          if (!buttonContainer) return;
          
          // Save the original content
          buttonContainer._originalContent = buttonContainer.innerHTML;
          
          // Replace all buttons with a single "Processing" button that matches the original button height
          buttonContainer.innerHTML = `
            <div class="flex flex-grow basis-[calc(33.333%-2*4px)]">
              <button disabled style="background-color:rgb(68, 68, 68); color: white;" class="rounded-full text-text relative flex w-full items-center justify-center text-nowrap px-5 py-3 font-semibold transition-colors motion-reduce:transition-none bg-button-disabled text-text-button-disabled">
                <span class="min-w-0 truncate">Processing Transaction...</span>
              </button>
            </div>
          `;
          
          try {
            if (typeof window.solana !== "undefined") {
              const result = await handleSolanaTransaction(href);
              console.log("Transaction result:", result);
            } else {
              console.log("Solana Provider is not found");
            }
          } catch (txError) {
            console.error("Transaction error:", txError);
          } finally {
            // Always restore original UI when transaction completes or fails
            if (buttonContainer && buttonContainer._originalContent) {
              buttonContainer.innerHTML = buttonContainer._originalContent;
            }
          }
        } catch (error) {
          console.error("Error in handleButtonClick:", error);
          
          // Restore original UI if there's an error in the outer try block
          const buttonContainer = event.currentTarget.closest('[id^="::action_block_"]');
          if (buttonContainer && buttonContainer._originalContent) {
            buttonContainer.innerHTML = buttonContainer._originalContent;
          }
        }
      }
  )HTML");
  return button_listner;
}

String ScriptBlockStates::SolanaTransactionHandler() {
  LOG(INFO) << "AMIT running solana transaction handler";
  String solana_handler = (R"HTML(
      async function ensureSolanaWeb3() {
        if (window.solanaWeb3) return;
        
        return new Promise((resolve, reject) => {
          const script = document.createElement('script');
          script.src = 'https://unpkg.com/@solana/web3.js@latest/lib/index.iife.min.js';
          script.onload = () => {
            window.solanaWeb3 = solanaWeb3;
            console.log("Solana web3.js loaded successfully");
            resolve();
          };
          script.onerror = () => {
            reject(new Error("Failed to load Solana web3.js"));
          };
          document.head.appendChild(script);
        });
      }

      async function confirmTransaction(signature) {
        const connection = new window.solanaWeb3.Connection(
          window.solana.publicKey ? window.solana.connection.rpcEndpoint : "https://api.mainnet-beta.solana.com"
        );
        
        const latestBlockhash = await connection.getLatestBlockhash();
        
        return connection.confirmTransaction({
          signature,
          blockhash: latestBlockhash.blockhash,
          lastValidBlockHeight: latestBlockhash.lastValidBlockHeight
        });
      }

      async function handleSolanaTransaction(href) {
        try {
          const buttonContainer = document.querySelector('[id^="::action_block_"]');
          
          // Update UI to connecting state
          buttonContainer.innerHTML = `
            <div class="flex justify-center w-full">
              <div class="flex flex-grow basis-[calc(33.333%-2*4px)]" style="flex-grow: 1; flex-basis: calc(33.333% - 8px); margin: 4px; min-width: 120px;">
              <button disabled style="background-color:rgb(68, 68, 68); color: white;" class="rounded-full text-text relative flex w-full items-center justify-center text-nowrap px-5 py-3 font-semibold transition-colors motion-reduce:transition-none bg-button-disabled text-text-button-disabled">
                <span class="min-w-0 truncate">Processing Transaction...</span>
              </button>
            </div>
            </div>
          `;
          
          const response = await window.solana.connect();
          console.log("Response: ", response);
          if(window.solana.publicKey == null) {
            buttonContainer.innerHTML = buttonContainer._originalContent;
            return;
          }
          let account = window.solana.publicKey.toString();
          console.log("Using existing connection, account:", account);
          
          // Update UI to fetching transaction state
          buttonContainer.innerHTML = `
            <div class="flex justify-center w-full">
              <div class="flex flex-grow basis-[calc(33.333%-2*4px)]" style="flex-grow: 1; flex-basis: calc(33.333% - 8px); margin: 4px; min-width: 120px;">
              <button disabled style="background-color:rgb(68, 68, 68); color: white;" class="rounded-full text-text relative flex w-full items-center justify-center text-nowrap px-5 py-3 font-semibold transition-colors motion-reduce:transition-none bg-button-disabled text-text-button-disabled">
                <span class="min-w-0 truncate">Preparing Transaction...</span>
              </button>
            </div>
          `;
                
          const apiResponse = await fetch(href, {
            method: "POST",
            body: JSON.stringify({ account }),
            headers: {
              "Content-type": "application/json",
            },
          });
          const { transaction: txBase64 } = await apiResponse.json();

          console.log("Transaction: ", txBase64);

          // Ensure @solana/web3.js is loaded
          await ensureSolanaWeb3();

          console.log("AMIT solanaWeb3: ", window.solanaWeb3);

          // Deserialize the transaction
          const decodeBase64 = (str) => {
            const binary = atob(str);
            const bytes = new Uint8Array(new ArrayBuffer(binary.length));
            const half = binary.length / 2;
            for (let i = 0, j = binary.length - 1; i <= half; i++, j--) {
              bytes[i] = binary.charCodeAt(i);
              bytes[j] = binary.charCodeAt(j);
            }
            return bytes;
          };
          const txBytes = decodeBase64(txBase64);
          console.log("AMIT txBytes: ", txBytes);
          const tx = window.solanaWeb3.VersionedTransaction.deserialize(txBytes);
          console.log("AMIT tx: ", tx);
          
          // Update UI to signing state
          buttonContainer.innerHTML = `
            <div class="flex justify-center w-full">
              <div class="flex flex-grow basis-[calc(33.333%-2*4px)]" style="flex-grow: 1; flex-basis: calc(33.333% - 8px); margin: 4px; min-width: 120px;">
              <button disabled style="background-color:rgb(68, 68, 68); color: white;" class="rounded-full text-text relative flex w-full items-center justify-center text-nowrap px-5 py-3 font-semibold transition-colors motion-reduce:transition-none bg-button-disabled text-text-button-disabled">
                <span class="min-w-0 truncate">Waiting for approval...</span>
              </button>
            </div>
          `;
          
          // Sign and send the transaction
          const signedTx = await window.solana.signAndSendTransaction(tx);
          console.log("Transaction signature:", signedTx.signature);
          
          // Update UI to confirming state
          buttonContainer.innerHTML = `
            <div class="flex justify-center w-full">
              <div class="flex flex-grow basis-[calc(33.333%-2*4px)]">
              <button disabled style="background-color:rgb(68, 68, 68); color: white;" class="rounded-full text-text relative flex w-full items-center justify-center text-nowrap px-5 py-3 font-semibold transition-colors motion-reduce:transition-none bg-button-disabled text-text-button-disabled">
                <span class="min-w-0 truncate">Confirming transaction...</span>
              </button>
            </div>
          `;
          
          // Wait for transaction confirmation
          try {
            await confirmTransaction(signedTx.signature);
            
            // Update UI to success state
            buttonContainer.innerHTML = `
              <div class="flex justify-center w-full">
                <button disabled style="background-color:rgb(53, 255, 90); color: white;" class="rounded-full text-text relative flex w-full">
                  <div class="flex items-c  enter justify-center">
                    Transaction Confirmed!
                  </div>
                </button>
              </div>
            `;
            
            // Return to original UI after 2 seconds
            setTimeout(() => {
              buttonContainer.innerHTML = buttonContainer._originalContent;
            }, 2000);
            
          } catch (confirmError) {
            console.error("Error confirming transaction:", confirmError);
            
            // Update UI to error state
            buttonContainer.innerHTML = `
              <div class="flex justify-center w-full">
                <button disabled class="bg-transparent-error text-text-error rounded-full py-2 px-4 w-full">
                  <div class="flex items-center justify-center">
                    <svg class="mr-2 h-5 w-5" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor">
                      <path fill-rule="evenodd" d="M10 18a8 8 0 100-16 8 8 0 000 16zM8.707 7.293a1 1 0 00-1.414 1.414L8.586 10l-1.293 1.293a1 1 0 101.414 1.414L10 11.414l1.293 1.293a1 1 0 001.414-1.414L11.414 10l1.293-1.293a1 1 0 00-1.414-1.414L10 8.586 8.707 7.293z" clip-rule="evenodd" />
                    </svg>
                    Confirmation Failed
                  </div>
                </button>
              </div>
            `;
            
            // Return to original UI after 2 seconds
            setTimeout(() => {
              buttonContainer.innerHTML = buttonContainer._originalContent;
            }, 2000);
          }
        } catch (error) {
          console.error("Error in handleSolanaTransaction:", error);
          
          // Update UI to error state
          const buttonContainer = document.querySelector('[id^="::action_block_"]');
          if (buttonContainer) {
            buttonContainer.innerHTML = `
              <div class="flex justify-center w-full">
                <button disabled class="bg-transparent-error text-text-error rounded-full py-2 px-4 w-full">
                  <div class="flex items-center justify-center">
                    <svg class="mr-2 h-5 w-5" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor">
                      <path fill-rule="evenodd" d="M10 18a8 8 0 100-16 8 8 0 000 16zM8.707 7.293a1 1 0 00-1.414 1.414L8.586 10l-1.293 1.293a1 1 0 101.414 1.414L10 11.414l1.293 1.293a1 1 0 001.414-1.414L11.414 10l1.293-1.293a1 1 0 00-1.414-1.414L10 8.586 8.707 7.293z" clip-rule="evenodd" />
                    </svg>
                    Transaction Failed
                  </div>
                </button>
              </div>
            `;
            
            // Return to original UI after 2 seconds
            setTimeout(() => {
              buttonContainer.innerHTML = buttonContainer._originalContent;
            }, 2000);
          }
          
          throw error; // Propagate the error to be handled by the caller
        }
      }
  )HTML");
  return solana_handler;
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
            // console.log("allBlockRadioOrCheckboxSelected: " + allBlockRadioOrCheckboxSelected);


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

}  // namespace blink
