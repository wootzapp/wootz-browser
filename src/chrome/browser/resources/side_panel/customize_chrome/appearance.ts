// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import './theme_snapshot.js';
import './hover_button.js';
import './strings.m.js'; // Required by <managed-dialog>.
import 'chrome://resources/cr_components/customize_color_scheme_mode/customize_color_scheme_mode.js';
import 'chrome://resources/cr_components/theme_color_picker/theme_color_picker.js';
import 'chrome://resources/cr_components/managed_dialog/managed_dialog.js';
import 'chrome://resources/cr_elements/cr_button/cr_button.js';
import 'chrome://resources/cr_elements/cr_hidden_style.css.js';
import 'chrome://resources/cr_elements/cr_icons.css.js';
import 'chrome://resources/cr_elements/cr_toggle/cr_toggle.js';

import type {CrToggleElement} from 'chrome://resources/cr_elements/cr_toggle/cr_toggle.js';
import {I18nMixin} from 'chrome://resources/cr_elements/i18n_mixin.js';
import {assert} from 'chrome://resources/js/assert.js';
import {loadTimeData} from 'chrome://resources/js/load_time_data.js';
import {PolymerElement} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';

import {getTemplate} from './appearance.html.js';
import {CustomizeChromeAction, recordCustomizeChromeAction} from './common.js';
import type {CustomizeChromePageCallbackRouter, CustomizeChromePageHandlerInterface, Theme} from './customize_chrome.mojom-webui.js';
import {CustomizeChromeApiProxy} from './customize_chrome_api_proxy.js';

export interface AppearanceElement {
  $: {
    chromeColors: HTMLElement,
    editThemeButton: HTMLButtonElement,
    themeSnapshot: HTMLElement,
    setClassicChromeButton: HTMLButtonElement,
    thirdPartyLinkButton: HTMLButtonElement,
    followThemeToggle: HTMLElement,
    followThemeToggleControl: CrToggleElement,
    uploadedImageButton: HTMLButtonElement,
    searchedImageButton: HTMLButtonElement,
  };
}

