// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import {webUIListenerCallback} from 'chrome://resources/js/cr.js';
import {loadTimeData} from 'chrome://resources/js/load_time_data.js';
import {stringToMojoString16} from 'chrome://resources/js/mojo_type_util.js';
import type {CrInputElement, SyncInfo, Tab, TabOrganizationPageElement, TabOrganizationResultsElement, TabOrganizationSession} from 'chrome://tab-search.top-chrome/tab_search.js';
import {TabOrganizationError, TabOrganizationState, TabSearchApiProxyImpl, TabSearchSyncBrowserProxyImpl} from 'chrome://tab-search.top-chrome/tab_search.js';
import {assertEquals, assertFalse, assertTrue} from 'chrome://webui-test/chai_assert.js';
import {flushTasks} from 'chrome://webui-test/polymer_test_util.js';
import {isVisible} from 'chrome://webui-test/test_util.js';

import {TestTabSearchApiProxy} from './test_tab_search_api_proxy.js';
import {TestTabSearchSyncBrowserProxy} from './test_tab_search_sync_browser_proxy.js';

suite('TabOrganizationPageTest', () => {
  let tabOrganizationPage: TabOrganizationPageElement;
  let tabOrganizationResults: TabOrganizationResultsElement;
  let testApiProxy: TestTabSearchApiProxy;
  let testSyncProxy: TestTabSearchSyncBrowserProxy;

  async function tabOrganizationPageSetup(syncInfo: SyncInfo = {
    syncing: true,
    syncingHistory: true,
    paused: false,
  }) {
    testApiProxy = new TestTabSearchApiProxy();
    const session = createSession();
    testApiProxy.setSession(session);
    TabSearchApiProxyImpl.setInstance(testApiProxy);

    testSyncProxy = new TestTabSearchSyncBrowserProxy();
    testSyncProxy.syncInfo = syncInfo;
    TabSearchSyncBrowserProxyImpl.setInstance(testSyncProxy);

    tabOrganizationPage = document.createElement('tab-organization-page');

    document.body.innerHTML = window.trustedTypes!.emptyHTML;
    document.body.appendChild(tabOrganizationPage);
    tabOrganizationPage.setSessionForTesting(session);
    await flushTasks();
  }

  async function tabOrganizationResultsSetup() {
    testApiProxy = new TestTabSearchApiProxy();
    const session = createSession();
    testApiProxy.setSession(session);
    TabSearchApiProxyImpl.setInstance(testApiProxy);

    testSyncProxy = new TestTabSearchSyncBrowserProxy();
    TabSearchSyncBrowserProxyImpl.setInstance(testSyncProxy);

    tabOrganizationResults = document.createElement('tab-organization-results');
    tabOrganizationResults.multiTabOrganization = false;
    tabOrganizationResults.session = session;

    document.body.innerHTML = window.trustedTypes!.emptyHTML;
    document.body.appendChild(tabOrganizationResults);
    await flushTasks();
  }

  function createTab(override: Partial<Tab> = {}): Tab {
    return Object.assign(
        {
          active: false,
          alertStates: [],
          index: -1,
          faviconUrl: null,
          tabId: -1,
          groupId: -1,
          pinned: false,
          title: '',
          url: {url: 'about:blank'},
          isDefaultFavicon: false,
          showIcon: false,
          lastActiveTimeTicks: -1,
          lastActiveElapsedText: '',
        },
        override);
  }

  function createSession(override: Partial<TabOrganizationSession> = {}):
      TabOrganizationSession {
    return Object.assign(
        {
          sessionId: 1,
          state: TabOrganizationState.kNotStarted,
          organizations: [{
            organizationId: 1,
            name: stringToMojoString16('foo'),
            tabs: [
              createTab({title: 'Tab 1', url: {url: 'https://tab-1.com/'}}),
              createTab({title: 'Tab 2', url: {url: 'https://tab-2.com/'}}),
              createTab({title: 'Tab 3', url: {url: 'https://tab-3.com/'}}),
            ],
          }],
          error: TabOrganizationError.kNone,
        },
        override);
  }

  function createMultiOrganizationSession(
      count: number, override: Partial<TabOrganizationSession> = {}) {
    const organizations: Object[] = [];
    for (let i = 0; i < count; i++) {
      organizations.push(
          {
            organizationId: i,
            name: stringToMojoString16('Organization ' + i),
            tabs: [
              createTab({
                title: 'Tab 1 Organization ' + i,
                url: {url: 'https://tab-1.com/'},
              }),
              createTab({
                title: 'Tab 2 Organization ' + i,
                url: {url: 'https://tab-2.com/'},
              }),
              createTab({
                title: 'Tab 3 Organization ' + i,
                url: {url: 'https://tab-3.com/'},
              }),
            ],
          },
      );
    }

    return Object.assign(
        {
          sessionId: 1,
          state: TabOrganizationState.kSuccess,
          organizations: organizations,
          error: TabOrganizationError.kNone,
        },
        override);
  }

  test('Organize tabs starts request', async () => {
    await tabOrganizationPageSetup();
    assertEquals(0, testApiProxy.getCallCount('requestTabOrganization'));
    const notStarted = tabOrganizationPage.shadowRoot!.querySelector(
        'tab-organization-not-started');
    assertTrue(!!notStarted);
    assertTrue(isVisible(notStarted));

    const organizeTabsButton =
        notStarted.shadowRoot!.querySelector('cr-button');
    assertTrue(!!organizeTabsButton);
    organizeTabsButton.click();

    assertEquals(1, testApiProxy.getCallCount('requestTabOrganization'));
  });

  test('Single organization input blurs on enter', async () => {
    await tabOrganizationResultsSetup();
    const group = tabOrganizationResults.shadowRoot!.querySelector(
        'tab-organization-group');
    assertTrue(!!group);
    const input = group.shadowRoot!.querySelector<CrInputElement>(
        '#singleOrganizationInput');
    assertTrue(!!input);
    assertFalse(input.hasAttribute('focused_'));

    input.focus();
    await input.updateComplete;
    assertTrue(input.hasAttribute('focused_'));

    input.dispatchEvent(new KeyboardEvent('keydown', {key: 'Enter'}));
    await input.updateComplete;
    assertFalse(input.hasAttribute('focused_'));
  });

  test('Multi organization input toggles on enter/edit', async () => {
    await tabOrganizationResultsSetup();
    tabOrganizationResults.multiTabOrganization = true;
    await flushTasks();

    const group = tabOrganizationResults.shadowRoot!.querySelector(
        'tab-organization-group');
    assertTrue(!!group);
    const input =
        group.shadowRoot!.querySelector<HTMLElement>('#multiOrganizationInput');
    assertTrue(!!input);

    assertTrue(isVisible(input));

    input.dispatchEvent(new KeyboardEvent('keydown', {key: 'Enter'}));
    await flushTasks();

    assertFalse(isVisible(input));

    const editButton =
        group.shadowRoot!.querySelector<HTMLElement>('.icon-edit');
    assertTrue(!!editButton);
    assertTrue(isVisible(editButton));

    editButton.click();
    await flushTasks();

    assertTrue(isVisible(input));
    assertFalse(isVisible(editButton));
  });

  test('Tab close removes from tab list', async () => {
    await tabOrganizationPageSetup();

    testApiProxy.getCallbackRouterRemote().tabOrganizationSessionUpdated(
        createSession({state: TabOrganizationState.kSuccess}));
    await flushTasks();

    const results = tabOrganizationPage.shadowRoot!.querySelector(
        'tab-organization-results');
    assertTrue(!!results);
    const group = results.shadowRoot!.querySelector('tab-organization-group');
    assertTrue(!!group);

    assertEquals(0, testApiProxy.getCallCount('removeTabFromOrganization'));

    const tabRows = group.shadowRoot!.querySelectorAll('tab-search-item');
    assertTrue(!!tabRows);
    assertEquals(3, tabRows.length);

    const cancelButton =
        tabRows[0]!.shadowRoot!.querySelector('cr-icon-button');
    assertTrue(!!cancelButton);
    cancelButton.click();

    assertEquals(1, testApiProxy.getCallCount('removeTabFromOrganization'));
  });

  test('Arrow keys traverse focus in results list', async () => {
    await tabOrganizationResultsSetup();

    const group = tabOrganizationResults.shadowRoot!.querySelector(
        'tab-organization-group');
    assertTrue(!!group);
    const tabRows = group.shadowRoot!.querySelectorAll('tab-search-item');
    assertTrue(!!tabRows);
    assertEquals(3, tabRows.length);

    const closeButton0 =
        tabRows[0]!.shadowRoot!.querySelector(`cr-icon-button`);
    assertTrue(!!closeButton0);
    const closeButton1 =
        tabRows[1]!.shadowRoot!.querySelector(`cr-icon-button`);
    assertTrue(!!closeButton1);
    const closeButton2 =
        tabRows[2]!.shadowRoot!.querySelector(`cr-icon-button`);
    assertTrue(!!closeButton2);

    closeButton0.focus();

    assertTrue(closeButton0.matches(':focus'));
    assertFalse(closeButton1.matches(':focus'));
    assertFalse(closeButton2.matches(':focus'));

    group.$.selector.dispatchEvent(
        new KeyboardEvent('keydown', {key: 'ArrowUp'}));

    assertFalse(closeButton0.matches(':focus'));
    assertFalse(closeButton1.matches(':focus'));
    assertTrue(closeButton2.matches(':focus'));

    group.$.selector.dispatchEvent(
        new KeyboardEvent('keydown', {key: 'ArrowDown'}));

    assertTrue(closeButton0.matches(':focus'));
    assertFalse(closeButton1.matches(':focus'));
    assertFalse(closeButton2.matches(':focus'));
  });

  test('Arrow keys traverse focus in footer', async () => {
    await tabOrganizationResultsSetup();

    const focusableElement0 = tabOrganizationResults.$.learnMore;
    const focusableElement1 =
        tabOrganizationResults.$.feedbackButtons.$.thumbsUp;
    const focusableElement2 =
        tabOrganizationResults.$.feedbackButtons.$.thumbsDown;
    focusableElement0.focus();

    assertTrue(focusableElement0.matches(':focus'));
    assertFalse(focusableElement1.matches(':focus'));
    assertFalse(focusableElement2.matches(':focus'));

    const feedback =
        tabOrganizationResults.shadowRoot!.querySelector('.feedback');
    assertTrue(!!feedback);
    feedback.dispatchEvent(new KeyboardEvent('keydown', {key: 'ArrowLeft'}));

    assertFalse(focusableElement0.matches(':focus'));
    assertFalse(focusableElement1.matches(':focus'));
    assertTrue(focusableElement2.matches(':focus'));

    feedback.dispatchEvent(new KeyboardEvent('keydown', {key: 'ArrowRight'}));

    assertTrue(focusableElement0.matches(':focus'));
    assertFalse(focusableElement1.matches(':focus'));
    assertFalse(focusableElement2.matches(':focus'));
  });

  test('Single organization create group accepts organization', async () => {
    await tabOrganizationPageSetup();

    testApiProxy.getCallbackRouterRemote().tabOrganizationSessionUpdated(
        createSession({state: TabOrganizationState.kSuccess}));

    assertEquals(0, testApiProxy.getCallCount('acceptTabOrganization'));

    const results = tabOrganizationPage.shadowRoot!.querySelector(
        'tab-organization-results');
    assertTrue(!!results);
    const group = results.shadowRoot!.querySelector('tab-organization-group');
    assertTrue(!!group);
    const actions =
        group.shadowRoot!.querySelector('tab-organization-results-actions');
    assertTrue(!!actions);
    const createGroupButton = actions.shadowRoot!.querySelector('cr-button');
    assertTrue(!!createGroupButton);
    createGroupButton.click();
    await flushTasks();

    assertEquals(1, testApiProxy.getCallCount('acceptTabOrganization'));
  });

  test(
      'Multi organization create groups accepts all organizations',
      async () => {
        loadTimeData.overrideValues({
          multiTabOrganizationEnabled: true,
        });

        await tabOrganizationPageSetup();

        const organizationCount = 3;
        testApiProxy.getCallbackRouterRemote().tabOrganizationSessionUpdated(
            createMultiOrganizationSession(organizationCount));
        await flushTasks();

        assertEquals(0, testApiProxy.getCallCount('acceptTabOrganization'));

        const results = tabOrganizationPage.shadowRoot!.querySelector(
            'tab-organization-results');
        assertTrue(!!results);
        const actions = results.shadowRoot!.querySelector(
            'tab-organization-results-actions');
        assertTrue(!!actions);
        const createGroupsButton =
            actions.shadowRoot!.querySelector<HTMLElement>('#createButton');
        assertTrue(!!createGroupsButton);
        createGroupsButton.click();
        await flushTasks();

        assertEquals(
            organizationCount,
            testApiProxy.getCallCount('acceptTabOrganization'));
      });

  test('Group cancel button rejects organization', async () => {
    loadTimeData.overrideValues({
      multiTabOrganizationEnabled: true,
    });

    await tabOrganizationPageSetup();

    testApiProxy.getCallbackRouterRemote().tabOrganizationSessionUpdated(
        createMultiOrganizationSession(
            3, {state: TabOrganizationState.kSuccess}));
    await flushTasks();

    assertEquals(0, testApiProxy.getCallCount('rejectTabOrganization'));

    const results = tabOrganizationPage.shadowRoot!.querySelector(
        'tab-organization-results');
    assertTrue(!!results);
    const group = results.shadowRoot!.querySelector('tab-organization-group');
    assertTrue(!!group);
    const cancelButton =
        group.shadowRoot!.querySelector<HTMLElement>('#rejectButton');
    assertTrue(!!cancelButton);
    cancelButton.click();
    await flushTasks();

    assertEquals(1, testApiProxy.getCallCount('rejectTabOrganization'));
  });

  test('Clear rejects session', async () => {
    loadTimeData.overrideValues({
      multiTabOrganizationEnabled: true,
    });

    await tabOrganizationPageSetup();

    testApiProxy.getCallbackRouterRemote().tabOrganizationSessionUpdated(
        createSession({state: TabOrganizationState.kSuccess}));
    await flushTasks();

    assertEquals(0, testApiProxy.getCallCount('rejectSession'));

    const results = tabOrganizationPage.shadowRoot!.querySelector(
        'tab-organization-results');
    assertTrue(!!results);
    const actions =
        results.shadowRoot!.querySelector('tab-organization-results-actions');
    assertTrue(!!actions);
    const clearButton =
        actions.shadowRoot!.querySelector<HTMLElement>('#clearButton');
    assertTrue(!!clearButton);
    clearButton.click();
    await flushTasks();

    assertEquals(1, testApiProxy.getCallCount('rejectSession'));
  });

  test('Sync required for organization', async () => {
    const syncInfo: SyncInfo = {
      syncing: false,
      syncingHistory: false,
      paused: false,
    };
    await tabOrganizationPageSetup(syncInfo);

    const notStarted = tabOrganizationPage.shadowRoot!.querySelector(
        'tab-organization-not-started');
    assertTrue(!!notStarted);
    assertTrue(isVisible(notStarted));

    const actionButton = notStarted.shadowRoot!.querySelector('cr-button');
    assertTrue(!!actionButton);
    actionButton.click();

    // The action button should not request tab organization if the user is in
    // an invalid sync state.
    assertEquals(0, testApiProxy.getCallCount('requestTabOrganization'));
  });

  test('Triggers sync when not syncing', async () => {
    const syncInfo: SyncInfo = {
      syncing: false,
      syncingHistory: true,
      paused: false,
    };
    await tabOrganizationPageSetup(syncInfo);

    const notStarted = tabOrganizationPage.shadowRoot!.querySelector(
        'tab-organization-not-started');
    assertTrue(!!notStarted);
    assertTrue(isVisible(notStarted));

    const actionButton = notStarted.shadowRoot!.querySelector('cr-button');
    assertTrue(!!actionButton);
    actionButton.click();

    assertEquals(1, testApiProxy.getCallCount('triggerSync'));
  });

  test('Triggers sign in when paused', async () => {
    const syncInfo: SyncInfo = {
      syncing: true,
      syncingHistory: true,
      paused: true,
    };
    await tabOrganizationPageSetup(syncInfo);

    const notStarted = tabOrganizationPage.shadowRoot!.querySelector(
        'tab-organization-not-started');
    assertTrue(!!notStarted);
    assertTrue(isVisible(notStarted));

    const actionButton = notStarted.shadowRoot!.querySelector('cr-button');
    assertTrue(!!actionButton);
    actionButton.click();

    assertEquals(1, testApiProxy.getCallCount('triggerSignIn'));
  });

  test('Opens settings when not syncing history', async () => {
    const syncInfo: SyncInfo = {
      syncing: true,
      syncingHistory: false,
      paused: false,
    };
    await tabOrganizationPageSetup(syncInfo);

    const notStarted = tabOrganizationPage.shadowRoot!.querySelector(
        'tab-organization-not-started');
    assertTrue(!!notStarted);
    assertTrue(isVisible(notStarted));

    const actionButton = notStarted.shadowRoot!.querySelector('cr-button');
    assertTrue(!!actionButton);
    actionButton.click();

    assertEquals(1, testApiProxy.getCallCount('openSyncSettings'));
  });

  test('Updates with sync changes', async () => {
    await tabOrganizationPageSetup();

    const notStarted = tabOrganizationPage.shadowRoot!.querySelector(
        'tab-organization-not-started');
    assertTrue(!!notStarted);
    assertTrue(isVisible(notStarted));

    const accountRowSynced =
        notStarted.shadowRoot!.querySelector('.account-row');
    assertFalse(!!accountRowSynced);

    testSyncProxy.syncInfo = {
      syncing: false,
      syncingHistory: false,
      paused: false,
    };
    webUIListenerCallback('sync-info-changed', testSyncProxy.syncInfo);
    await testSyncProxy.whenCalled('getSyncInfo');

    const accountRowUnsynced =
        notStarted.shadowRoot!.querySelector('.account-row');
    assertTrue(!!accountRowUnsynced);
  });

  test('Tip action activates on Enter', async () => {
    loadTimeData.overrideValues({
      showTabOrganizationFRE: true,
    });

    await tabOrganizationPageSetup();

    testApiProxy.getCallbackRouterRemote().tabOrganizationSessionUpdated(
        createSession({
          state: TabOrganizationState.kFailure,
          error: TabOrganizationError.kGeneric,
        }));

    assertEquals(0, testApiProxy.getCallCount('startTabGroupTutorial'));

    const failure = tabOrganizationPage.shadowRoot!.querySelector(
        'tab-organization-failure');
    assertTrue(!!failure);
    const links = failure.shadowRoot!.querySelectorAll<HTMLElement>(
        '.tab-organization-link');
    assertEquals(1, links.length);
    const tipAction = links[0]!;
    tipAction.dispatchEvent(new KeyboardEvent('keydown', {key: 'Enter'}));
    await flushTasks();

    assertEquals(1, testApiProxy.getCallCount('startTabGroupTutorial'));
  });
});
