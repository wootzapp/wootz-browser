// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview
 * 'settings-cookies-page' is the settings page containing cookies
 * settings.
 */

import 'chrome://resources/cr_components/settings_prefs/prefs.js';
import 'chrome://resources/cr_elements/cr_icon_button/cr_icon_button.js';
import 'chrome://resources/cr_elements/cr_link_row/cr_link_row.js';
import 'chrome://resources/cr_elements/cr_toast/cr_toast.js';
import '../controls/settings_toggle_button.js';
import '../icons.html.js';
import '../settings_shared.css.js';
import '../site_settings/site_list.js';
import './collapse_radio_button.js';
import './do_not_track_toggle.js';
import '../controls/settings_radio_group.js';

import {PrefsMixin} from 'chrome://resources/cr_components/settings_prefs/prefs_mixin.js';
import type {CrToastElement} from 'chrome://resources/cr_elements/cr_toast/cr_toast.js';
import {I18nMixin} from 'chrome://resources/cr_elements/i18n_mixin.js';
import {WebUiListenerMixin} from 'chrome://resources/cr_elements/web_ui_listener_mixin.js';
import {assert} from 'chrome://resources/js/assert.js';
import {focusWithoutInk} from 'chrome://resources/js/focus_without_ink.js';
import {OpenWindowProxyImpl} from 'chrome://resources/js/open_window_proxy.js';
import {PolymerElement} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';

import type {SettingsRadioGroupElement} from '../controls/settings_radio_group.js';
import type {SettingsToggleButtonElement} from '../controls/settings_toggle_button.js';
import type {FocusConfig} from '../focus_config.js';
import {loadTimeData} from '../i18n_setup.js';
import type {MetricsBrowserProxy} from '../metrics_browser_proxy.js';
import {MetricsBrowserProxyImpl, PrivacyElementInteractions} from '../metrics_browser_proxy.js';
import {routes} from '../route.js';
import type {Route} from '../router.js';
import {RouteObserverMixin, Router} from '../router.js';
import {ContentSetting, ContentSettingsTypes, CookieControlsMode} from '../site_settings/constants.js';
import {CookiePrimarySetting} from '../site_settings/site_settings_prefs_browser_proxy.js';

import {getTemplate} from './cookies_page.html.js';

export interface SettingsCookiesPageElement {
  $: {
    toast: CrToastElement,
  };
}

const SettingsCookiesPageElementBase = RouteObserverMixin(
    WebUiListenerMixin(I18nMixin(PrefsMixin(PolymerElement))));

export class SettingsCookiesPageElement extends SettingsCookiesPageElementBase {
  static get is() {
    return 'settings-cookies-page';
  }

  static get template() {
    return getTemplate();
  }

  static get properties() {
    return {
      /**
       * Preferences state.
       */
      prefs: {
        type: Object,
        notify: true,
      },

      /**
       * Current search term.
       */
      searchTerm: {
        type: String,
        notify: true,
        value: '',
      },

      /**
       * Primary cookie control states for use in bindings.
       */
      cookiePrimarySettingEnum_: {
        type: Object,
        value: CookiePrimarySetting,
      },

      /** Cookie control modes for use in bindings. */
      cookieControlsModeEnum_: {
        type: Object,
        value: CookieControlsMode,
      },

      contentSetting_: {
        type: Object,
        value: ContentSetting,
      },

      cookiesContentSettingType_: {
        type: String,
        value: ContentSettingsTypes.COOKIES,
      },

      exceptionListsReadOnly_: {
        type: Boolean,
        value: false,
      },

      blockAllPref_: {
        type: Object,
        value() {
          return {};
        },
      },

      focusConfig: {
        type: Object,
        observer: 'focusConfigChanged_',
      },

      enableFirstPartySetsUI_: {
        type: Boolean,
        value: () => loadTimeData.getBoolean('firstPartySetsUIEnabled'),
      },

      is3pcdRedesignEnabled_: {
        type: Boolean,
        value: () =>
            loadTimeData.getBoolean('is3pcdCookieSettingsRedesignEnabled'),
      },

      isCookieSettingsUiAlignmentEnabled_: {
        type: Boolean,
        value: () =>
            loadTimeData.getBoolean('isCookieSettingsUiAlignmentEnabled'),
      },

      isCookiesUiV2_: {
        type: Boolean,
        value: () =>
            (loadTimeData.getBoolean('isCookieSettingsUiAlignmentEnabled') ||
             loadTimeData.getBoolean('is3pcdCookieSettingsRedesignEnabled')),
      },

      isIpProtectionAvailable_: {
        type: Boolean,
        value: () => loadTimeData.getBoolean('isIpProtectionV1Enabled'),
      },

      showTrackingProtectionRollbackNotice_: {
        type: Boolean,
        value: () => loadTimeData.getBoolean(
            'showTrackingProtectionSettingsRollbackNotice'),
      },
    };
  }