export class AppearanceElement extends I18nMixin
(PolymerElement) {
  static get is() {
    return 'customize-chrome-appearance';
  }

  static get template() {
    return getTemplate();
  }

  static get properties() {
    return {
      theme_: Object,
      themeButtonClass_: String,

      chromeRefresh2023Enabled_: {
        type: Boolean,
        value: () =>
            document.documentElement.hasAttribute('chrome-refresh-2023'),
      },

      editThemeButtonText_: {
        type: String,
        computed: 'computeEditThemeButtonText_(wallpaperSearchButtonEnabled_)',
      },

      thirdPartyThemeId_: {
        type: String,
        computed: 'computeThirdPartyThemeId_(theme_)',
        reflectToAttribute: true,
      },

      thirdPartyThemeName_: {
        type: String,
        computed: 'computeThirdPartyThemeName_(theme_)',
        reflectToAttribute: true,
      },

      showBottomDivider_: {
        type: Boolean,
        value: false,
        computed:
            'computeShowBottomDivider_(showClassicChromeButton_, showDeviceThemeToggle_)',
      },

      showClassicChromeButton_: {
        type: Boolean,
        value: false,
        computed: 'computeShowClassicChromeButton_(theme_)',
      },

      showColorPicker_: {
        type: Boolean,
        value: false,
        computed: 'computeShowColorPicker_(theme_)',
      },

      showDeviceThemeToggle_: {
        type: Boolean,
        value: false,
        computed: 'computeShowDeviceThemeToggle_(theme_)',
      },

      showThemeSnapshot_: {
        type: Boolean,
        value: false,
        computed: 'computeShowThemeSnapshot_(theme_)',
      },

      showUploadedImageButton_: {
        type: Boolean,
        value: false,
        computed: 'computeShowUploadedImageButton_(theme_)',
      },

      showSearchedImageButton_: {
        type: Boolean,
        value: false,
        computed: 'computeShowSearchedImageButton_(theme_)',
      },

      showManagedDialog_: Boolean,

      wallpaperSearchButtonEnabled_: {
        type: Boolean,
        value: () => loadTimeData.getBoolean('wallpaperSearchButtonEnabled'),
        reflectToAttribute: true,
      },

      wallpaperSearchEnabled_: {
        type: Boolean,
        value: () => loadTimeData.getBoolean('wallpaperSearchEnabled'),
      },
    };
  }

  private theme_: Theme|undefined = undefined;
  private themeButtonClass_: string;
  private chromeRefresh2023Enabled_: boolean;
  private editThemeButtonText_:
    string;
  private thirdPartyThemeId_: string|null = null;
  private thirdPartyThemeName_: string|null = null;
  private hasUploadedImage_: boolean;
  private showBottomDivider_: boolean;
  private showClassicChromeButton_: boolean;
  private showColorPicker_: boolean;
  private showDeviceThemeToggle_: boolean;
  private showThemeSnapshot: boolean;
  private showManagedDialog_: boolean;
  private wallpaperSearchButtonEnabled_:
    boolean;
  private wallpaperSearchEnabled_:
    boolean;

  private setThemeListenerId_: number|null = null;

  private callbackRouter_: CustomizeChromePageCallbackRouter;
  private pageHandler_: CustomizeChromePageHandlerInterface;

  constructor() {
    super();
    this.pageHandler_ = CustomizeChromeApiProxy.getInstance().handler;
    this.callbackRouter_ = CustomizeChromeApiProxy.getInstance().callbackRouter;
  }

  override connectedCallback() {
    super.connectedCallback();
    this.themeButtonClass_ =
        this.chromeRefresh2023Enabled_ ? 'floating-button' : 'action-button';
    this.setThemeListenerId_ =
        this.callbackRouter_.setTheme.addListener((theme: Theme) => {
          this.theme_ = theme;
        });
    this.pageHandler_.updateTheme();
  }


  override disconnectedCallback() {
    super.disconnectedCallback();
    assert(this.setThemeListenerId_);
    this.callbackRouter_.removeListener(this.setThemeListenerId_);
  }

  focusOnThemeButton() {
    this.$.editThemeButton.focus();
  }

  private computeEditThemeButtonText_(): string {
    if (!this.wallpaperSearchButtonEnabled_) {
      return this.i18n('changeTheme');
    } else {
      return this.i18n('categoriesHeader');
    }
  }

  private computeThirdPartyThemeId_(): string|null {
    if (this.theme_ && this.theme_.thirdPartyThemeInfo) {
      return this.theme_.thirdPartyThemeInfo.id;
    } else {
      return null;
    }
  }

  private computeThirdPartyThemeName_(): string|null {
    if (this.theme_ && this.theme_.thirdPartyThemeInfo) {
      return this.theme_.thirdPartyThemeInfo.name;
    } else {
      return null;
    }
  }

  private computeShowBottomDivider_(): boolean {
    return !!(this.showClassicChromeButton_ || this.showDeviceThemeToggle_);
  }

  private computeShowClassicChromeButton_(): boolean {
    return !!(
        this.theme_ &&
        (this.theme_.backgroundImage || this.theme_.thirdPartyThemeInfo));
  }

  private computeShowColorPicker_(): boolean {
    return !!this.theme_ && !this.theme_.thirdPartyThemeInfo;
  }

  private computeShowDeviceThemeToggle_(): boolean {
    return loadTimeData.getBoolean('showDeviceThemeToggle') &&
        !(!!this.theme_ && !!this.theme_.thirdPartyThemeInfo);
  }

  private computeShowThemeSnapshot_(): boolean {
    return !!this.theme_ && !this.theme_.thirdPartyThemeInfo &&
        (!this.chromeRefresh2023Enabled_ ||
         !(this.theme_.backgroundImage &&
           this.theme_.backgroundImage.isUploadedImage));
  }

  private computeShowUploadedImageButton_(): boolean {
    return !!(
        this.chromeRefresh2023Enabled_ && this.theme_ &&
        this.theme_.backgroundImage &&
        this.theme_.backgroundImage.isUploadedImage &&
        !this.theme_.backgroundImage.localBackgroundId);
  }

  private computeShowSearchedImageButton_(): boolean {
    return !!(
        this.chromeRefresh2023Enabled_ && this.theme_ &&
        this.theme_.backgroundImage &&
        this.theme_.backgroundImage.localBackgroundId);
  }

  private onEditThemeClicked_() {
    recordCustomizeChromeAction(CustomizeChromeAction.EDIT_THEME_CLICKED);
    if (this.handleClickForManagedThemes_()) {
      return;
    }
    this.dispatchEvent(new Event('edit-theme-click'));
  }

  private onWallpaperSearchClicked_() {
    recordCustomizeChromeAction(
        CustomizeChromeAction.WALLPAPER_SEARCH_APPEARANCE_BUTTON_CLICKED);
    if (this.handleClickForManagedThemes_()) {
      return;
    }
    this.dispatchEvent(new Event('wallpaper-search-click'));
  }

  private onThirdPartyLinkButtonClick_() {
    if (this.thirdPartyThemeId_) {
      this.pageHandler_.openThirdPartyThemePage(this.thirdPartyThemeId_);
    }
  }

  private onUploadedImageButtonClick_() {
    this.pageHandler_.chooseLocalCustomBackground();
  }

  private onSearchedImageButtonClick_() {
    if (this.wallpaperSearchEnabled_) {
      this.dispatchEvent(new CustomEvent('wallpaper-search-click'));
    } else {
      this.dispatchEvent(new Event('edit-theme-click'));
    }
  }

  private onSetClassicChromeClicked_() {
    if (this.handleClickForManagedThemes_()) {
      return;
    }
    this.pageHandler_.removeBackgroundImage();
    this.pageHandler_.setDefaultColor();
    recordCustomizeChromeAction(
        CustomizeChromeAction.SET_CLASSIC_CHROME_THEME_CLICKED);
  }

  private onFollowThemeToggleChange_(e: CustomEvent<boolean>) {
    this.pageHandler_.setFollowDeviceTheme(e.detail);
  }

  private onManagedDialogClosed_() {
    this.showManagedDialog_ = false;
  }

  private handleClickForManagedThemes_(): boolean {
    if (!this.theme_ || !this.theme_.backgroundManagedByPolicy) {
      return false;
    }
    this.showManagedDialog_ = true;
    return true;
  }
}

declare global {
  interface HTMLElementTagNameMap {
    'customize-chrome-appearance': AppearanceElement;
  }
}

customElements.define(AppearanceElement.is, AppearanceElement);
