// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import '//resources/ash/common/cr_elements/cros_color_overrides.css.js';
import '//resources/ash/common/cr_elements/cr_input/cr_input.js';
import '//resources/polymer/v3_0/iron-icon/iron-icon.js';
import '//resources/polymer/v3_0/iron-media-query/iron-media-query.js';
import '../../components/oobe_icons.html.js';
import '../../components/buttons/oobe_next_button.js';
import '../../components/common_styles/oobe_common_styles.css.js';
import '../../components/common_styles/oobe_dialog_host_styles.css.js';
import '../../components/dialogs/oobe_adaptive_dialog.js';

import {PolymerElementProperties} from '//resources/polymer/v3_0/polymer/interfaces.js';
import {mixinBehaviors, PolymerElement} from '//resources/polymer/v3_0/polymer/polymer_bundled.min.js';

import {LoginScreenBehavior, LoginScreenBehaviorInterface} from '../../components/behaviors/login_screen_behavior.js';
import {OobeDialogHostBehavior, OobeDialogHostBehaviorInterface} from '../../components/behaviors/oobe_dialog_host_behavior.js';
import {OobeI18nMixin, OobeI18nMixinInterface} from '../../components/mixins/oobe_i18n_mixin.js';
import {OobeUiState} from '../../components/display_manager_types.js';

import {getTemplate} from './local_data_loss_warning.html.js';


const LocalDataLossWarningBase = mixinBehaviors(
      [
        OobeDialogHostBehavior,
        LoginScreenBehavior,
      ],
      OobeI18nMixin(PolymerElement)) as {
new (): PolymerElement & OobeI18nMixinInterface &
LoginScreenBehaviorInterface & OobeDialogHostBehaviorInterface,
};

/**
 * Data that is passed to the screen during onBeforeShow.
 */
interface LocalDataLossWarningScreenData {
  isOwner: boolean;
  email: string;
  canGoBack: boolean;
}

export class LocalDataLossWarning extends LocalDataLossWarningBase {
  static get is() {
    return 'local-data-loss-warning-element' as const;
  }

  static get template(): HTMLTemplateElement {
    return getTemplate();
  }

  static get properties(): PolymerElementProperties {
    return {
      email: {
        type: String,
        value: '',
      },

      disabled: {
        type: Boolean,
      },

      isOwner: {
        type: Boolean,
      },

      canGoBack: {
        type: Boolean,
      },
    };
  }

  private email:string;
  private disabled : boolean;
  private isOwner : boolean;
  private canGoBack : boolean;


  constructor() {
    super();
    this.disabled = false;
  }

  override ready(): void {
    super.ready();
    this.initializeLoginScreen('LocalDataLossWarningScreen');
  }

  /** Initial UI State for screen */
  // eslint-disable-next-line @typescript-eslint/naming-convention
  override getOobeUIInitialState(): OobeUiState {
    return OobeUiState.PASSWORD_CHANGED;
  }

  /**
   * Invoked just before being shown. Contains all the data for the screen.
   */
  override onBeforeShow(data: LocalDataLossWarningScreenData) : void {
    this.isOwner = data['isOwner'];
    this.email = data['email'];
    this.canGoBack = data['canGoBack'];
  }

  /**
   * Returns the subtitle message for the data loss warning screen.
   * @param locale The i18n locale.
   * @param email The email address that the user is trying to recover.
   * @return The translated subtitle message.
   */
  private getDataLossWarningSubtitleMessage(locale: string, email: string):
      TrustedHTML {
    return this.i18nAdvancedDynamic(
        locale, 'dataLossWarningSubtitle', {substitutions: [email]});
  }

  private onProceedClicked() : void {
    if (this.disabled) {
      return;
    }
    this.disabled = true;
    this.userActed('recreateUser');
  }

  private onResetClicked(): void {
    if (this.disabled) {
      return;
    }
    this.disabled = true;
    this.userActed('powerwash');
  }

  private onBackButtonClicked(): void {
    if (this.disabled) {
      return;
    }
    this.userActed('back');
  }

  private onCancelClicked() : void {
    if (this.disabled) {
      return;
    }
    this.userActed('cancel');
  }
}

declare global {
  interface HTMLElementTagNameMap {
    [LocalDataLossWarning.is]: LocalDataLossWarning;
  }
}


customElements.define(LocalDataLossWarning.is, LocalDataLossWarning);