  static get observers() {
    return [`onGeneratedPrefsUpdated_(prefs.generated.cookie_session_only,
        prefs.generated.cookie_primary_setting,
        prefs.generated.cookie_default_content_setting)`];
  }

  searchTerm: string;
  private cookiesContentSettingType_: ContentSettingsTypes;
  private exceptionListsReadOnly_: boolean;
  private blockAllPref_: chrome.settingsPrivate.PrefObject;
  focusConfig: FocusConfig;
  private enableFirstPartySetsUI_: boolean;
  private is3pcdRedesignEnabled_: boolean;
  private isCookieSettingsUiAlignmentEnabled_: boolean;
  private isIpProtectionAvailable_: boolean;
  private isCookiesUiV2_: boolean;

  private metricsBrowserProxy_: MetricsBrowserProxy =
      MetricsBrowserProxyImpl.getInstance();

  private focusConfigChanged_(_newConfig: FocusConfig, oldConfig: FocusConfig) {
    assert(!oldConfig);
    const selectSiteDataLinkRow = () => {
      const toFocus =
          this.shadowRoot!.querySelector<HTMLElement>('#site-data-trigger');
      assert(toFocus);
      focusWithoutInk(toFocus);
    };
    if (this.is3pcdRedesignEnabled_) {
      this.focusConfig.set(
          `${routes.SITE_SETTINGS_ALL.path}_${routes.TRACKING_PROTECTION.path}`,
          selectSiteDataLinkRow);
    } else {
      this.focusConfig.set(
          `${routes.SITE_SETTINGS_ALL.path}_${routes.COOKIES.path}`,
          selectSiteDataLinkRow);
    }
  }

  override currentRouteChanged(route: Route) {
    if (this.is3pcdRedesignEnabled_) {
      if (route !== routes.TRACKING_PROTECTION) {
        this.$.toast.hide();
      }
    } else if (route !== routes.COOKIES) {
      this.$.toast.hide();
    }
  }

  private getPageDescription_(): string {
    return this.i18n(
        this.isCookieSettingsUiAlignmentEnabled_ ?
            'thirdPartyCookiesAlignedPageDescription' :
            'thirdPartyCookiesPageDescription');
  }

  private getThirdPartyCookiesPageBlockThirdPartyIncognitoBulTwoLabel_():
      string {
    return this.i18n(
        this.enableFirstPartySetsUI_ ?
            'cookiePageBlockThirdIncognitoBulTwoFps' :
            'thirdPartyCookiesPageBlockIncognitoBulTwo');
  }

  private getCookiesPageBlockThirdPartyIncognitoBulTwoLabel_(): string {
    return this.i18n(
        this.enableFirstPartySetsUI_ ?
            'cookiePageBlockThirdIncognitoBulTwoFps' :
            'cookiePageBlockThirdIncognitoBulTwo');
  }

  private onSiteDataClick_() {
    Router.getInstance().navigateTo(routes.SITE_SETTINGS_ALL);
  }

  private onIpProtectionLearnMoreClicked_() {
    OpenWindowProxyImpl.getInstance().openUrl(
        loadTimeData.getString('ipProtectionLearnMoreUrl'));
  }

  private onGeneratedPrefsUpdated_() {
    // If the default cookie content setting is managed, the exception lists
    // should be disabled. `profile.cookie_controls_mode` doesn't control the
    // ability to create exceptions but the content setting does.
    const defaultContentSettingPref =
        this.getPref('generated.cookie_default_content_setting');
    this.exceptionListsReadOnly_ = defaultContentSettingPref.enforcement ===
        chrome.settingsPrivate.Enforcement.ENFORCED;
  }

