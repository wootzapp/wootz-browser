// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'chrome-untrusted://compose/app.js';

import type {ComposeAppElement} from 'chrome-untrusted://compose/app.js';
import {Length, Tone, UserFeedback} from 'chrome-untrusted://compose/compose.mojom-webui.js';
import {ComposeApiProxyImpl} from 'chrome-untrusted://compose/compose_api_proxy.js';
import {ComposeStatus} from 'chrome-untrusted://compose/compose_enums.mojom-webui.js';
import {assertEquals} from 'chrome-untrusted://webui-test/chai_assert.js';
import {flushTasks} from 'chrome-untrusted://webui-test/polymer_test_util.js';

import {TestComposeApiProxy} from './test_compose_api_proxy.js';

suite('ComposeApp', function() {
  let testProxy: TestComposeApiProxy;

  async function createApp(): Promise<ComposeAppElement> {
    const app = document.createElement('compose-app');
    document.body.appendChild(app);

    await testProxy.whenCalled('requestInitialState');
    await flushTasks();
    return app;
  }

  setup(async () => {
    testProxy = new TestComposeApiProxy();
    ComposeApiProxyImpl.setInstance(testProxy);
    document.body.innerHTML = window.trustedTypes!.emptyHTML;
  });

  function mockResponse(
      result: string = 'some response',
      status: ComposeStatus = ComposeStatus.kOk): Promise<void> {
    testProxy.remote.responseReceived({
      status: status,
      undoAvailable: false,
      result,
      onDeviceEvaluationUsed: false,
    });
    return testProxy.remote.$.flushForTesting();
  }

  test('RefocusesInputOnInvalidate', async () => {
    const app = await createApp();
    app.$.textarea.value = 'short';
    app.$.textarea.dispatchEvent(new CustomEvent('value-changed'));
    app.$.submitButton.focus();
    app.$.submitButton.click();
    await flushTasks();
    assertEquals(app.$.textarea, app.shadowRoot!.activeElement);
  });

  test('FocusesEditInput', async () => {
    testProxy.setOpenMetadata({}, {
      webuiState: JSON.stringify({
        input: 'some input',
        isEditingSubmittedInput: true,
      }),
    });
    const app = await createApp();
    assertEquals(app.$.editTextarea, app.shadowRoot!.activeElement);
  });

  test('FocusesRefreshButtonAfterRefreshRewrite', async () => {
    const app = await createApp();
    app.$.textarea.value = 'test value one';
    app.$.submitButton.click();
    await mockResponse();

    app.$.refreshButton.click();
    await testProxy.whenCalled('rewrite');
    await mockResponse('refreshed');

    assertEquals(app.$.refreshButton, app.shadowRoot!.activeElement);
  });

  test('FocusesEditInputAfterSubmitInput', async () => {
    const app = await createApp();
    app.$.textarea.value = 'test value one';
    app.$.submitButton.click();

    await testProxy.whenCalled('compose');
    await mockResponse();

    assertEquals(app.$.textarea, app.shadowRoot!.activeElement);
  });

  test('FocusesLengthMenuAfterLengthRewrite', async () => {
    const app = await createApp();
    app.$.textarea.value = 'test value';
    app.$.submitButton.click();
    await mockResponse();

    app.$.lengthMenu.value = `${Length.kShorter}`;
    app.$.lengthMenu.dispatchEvent(new CustomEvent('change'));

    await testProxy.whenCalled('rewrite');
    await mockResponse();

    assertEquals(app.$.lengthMenu, app.shadowRoot!.activeElement);
  });

  test('FocusesToneMenuAfterToneRewrite', async () => {
    const app = await createApp();
    app.$.textarea.value = 'test value';
    app.$.submitButton.click();
    await mockResponse();

    app.$.toneMenu.value = `${Tone.kCasual}`;
    app.$.toneMenu.dispatchEvent(new CustomEvent('change'));

    await testProxy.whenCalled('rewrite');
    await mockResponse();

    assertEquals(app.$.toneMenu, app.shadowRoot!.activeElement);
  });

  test('FocusesUndoButtonAfterUndoClick', async () => {
    // Set up initial state to show undo button and mock up a previous state.
    document.body.innerHTML = window.trustedTypes!.emptyHTML;
    testProxy.setOpenMetadata({}, {
      hasPendingRequest: false,
      response: {
        status: ComposeStatus.kOk,
        undoAvailable: true,
        result: 'here is a result',
        onDeviceEvaluationUsed: false,
      },
    });
    testProxy.setUndoResponse({
      hasPendingRequest: false,
      response: {
        status: ComposeStatus.kOk,
        undoAvailable: false,
        result: 'some undone result',
        onDeviceEvaluationUsed: false,
      },
      webuiState: JSON.stringify({
        input: 'my old input',
        selectedLength: Number(Length.kLonger),
        selectedTone: Number(Tone.kCasual),
      }),
      feedback: UserFeedback.kUserFeedbackUnspecified,
    });
    const appWithUndo = document.createElement('compose-app');
    document.body.appendChild(appWithUndo);
    await testProxy.whenCalled('requestInitialState');

    // CLick undo.
    appWithUndo.$.undoButton.click();
    await testProxy.whenCalled('undo');

    assertEquals(
        appWithUndo.$.undoButton, appWithUndo.shadowRoot!.activeElement);
  });

});