  private onBlockAll3pcToggleChanged_(event: Event) {
    this.metricsBrowserProxy_.recordSettingsPageHistogram(
        PrivacyElementInteractions.BLOCK_ALL_THIRD_PARTY_COOKIES);
    const target = event.target as SettingsToggleButtonElement;
    if (target.checked) {
      this.metricsBrowserProxy_.recordAction(
          'Settings.PrivacySandbox.Block3PCookies');
    }
  }

  private onIpProtectionChanged_() {
    this.metricsBrowserProxy_.recordSettingsPageHistogram(
        PrivacyElementInteractions.IP_PROTECTION);
  }

  private onCookieControlsModeChanged_() {
    // TODO(crbug.com/1378703): Use this.$.primarySettingGroup after the feature
    // is launched and element isn't in dom-if anymore.
    const primarySettingGroup: SettingsRadioGroupElement =
        this.shadowRoot!.querySelector('#primarySettingGroup')!;
    const selection = Number(primarySettingGroup.selected);
    if (selection === CookieControlsMode.OFF) {
      this.metricsBrowserProxy_.recordSettingsPageHistogram(
          PrivacyElementInteractions.THIRD_PARTY_COOKIES_ALLOW);
    } else if (selection === CookieControlsMode.INCOGNITO_ONLY) {
      this.metricsBrowserProxy_.recordSettingsPageHistogram(
          PrivacyElementInteractions.THIRD_PARTY_COOKIES_BLOCK_IN_INCOGNITO);
    } else {
      assert(selection === CookieControlsMode.BLOCK_THIRD_PARTY);
      this.metricsBrowserProxy_.recordSettingsPageHistogram(
          PrivacyElementInteractions.THIRD_PARTY_COOKIES_BLOCK);
    }

    // If this change resulted in the user now blocking 3P cookies where they
    // previously were not, and any of privacy sandbox APIs are enabled,
    // the privacy sandbox toast should be shown.
    const currentCookieControlsMode =
        this.getPref('profile.cookie_controls_mode').value;
    const areAnyPrivacySandboxApisEnabled =
        this.getPref('privacy_sandbox.m1.topics_enabled').value ||
        this.getPref('privacy_sandbox.m1.fledge_enabled').value ||
        this.getPref('privacy_sandbox.m1.ad_measurement_enabled').value;
    const areThirdPartyCookiesAllowed =
        currentCookieControlsMode === CookieControlsMode.OFF ||
        currentCookieControlsMode === CookieControlsMode.INCOGNITO_ONLY;

    if (areAnyPrivacySandboxApisEnabled && areThirdPartyCookiesAllowed &&
        selection === CookieControlsMode.BLOCK_THIRD_PARTY) {
      if (!loadTimeData.getBoolean('isPrivacySandboxRestricted')) {
        this.$.toast.show();
      }
      this.metricsBrowserProxy_.recordAction(
          'Settings.PrivacySandbox.Block3PCookies');
    } else {
      this.$.toast.hide();
    }

    primarySettingGroup.sendPrefChange();
  }

  private onClearOnExitChange_() {
    this.metricsBrowserProxy_.recordSettingsPageHistogram(
        PrivacyElementInteractions.COOKIES_SESSION);
  }

  private onPrivacySandboxClick_() {
    this.metricsBrowserProxy_.recordAction(
        'Settings.PrivacySandbox.OpenedFromCookiesPageToast');
    this.$.toast.hide();
    // TODO(crbug/1159942): Replace this with an ordinary OpenWindowProxy call.
    this.shadowRoot!.querySelector<HTMLAnchorElement>(
                        '#privacySandboxLink')!.click();
  }

  private firstPartySetsToggleDisabled_() {
    return this.getPref('profile.cookie_controls_mode').value !==
        CookieControlsMode.BLOCK_THIRD_PARTY;
  }
}

declare global {
  interface HTMLElementTagNameMap {
    'settings-cookies-page': SettingsCookiesPageElement;
  }
}

customElements.define(
    SettingsCookiesPageElement.is, SettingsCookiesPageElement);
