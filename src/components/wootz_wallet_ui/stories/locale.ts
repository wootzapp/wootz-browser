// Copyright (c) 2021 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.
import { provideStrings } from '../../../.storybook/locale'

provideStrings({
  // Entities
  wootzWalletContract: 'Contract',
  wootzWalletStaker: 'Staker',
  wootzWalletWithdrawer: 'Withdrawer',

  // Search Text
  wootzWalletSearchText: 'Search',

  // Side Nav Buttons
  wootzWalletSideNavCrypto: 'Crypto',
  wootzWalletSideNavCards: 'Cards',

  // Top Nav Tab Buttons
  wootzWalletTopNavPortfolio: 'Portfolio',
  wootzWalletTopTabPrices: 'Prices',
  wootzWalletTopTabApps: 'Apps',
  wootzWalletTopNavNFTS: 'NFTs',
  wootzWalletTopNavAccounts: 'Accounts',
  wootzWalletTopNavMarket: 'Market',

  // Chart Timeline Buttons
  wootzWalletChartOneHour: '1 Hour',
  wootzWalletChartOneDay: '1 Day',
  wootzWalletChartOneWeek: '1 Week',
  wootzWalletChartOneMonth: '1 Month',
  wootzWalletChartThreeMonths: '3 Months',
  wootzWalletChartOneYear: '1 Year',
  wootzWalletChartAllTime: 'All time',
  wootzWalletChartOneHourAbbr: '1H',
  wootzWalletChartOneDayAbbr: '1D',
  wootzWalletChartOneWeekAbbr: '1W',
  wootzWalletChartOneMonthAbbr: '1M',
  wootzWalletChartThreeMonthsAbbr: '3M',
  wootzWalletChartOneYearAbbr: '1Y',
  wootzWalletChartAllTimeAbbr: 'All',

  // Portfolio View
  wootzWalletAddCoin: 'Add Coin',
  wootzWalletBalance: 'Balance',
  wootzWalletPortfolioAssetNetworkDescription: '$1 on $2',

  // Portfolio SubView
  wootzWalletAccounts: 'Accounts',
  wootzWalletSubviewAccount: 'Account',
  wootzWalletOwner: 'Owner',
  wootzWalletActivity: 'Activity',
  wootzWalletTransactions: 'Transactions',
  wootzWalletPrice: 'Price',
  wootzWalletBack: 'Back',
  wootzWalletAddAccount: 'Add Account',
  wootzWalletPoweredByCoinGecko: 'Price data powered by CoinGecko',

  // Actions
  wootzWalletClickToSwitch: 'Click to switch',
  wootzWalletEnterYourPassword: 'Enter your password',
  wootzWalletEnterAPasswordToContinue: 'Enter a password to continue',
  wootzWalletEnterYourPasswordToStartBackup:
    'Enter your Wootz Wallet password to start backing up wallet.',

  // BuySendSwap
  wootzWalletBuy: 'Buy',
  wootzWalletSend: 'Send',
  wootzWalletSwap: 'Swap',
  wootzWalletBridge: 'Bridge',
  wootzWalletReset: 'Reset',
  wootzWalletSell: 'Sell',
  wootzWalletNotEnoughBalance: 'You don’t have enough $1 in this account.',
  wootzWalletSlippageToleranceWarning: 'Transaction may be frontrun',
  wootzWalletExpiresInTitle: 'Expires in',
  wootzWalletSendPlaceholder: 'Wallet address or URL',
  wootzWalletSendNoURLPlaceholder: 'Wallet address',
  wootzWalletSwapDisclaimer: 'Wootz uses $1$3$2 as a DEX aggregator.',
  wootzWalletSwapDisclaimerDescription:
    '0x will process the Ethereum address and IP address ' +
    'to fulfill a transaction (including getting quotes). ' +
    '0x will ONLY use this data for the purposes of processing transactions.',
  wootzWalletJupiterSwapDisclaimerDescription:
    'Jupiter will process the Solana address and IP address ' +
    'to fulfill a transaction (including getting quotes). ' +
    'Jupiter will ONLY use this data ' +
    'for the purposes of processing transactions.',
  wootzWalletSwapFeesNotice: 'Quote includes a $1 Wootz fee.',
  wootzWalletDecimalPlacesError: 'Too many decimal places',
  wootzWalletBuyTapBuyNotSupportedMessage:
    'Buy not supported for selected network',
  wootzWalletSearchingForDomain: 'Searching for domain...',
  wootzWalletEnsOffChainLookupTitle:
    'Wootz supports using off-chain gateways to resolve .eth domains.',
  wootzWalletEnsOffChainLookupDescription:
    "It looks like you've entered an ENS address. " +
    "We'll need to use a third-party resolver to resolve this request, " +
    'which may be able to see your IP address and domain.',
  wootzWalletEnsOffChainButton: 'Use ENS domain',
  wootzWalletFEVMAddressTranslationTitle:
    'ETH address will be converted to the Filecoin address.',
  wootzWalletFEVMAddressTranslationDescription:
    "It looks like you've entered an ENS address. " +
    "We'll need to use a third-party resolver to resolve this request, " +
    'which may be able to see your IP address and domain.',
  wootzWalletFetchingQuote: 'Fetching quote',
  wootzWalletReceiveEstimate: 'Receive (est.)',
  wootzWalletFromToken: 'From token',
  wootzWalletToToken: 'To token',
  wootzWalletNewQuoteIn: 'New quote in $1$3$2',

  // Send Tab
  wootzWalletChooseAssetToSend: 'Choose asset to send',
  wootzWalletChooseAssetToSwap: 'Choose asset to swap',
  wootzWalletEnterRecipientAddress: 'Enter recipient address',
  wootzWalletNotEnoughFunds: 'Not enough funds',
  wootzWalletSendHalf: 'HALF',
  wootzWalletSendMax: 'MAX',
  wootzWalletReviewSend: 'Review send',
  wootzWalletNoAvailableTokens: 'No available tokens',
  wootzWalletSearchTokens: 'Search by name or paste address',
  wootzWalletSearchNFTs: 'Search NFT by name, id',
  wootzWalletChooseAsset: 'Choose asset',
  wootzWalletOrdinalsWarningMessage:
    'Wootz Wallet does not currently support Bitcoin NFTs (ordinals). Sending BTC from an address that has ordinals may result in its ordinals being transferred inadvertently.',
  wootzWalletUserUnderstandsLabel: 'I understand',
  wootzWalletChooseRecipient: 'Choose recipient',
  wootzWalletMyAddresses: 'My addresses',
  wootzWalletAddressOrDomainPlaceholder: 'Enter public address or domain.',
  wootzWalletSendTo: 'Send to',
  wootzWalletInvalidDomainExtension: 'Invalid domain extension.',
  wootzWalletChooseAccount: 'Choose an account to continue',
  wootzWalletMarketPrice: 'Market price',
  wootzWalletOwned: 'Owned',
  wootzWalletNotOwned: 'Not owned',
  wootzWalletAmount24H: 'Amount/24h',

  // Bridge
  wootzWalletChooseAssetToBridge: 'Choose asset to bridge',
  wootzWalletEstTime: 'Est time',
  wootzWalletExchangeRate: 'Exchange rate',
  wootzWalletExchangeFor: '$1 for $2',
  wootzWalletRecipient: 'Recipient',
  wootzWalletReviewBridge: 'Review bridge',

  // Create Account Tab
  wootzWalletCreateAccountDescription:
    'You don’t yet have a $1 account. Create one now?',
  wootzWalletCreateAccountYes: 'Yes',
  wootzWalletCreateAccountNo: 'No',

  // Buttons
  wootzWalletButtonContinue: 'Continue',
  wootzWalletContinueWithXItems: 'Continue with $1 $2',
  wootzWalletButtonNext: 'Next',
  wootzWalletButtonVerifyPhrase: 'Show my recovery phrase',
  wootzWalletButtonCopy: 'Copy',
  wootzWalletButtonCopied: 'Copied!',
  wootzWalletButtonVerify: 'Verify',
  wootzWalletButtonClose: 'Close',
  wootzWalletButtonMore: 'More',
  wootzWalletButtonDone: 'Done',
  wootzWalletButtonSkip: 'Skip',
  wootzWalletButtonCancel: 'Cancel',
  wootzWalletButtonSaveChanges: 'Save changes',
  wootzWalletLearnMore: 'Learn more',
  wootzWalletButtonNoThanks: 'No thanks',
  wootzWalletButtonEnable: 'Enable',
  wootzWalletProceedButton: 'Proceed',
  wootzWalletButtonRetry: 'Retry',

  // Onboarding - Welcome
  wootzWalletWelcomeRestoreButton: 'Restore',
  wootzWalletWelcomeTitle: 'Browser-native. Self-custody. And multi-chain.',
  wootzWalletWelcomeDescription:
    'Take control of your crypto and NFTs. ' +
    'Wootz Wallet supports Ethereum, EVM chains, ' +
    'Solana, Filecoin, Bitcoin, and more.',
  wootzWalletWelcomeNewWalletTitle: 'Need a new wallet?',
  wootzWalletWelcomeNewWalletDescription:
    'Get started with Wootz Wallet in minutes.',
  wootzWalletWelcomeImportWalletTitle: 'Already have a wallet?',
  wootzWalletWelcomeImportWalletDescription: 'Import your existing wallet.',
  wootzWalletWelcomeWatchOnlyTitle: 'I need a watch-only wallet',
  wootzWalletCopyright:
    '©2024 Wootz Software Inc. Wootz and the Wootz logo are registered ' +
    'trademarks of Wootz Software, Inc. Other product names and logos may be ' +
    'trademarks of their respective companies. All rights reserved.',
  wootzWalletWelcomeDividerText: 'or',

  // Onboarding - Disclosures
  wootzWalletDisclosuresTitle: 'Before we begin',
  wootzWalletDisclosuresDescription:
    'We require that you acknowledge the items below',
  wootzWalletSelfCustodyDisclosureCheckboxText:
    'I understand that this is a self-custody wallet, ' +
    'and that I am solely responsible for any associated funds, assets, ' +
    'or accounts, and for taking any appropriate action to ' +
    'secure, protect, and back up my wallet. ' +
    'I understand that Wootz cannot access my wallet or ' +
    'reverse transactions on my behalf, ' +
    'and that my recovery phrase is the ONLY way to ' +
    'regain access in the event of a ' +
    'lost password, stolen device, or similar circumstance.',
  wootzWalletTermsOfServiceCheckboxText:
    'I have read and agree to the $1Terms of use$2.',

  // Onboarding import or restore wallet page
  wootzWalletCheckingInstalledExtensions: 'Checking for wallet extensions...',
  wootzWalletImportOrRestoreWalletTitle: 'Connect to your existing wallet',
  wootzWalletImportOrRestoreDescription:
    'To connect a wallet you already have, ' +
    'you may need to enter your recovery phrase. At this time we support ' +
    'restoring / importing from Ethereum and Solana wallets.',
  wootzWalletRestoreMyWootzWallet: 'Import an existing wallet',
  wootzWalletRestoreMyWootzWalletDescription: '12-24 words',
  wootzWalletImportFromMetaMask: 'Import from MetaMask',
  wootzWalletImportFromMetaMaskDescription:
    'Use your MetaMask password to import your seed phrase',
  wootzWalletImportFromLegacy: 'Import from legacy Wootz crypto wallets',
  wootzWalletCreateWalletInsteadLink: 'Never mind, I’ll create a new wallet',
  wootzWalletRestoreAlternateLength: 'I have a $1-word recovery phrase',
  wootzWalletRestoreWalletError: 'Check your recovery phrase and try again.',
  wootzWalletImportWithRecoveryPhrase: 'Import using your recovery phrase',
  wootzWalletMetamaskDetected: 'Metamask wallet detected!',
  wootzWalletLegacyWalletDetected: 'Legacy wallet detected!',
  wootzWalletUsePassword: 'Use password',
  wootzWalletRecoveryPhraseWord: 'Word $1',

  // onboarding import wallet screen
  wootzWalletImportPasswordError: 'Password is not correct',
  wootzWalletMetaMaskPasswordInputPlaceholder: 'Type MetaMask password',
  wootzWalletImportFromMetaMaskSeedInstructions:
    'Type your MetaMask 12-24 word recovery phrase.',
  wootzWalletMetaMaskExtensionDetected: 'Import MetaMask wallet',

  wootzWalletMetaMaskExtensionImportDescription:
    'You can easily import your MetaMask wallet just by inputting ' +
    'your password.',
  wootzWalletRestoreMyWootzWalletInstructions:
    'You can paste your entire recovery phrase into any field.',
  wootzWalletRecoveryPhraseLengthError:
    'Recovery phrase must be 12, 15, 18, 21, or 24 words long',
  wootzWalletInvalidMnemonicError:
    'The mnemonic being imported is not valid for Wootz Wallet',
  wootzWalletImportWalletTypeTitle:
    'Which type of wallet would you like to import?',
  wootzWalletImportWalletTypeHotWalletTitle: 'Ethereum/Solana/Filecoin wallet',
  wootzWalletImportWalletTypeHotWalletDescription:
    'Import your seed phrase from an existing wallet',
  wootzWalletImportWalletTypeHardwareWalletTitle: 'Hardware wallet',
  wootzWalletImportWalletTypeHardwareWalletDescription:
    'Connect your hardware wallet with Wootz',

  // Onboarding - Backup Wallet - Intro
  wootzWalletOnboardingRecoveryPhraseBackupIntroTitle:
    'Save your recovery phrase',
  wootzWalletOnboardingRecoveryPhraseBackupIntroDescription:
    'Your recovery phrase is the key to access your wallet in case you ' +
    'forget your password or lose your device.',
  wootzWalletRecoveryPhraseBackupWarningImportant:
    'Keep it in a secure place that is not accessible to others and avoid ' +
    'sharing it with anyone.',
  wootzWalletRecoveryWordInstructions:
    'Enter the word in position $1 from your recovery phrase.',
  wootzWalletVerifyRecoveryPhraseGoBack: 'Forgot to save? Go back',

  // Onboarding - Backup Wallet - Recovery Phrase Backup
  wootzWalletRecoveryPhraseBackupTitle: 'Back up your wallet recovery phrase',
  wootzWalletRecoveryPhraseBackupWarning:
    'Wootz cannot access your secret recovery phrase. ' +
    'Keep it safe, and never share it with anyone else.',
  wootzWalletCopiedToClipboard: 'Copied to clipboard',
  wootzWalletClickToSeeRecoveryPhrase: 'Click to see your phrase',
  wootzWalletSkipBackupWarning:
    'You can skip this step now, but you still need to back up your recovery ' +
    'phrase to ensure account security.',
  wootzWalletSkipBackup: 'I’ll back up later',
  wootzWalletGoBack: 'Go back',

  // Onboarding - Backup Wallet - Verify Recovery Phrase
  wootzWalletVerifyRecoveryPhraseTitle: "Let's check",
  wootzWalletVerifyRecoveryPhraseInstructions:
    'Click the $1$7 ($8)$2, $3$9 ($10)$4, and $5$11 ($12)$6 words of ' +
    'your recovery phrase.',
  wootzWalletVerifyPhraseError: "Recovery phrase didn't match",

  // Recovery Phrase Backup - Intro
  wootzWalletBackupIntroTitle: 'Back up your crypto wallet',
  wootzWalletBackupIntroTerms:
    'I understand that if I lose my recovery words, ' +
    'I will not be able to access my crypto wallet.',

  // Recovery Phrase Backup - Intro
  wootzWalletRecoveryTitle: 'Your recovery phrase',
  wootzWalletRecoveryDescription:
    'Write down or copy these words in the exact order shown below, ' +
    'and save them somewhere safe. Your recovery phrase is the only ' +
    'way to regain account access in case of forgotten password, ' +
    'lost or stolen device, or you want to switch wallets.',
  wootzWalletRecoveryWarning1: 'WARNING:',
  wootzWalletRecoveryWarning2: 'Never share your recovery phrase.',
  wootzWalletRecoveryWarning3:
    'Anyone with this phrase can take your assets forever.',
  wootzWalletRecoveryTerms: 'I have backed up my phrase somewhere safe.',

  // Recovery Phrase Backup - Verify Recovery Phrase
  wootzWalletVerifyRecoveryTitle: 'Verify recovery phrase',
  wootzWalletVerifyRecoveryDescription:
    'Select the words in your recovery phrase in their correct order.',
  wootzWalletVerifyError: 'Recovery phrase did not match, please try again.',

  // Create Password
  wootzWalletCreatePasswordTitle: 'Create a new password',
  wootzWalletCreatePasswordDescription:
    "You'll use this password to access your wallet.",
  wootzWalletCreatePasswordInput: 'Enter new password',
  wootzWalletConfirmPasswordInput: 'Re-enter password',
  wootzWalletCreatePasswordError: "Password criteria doesn't match.",
  wootzWalletConfirmPasswordError: 'Passwords do not match',
  wootzWalletPasswordMatch: 'Passwords match',
  wootzWalletPasswordIsStrong: 'Strong!',
  wootzWalletPasswordIsMediumStrength: 'Medium',
  wootzWalletPasswordIsWeak: 'Weak',
  wootzWalletAutoLockTitle: 'Wootz Wallet will auto-lock after',
  wootzWalletAutoLockDurationMinutes: '$1 minutes',
  wootzWalletAutoLockDurationHours: '$1 hour(s)',

  // Creating Wallet
  wootzWalletCreatingWallet: 'Creating Wallet...',

  // Create Password - Strength Tooltip
  wootzWalletPasswordStrengthTooltipHeading: 'Password should have:',
  wootzWalletPasswordStrengthTooltipIsLongEnough: 'At least 8 characters',

  // Onboarding Success
  wootzWalletOnboardingSuccessTitle: 'You’re all set!',
  wootzWalletOnboardingSuccessDescription:
    'Your Wootz Wallet is now set up and ready for use.',
  wootzWalletBuyCryptoButton: 'Buy crypto',
  wootzWalletDepositCryptoButton: 'Deposit',
  wootzWalletLearnAboutMyWallet: 'Learn more about my new wallet',
  wootzWalletOnboardingSuccessGoToPortfolio: 'Go to portfolio',

  // Wallet Article Links
  wootzWalletArticleLinkWhatsARecoveryPhrase: 'What’s a recovery phrase?',

  // Lock Screen
  wootzWalletEnterYourWootzWalletPassword: 'Enter your Wootz Wallet password',
  wootzWalletLockScreenTitle: 'Enter password to unlock wallet',
  wootzWalletLockScreenButton: 'Unlock',
  wootzWalletLockScreenError: 'Incorrect password',
  wootzWalletUnlockWallet: 'Unlock Wallet',

  // Wallet More Popup
  wootzWalletWalletPopupSettings: 'Settings',
  wootzWalletWalletPopupLock: 'Lock wallet',
  wootzWalletWalletPopupBackup: 'Backup Wallet',
  wootzWalletWalletPopupConnectedSites: 'Connected sites',
  wootzWalletWalletPopupHideBalances: 'Balances',
  wootzWalletWalletPopupShowGraph: 'Graph',
  wootzWalletWalletNFTsTab: 'NFTs tab',

  // Backup Warning
  wootzWalletBackupWarningText:
    'Back up your wallet now ' +
    'to protect your assets and ensure you never lose access.',
  wootzWalletBackupButton: 'Back up now',
  wootzWalletDismissButton: 'Dismiss',

  // Default Wallet Banner
  wootzWalletDefaultWalletBanner:
    'Wootz Wallet is not set as your default wallet and will not ' +
    'respond to Web3 DApps. Visit settings to change your default wallet.',

  // Restore Screen
  wootzWalletRestoreTite: 'Restore primary crypto accounts',
  wootzWalletRestoreDescription:
    'Enter your recovery phrase to restore your Wootz wallet crypto account.',
  wootzWalletRestoreError: 'The recovery phrase entered is invalid.',
  wootzWalletRestorePlaceholder: 'Paste recovery phrase from clipboard',
  wootzWalletRestoreShowPhrase: 'Show recovery phrase',
  wootzWalletRestoreLegacyCheckBox: 'Import from legacy Wootz crypto wallets?',
  wootzWalletRestoreFormText: 'New Password',

  // Clipboard
  wootzWalletToolTipCopyToClipboard: 'Copy to Clipboard',
  wootzWalletToolTipCopiedToClipboard: 'Copied',
  wootzWalletPasteFromClipboard: 'Paste from clipboard',
  wootzWalletClickToCopy: 'Click to copy',

  // Accounts Tab
  wootzWalletAccountsPrimary: 'Primary crypto accounts',
  wootzWalletAccountsSecondary: 'Imported accounts',
  wootzWalletConnectedHardwareWallets: 'Connected hardware wallets',
  wootzWalletAccountsAssets: 'Assets',
  wootzWalletAccountsEditVisibleAssets: 'Visible assets',
  wootzWalletAccountBalance: 'Account balance',
  wootzWalletViewAddressOn: 'View address on:',
  wootzWalletNetworkExplorer: '$1 Explorer',

  // Add Account Options
  wootzWalletCreateAccount: 'Create $1 account',
  wootzWalletAddAccountCreate: 'Create',
  wootzWalletAddAccountImport: 'Import',
  wootzWalletAddAccountImportHardware: 'Import from hardware wallet',
  wootzWalletAddAccountHardware: 'Hardware',
  wootzWalletAddAccountConnect: 'Connect',
  wootzWalletAddAccountPlaceholder: 'Name this account',
  wootzWalletCreateAccountButton: 'Create account',
  wootzWalletCreateAccountImportAccount: 'Import $1 account',
  wootzWalletCreateAccountTitle: 'Select one of the following account types',
  wootzWalletCreateAccountEthereumDescription:
    'Supports EVM compatible assets on the Ethereum blockchain ' +
    '(ERC-20, ERC-721, ERC-1551, ERC-1155)',
  wootzWalletCreateAccountSolanaDescription:
    'Supports SPL compatible assets on the Solana blockchain',
  wootzWalletCreateAccountBitcoinDescription: 'Store BTC assets',
  wootzWalletCreateAccountZCashDescription: 'Store ZEC assets',
  wootzWalletCreateAccountFilecoinDescription: 'Store FIL asset',
  wootzWalletFilecoinPrivateKeyProtocol: 'Private key $1',
  wootzWalletEnterPasswordIfApplicable: 'Enter password (if applicable)',
  wootzWalletPrivateKeyImportType: 'Import type',
  wootzWalletFilImportPrivateKeyFormatDescription:
    'NOTE: Private key must be provided as a hex-encoded JSON that contains ' +
    'private key and protocol information. $1Learn more$2.',

  // Import Account
  wootzWalletImportAccountDisclaimer:
    'These accounts can be used with Web3 DApps, and can be shown in your ' +
    'portfolio. However, note that secondary accounts cannot be restored ' +
    'via recovery phrase from your primary account backup.',
  wootzWalletImportAccountPlaceholder: 'Paste private key from clipboard',
  wootzWalletImportAccountKey: 'Private key',
  wootzWalletImportAccountFile: 'JSON file',
  wootzWalletImportAccountUploadButton: 'Choose file',
  wootzWalletImportAccountUploadPlaceholder: 'No file chosen',
  wootzWalletImportAccountError: 'Failed to import account, please try again.',
  wootzWalletImportAccount: 'Import account',

  // Connect Hardware Wallet
  wootzWalletConnectHardwareTitle: 'Select your hardware wallet device',
  wootzWalletConnectHardwareInfo1:
    'Connect your $1 wallet directly to your computer.',
  wootzWalletConnectHardwareInfo2: 'Unlock your device and select the $1 app.',
  wootzWalletConnectHardwareTrezor: 'Trezor',
  wootzWalletConnectHardwareLedger: 'Ledger',
  wootzWalletConnectHardwareDeviceDescription:
    'Connect your $1 device to Wootz Wallet',
  wootzWalletConnectHardwareAuthorizationNeeded:
    'Grant Wootz access to your $1 device.',
  wootzWalletConnectingHardwareWallet: 'Connecting...',
  wootzWalletAddCheckedAccountsHardwareWallet: 'Add checked accounts',
  wootzWalletLoadMoreAccountsHardwareWallet: 'Load more',
  wootzWalletLoadingMoreAccountsHardwareWallet: 'Loading more...',
  wootzWalletSearchScannedAccounts: 'Search account',
  wootzWalletHDPath: 'HD path',
  wootzWalletSwitchHDPathTextHardwareWallet:
    'Try switching HD path (above) if you ' +
    'cannot find the account you are looking for.',
  wootzWalletLedgerLiveDerivationPath: 'Ledger Live',
  wootzWalletLedgerLegacyDerivationPath: 'Legacy (MEW/MyCrypto)',
  wootzWalletUnknownInternalError:
    'Unknown error, please reconnect your hardware wallet and try again.',
  wootzWalletConnectHardwareSearchNothingFound: 'No results found.',
  wootzWalletAuthorizeHardwareWallet: 'Authorize your wallet',
  wootzWalletConnectHardwareWallet: 'Connect hardware wallet',
  wootzWallectConnectHardwareDescription:
    'We currently support $1 and $2 devices.',
  wootzWalletImportHardwareWalletDescription:
    'Connect your hardware wallet to manage your assets directly ' +
    'from Wootz Wallet',
  wootzWalletConnectHardwareWalletSelectBlockchain:
    'Select a blockchain to import your hardware wallet',

  // Account Settings Modal
  wootzWalletAccountSettingsWatchlist: 'Visible assets',
  wootzWalletAccountSettingsPrivateKey: 'Private key',
  wootzWalletAccountSettingsSave: 'Save',
  wootzWalletAccountSettingsRemove: 'Remove account',
  wootzWalletWatchlistAddCustomAsset: 'Add custom asset',
  wootzWalletWatchListTokenName: 'Token name',
  wootzWalletWatchListTokenAddress: 'Token address',
  wootzWalletWatchListTokenSymbol: 'Token symbol',
  wootzWalletWatchListTokenDecimals: 'Decimals of precision',
  wootzWalletWatchListAdd: 'Add',
  wootzWalletWatchListDoneButton: 'Done',
  wootzWalletWatchListNoAsset: 'No assets named',
  wootzWalletWatchListSearchPlaceholder: 'Search assets or contract address',
  wootzWalletWatchListError: 'Failed to add custom token, please try again.',
  wootzWalletCustomTokenExistsError:
    'This token has already been added to your portfolio.',
  wootzWalletAccountSettingsDisclaimer:
    'WARNING: Never share your private key. ' +
    'Anyone with this private key can take your assets forever.',
  wootzWalletAccountSettingsShowKey: 'Show key',
  wootzWalletAccountSettingsHideKey: 'Hide key',
  wootzWalletAccountSettingsUpdateError:
    'Failed to update account name, please try again.',
  wootzWalletWatchListTokenId: 'Token ID (only for ERC721)',
  wootzWalletWatchListTokenIdError: 'Token ID is required',
  wootzWalletWatchListAdvanced: 'Advanced',
  wootzWalletWatchListCoingeckoId: 'Coingecko ID',
  wootzWalletIconURL: 'Icon URL',
  wootzWalletAddAsset: 'Add asset',
  wootzWalletAccountsExport: 'Export',
  wootzWalletAccountsDeposit: 'Deposit',
  wootzWalletAccountsRemove: 'Remove',
  wootzWalletAccountSettings: 'Account settings',
  wootzWalletPasswordIsRequiredToTakeThisAction:
    'Password is required to take this action.',

  // Empty Token List State
  wootzWalletNoAvailableAssets: 'No available assets',
  wootzWalletNoAvailableAssetsDescription:
    "Deposit or purchase tokens to get started. If you don't see tokens " +
    'from an imported account, check the filters and display settings. ' +
    'Unknown tokens may need to be added as custom assets.',

  // AmountPresets
  wootzWalletPreset25: '25%',
  wootzWalletPreset50: '50%',
  wootzWalletPreset75: '75%',
  wootzWalletPreset100: '100%',

  // Ordinals
  wootzWalletOrdinalFirst: 'First',
  wootzWalletOrdinalSecond: 'Second',
  wootzWalletOrdinalThird: 'Third',
  wootzWalletOrdinalFourth: 'Fourth',
  wootzWalletOrdinalFifth: 'Fifth',
  wootzWalletOrdinalSixth: 'Sixth',
  wootzWalletOrdinalSeventh: 'Seventh',
  wootzWalletOrdinalEighth: 'Eighth',
  wootzWalletOrdinalNinth: 'Ninth',
  wootzWalletOrdinalTenth: 'Tenth',
  wootzWalletOrdinalEleventh: 'Eleventh',
  wootzWalletOrdinalTwelfth: 'Twelfth',
  wootzWalletOrdinalThirteenth: 'Thirteenth',
  wootzWalletOrdinalFourteenth: 'Fourteenth',
  wootzWalletOrdinalFifteenth: 'Fifteenth',
  wootzWalletOrdinalSixteenth: 'Sixteenth',
  wootzWalletOrdinalSeventeenth: 'Seventeenth',
  wootzWalletOrdinalEighteenth: 'Eighteenth',
  wootzWalletOrdinalNineteenth: 'Nineteenth',
  wootzWalletOrdinalTwentieth: 'Twentieth',
  wootzWalletOrdinalTwentyFirst: 'Twenty-first',
  wootzWalletOrdinalTwentySecond: 'Twenty-second',
  wootzWalletOrdinalTwentyThird: 'Twenty-third',
  wootzWalletOrdinalTwentyFourth: 'Twenty-fourth',
  wootzWalletOrdinalSuffixOne: 'st',
  wootzWalletOrdinalSuffixTwo: 'nd',
  wootzWalletOrdinalSuffixFew: 'rd',
  wootzWalletOrdinalSuffixOther: 'th',

  // Networks
  wootzWalletNetworkETH: 'Ethereum',
  wootzWalletNetworkMain: 'Mainnet',
  wootzWalletNetworkTest: 'Test Network',
  wootzWalletNetworkLocalhost: 'Localhost',
  wootzWalletAddNetworksAnytimeInSettings:
    'You can add networks anytime in Settings.',
  wootzWalletSupportedNetworks: 'Supported networks',
  wootzWalletChooseChainsToUse:
    'Choose which blockchains to use in your wallet.',
  wootzWalletShowTestnets: 'Show testnets',
  wootzWalletSeeAvailableNetworks: 'See available networks',

  // Grouping Labels
  wootzWalletFeatured: 'Featured',
  wootzWalletPopular: 'Popular',

  // Select Screens
  wootzWalletSelectAccount: 'Select account',
  wootzWalletSearchAccount: 'Search accounts',
  wootzWalletSelectNetwork: 'Select network',
  wootzWalletSelectAsset: 'Select from',
  wootzWalletSearchAsset: 'Search coins',
  wootzWalletSelectCurrency: 'Select currency',
  wootzWalletSearchCurrency: 'Search currencies',

  // Swap
  wootzWalletSwapFrom: 'Amount',
  wootzWalletSwapTo: 'To',
  wootzWalletSwapEstimate: 'estimate',
  wootzWalletSwapMarket: 'Market',
  wootzWalletSwapLimit: 'Limit',
  wootzWalletSwapPriceIn: 'Price in',
  wootzSwapInsufficientBalance: 'Insufficient balance',
  wootzWalletSwapInsufficientFundsForGas: 'Insufficient funds for gas',
  wootzSwapApproveToken: 'Approve $1',
  wootzSwapInsufficientLiquidity: 'Insufficient liquidity',
  wootzWalletSwapInsufficientAllowance: 'Activate token',
  wootzWalletSwapUnknownError: 'Unknown error',
  wootzWalletSwapReviewSpend: 'You spend',
  wootzWalletSwapReviewReceive: "You'll receive",
  wootzWalletSwapReviewHeader: 'Confirm order',
  wootzWalletSolanaSwap: 'Solana Swap',
  wootzWalletNoRoutesFound: 'No routes found',
  wootzWalletReviewSwap: 'Review swap',
  wootzWalletChooseQuoteProvider: 'Choose quote provider',
  wootzWalletQuoteProviderInfo:
    'Each aggregator provides quotes by using liquidity from its API. ' +
    'They gather price information from various exchanges and offer the ' +
    'prices based on this data.',
  wootzWalletNotAvailable: 'Not available',
  wootzWalletUpdate: 'Update',
  wootzWalletProviderNotSupported: 'This pair is not supported by $1',
  wootzWalletChangeProvider: 'Change provider',
  wootzWalletRoute: 'Route',
  wootzWalletMaxSlippage: 'Max slippage',
  wootzWalletMaxSlippageDescription:
    'When the deviation between the price of the transaction you ' +
    'submitted and the price at the time of the transaction on chain is ' +
    'greater than this set value, the transaction will fail.',
  wootzWalletPriceImpactDescription:
    'Price impact is how much your trade might move the market price. ' +
    'Some tokens with low liquidity are more sensitive to trades, so even ' +
    'smaller trades can shift the price a lot.',
  wootzWalletSuggestedValues: 'Suggested Values:',
  wootzWalletMainstreamAssetPairs: 'Mainstream Asset Pairs: $1%',
  wootzWalletStablecoinPairs: 'Stablecoin Pairs: $1%',
  wootzWalletOthers: 'Others: $1%',
  wootzWalletSuggested: 'Suggested',
  wootzWalletCustom: 'Custom',
  wootzWalletCheapest: 'Cheapest',
  wootzWalletFastest: 'Fastest',
  wootzWalletOnNetwork: 'on $1',
  wootzWalletRoutes: 'Routes',
  wootzWalletSwapOnNetworkViaExchange: 'Swap on $1 via $2',
  wootzWalletBridgeFromNetworkToNetwork: 'Bridge from $1 to $2',
  wootzWalletExchangeNamePlusSteps: '$1 + $2 $3',
  wootzWalletExchangeViaProvider: '$1$5$2 via $3$6$4',
  wootzWalletSortBy: 'Sort by',

  // Buy
  wootzWalletBuyTitle: 'Test faucet',
  wootzWalletBuyDescription: 'Get Ether from a faucet for $1',
  wootzWalletBuyFaucetButton: 'Get Ether',
  wootzWalletBuyContinueButton: 'Select purchase method',
  wootzWalletBuySelectAsset: 'Select an asset',
  wootzWalletBuyRampNetworkName: 'Ramp.Network',
  wootzWalletBuySardineName: 'Sardine',
  wootzWalletBuyTransakName: 'Transak',
  wootzWalletBuyStripeName: 'Link by Stripe',
  wootzWalletBuyCoinbaseName: 'Coinbase Pay',
  wootzWalletBuyRampDescription:
    'Buy with CC/Debit or ACH. ' + 'Competitive Rates.',
  wootzWalletBuySardineDescription:
    'Easiest, fastest and cheapest way to ' +
    'buy crypto with card and bank transfers.',
  wootzWalletBuyTransakDescription:
    'Instant buy with your bank account. Lower fees.',
  wootzWalletBuyStripeDescription: 'Pay with credit, debit, bank account',
  wootzWalletBuyCoinbaseDescription:
    'Buy with the most trusted name in crypto.',
  wootzWalletBuyWithRamp: 'Buy with Ramp',
  wootzWalletBuyWithSardine: 'Buy with Sardine',
  wootzWalletBuyWithTransak: 'Buy with Transak',
  wootzWalletBuyWithStripe: 'Buy with Link',
  wootzWalletBuyWithCoinbase: 'Buy with Coinbase Pay',
  wootzWalletSellWithProvider: 'Sell with $1',
  wootzWalletBuyDisclaimer:
    'Financial and transaction data is processed by our onramp partners. ' +
    'Wootz does not collect or have access to such data.',

  // Fund Wallet Screen
  wootzWalletFundWalletTitle:
    'To finish your $1 purchase, select one of our partners',
  wootzWalletFundWalletDescription:
    'On completion, your funds will be transfered to your Wootz Wallet',

  // Deposit Funds Screen
  wootzWalletDepositFundsTitle: 'Deposit crypto',
  wootzWalletDepositX: 'Deposit $1',
  wootzWalletDepositSolSplTokens: 'Deposit Solana or SPL tokens',
  wootzWalletDepositErc: 'Deposit ERC-based tokens',
  wootzWalletDepositOnlySendOnXNetwork:
    'Only send tokens to this address on $1',

  // Sign Transaction Panel
  wootzWalletSignTransactionTitle: 'Your signature is being requested',
  wootzWalletSignWarning:
    'Note that Wootz can’t verify what will happen if you sign. ' +
    'A signature could authorize nearly any operation in your account or ' +
    'on your behalf, including (but not limited to) giving total control ' +
    'of your account and crypto assets to the site making the request. ' +
    'Only sign if you’re sure you want to take this action, ' +
    'and trust the requesting site.',
  wootzWalletSignWarningTitle: 'Sign at your own risk',
  wootzWalletSignTransactionMessageTitle: 'Message',
  wootzWalletSignTransactionEIP712MessageHideDetails: 'Hide details',
  wootzWalletSignTransactionEIP712MessageDomain: 'Domain',
  wootzWalletSignTransactionButton: 'Sign',
  wootzWalletApproveTransaction: 'Approve transaction',
  wootzWalletRiskOfLossAction: 'Unsafe action with risk of loss',
  wootzWalletUntrustedRequestWarning:
    'This request appears malicious, ' +
    'and may leave you vulnerable to theft. ' +
    'Do not continue unless you trust the app.',

  // Sign in with Ethereum
  wootzWalletSignInWithWootzWallet: 'Sign in with Wootz Wallet',
  wootzWalletSignInWithWootzWalletMessage:
    'You are signing into $1. ' +
    'Wootz Wallet will share your wallet address with $1.',
  wootzWalletSeeDetails: 'See details',
  wootzWalletSignIn: 'Sign in',
  wootzWalletOrigin: 'Origin',
  wootzWalletAddress: 'Address',
  wootzWalletStatement: 'Statement',
  wootzWalletUri: 'URI',
  wootzWalletVersion: 'Version',
  wootzWalletNonce: 'Nonce',
  wootzWalletIssuedAt: 'Issued at',
  wootzWalletExpirationTime: 'Expiration time',
  wootzWalletNotBefore: 'Not before',
  wootzWalletRequestId: 'Request ID',
  wootzWalletResources: 'Resources',
  wootzWalletSecurityRiskDetected: 'Security risk detected',

  // Encryption Key Panel
  wootzWalletProvideEncryptionKeyTitle:
    'A DApp is requesting your public encryption key',
  wootzWalletProvideEncryptionKeyDescription:
    '$1$url$2 is requesting your wallets public encryption key. ' +
    'If you consent to providing this key, the site will be able to ' +
    'compose encrypted messages to you.',
  wootzWalletProvideEncryptionKeyButton: 'Provide',
  wootzWalletReadEncryptedMessageTitle:
    'This DApp would like to read this message to complete your request',
  wootzWalletReadEncryptedMessageDecryptButton: 'Decrypt message',
  wootzWalletReadEncryptedMessageButton: 'Allow',

  // Allow Spend ERC20 Panel
  wootzWalletAllowSpendTitle: 'Allow this app to spend your $1?',
  wootzWalletAllowSpendDescription:
    'By granting this permission, you are allowing this app to withdraw ' +
    'your $1 and automate transactions for you.',
  wootzWalletAllowSpendBoxTitle: 'Edit permissions',
  wootzWalletAllowSpendTransactionFee: 'Transaction fee',
  wootzWalletAllowSpendEditButton: 'Edit',
  wootzWalletAllowSpendDetailsButton: 'View details',
  wootzWalletAllowSpendRejectButton: 'Reject',
  wootzWalletAllowSpendConfirmButton: 'Confirm',
  wootzWalletAllowSpendUnlimitedWarningTitle: 'Unlimited approval requested',

  // Allow Add or Change Network Panel
  wootzWalletAllowAddNetworkTitle: 'Allow this site to add a network?',
  wootzWalletAllowAddNetworkDescription:
    'This will allow this network to be used within Wootz Wallet.',
  wootzWalletAllowAddNetworkLearnMoreButton: 'Learn more.',
  wootzWalletAllowAddNetworkName: 'Network name',
  wootzWalletAllowAddNetworkUrl: 'Network URL',
  wootzWalletAllowAddNetworkDetailsButton: 'View all details',
  wootzWalletAllowAddNetworkButton: 'Approve',
  wootzWalletChainId: 'Chain ID',
  wootzWalletAllowAddNetworkCurrencySymbol: 'Currency symbol',
  wootzWalletAllowAddNetworkExplorer: 'Block explorer URL',
  wootzWalletAllowChangeNetworkTitle: 'Allow this site to switch the network?',
  wootzWalletAllowChangeNetworkDescription:
    'This will switch the network to a previously added network.',
  wootzWalletAllowChangeNetworkButton: 'Switch network',
  wootzWalletAllowAddNetworkNetworkPanelTitle: 'Network',

  // Confirm Transaction Panel
  wootzWalletConfirmTransactionTotal: 'Total',
  wootzWalletConfirmTransactionGasFee: 'Gas fee',
  wootzWalletConfirmTransactionTransactionFee: 'Transaction fee',
  wootzWalletConfirmTransactionBid: 'Bid',
  wootzWalletConfirmTransactionAmountGas: 'Amount + gas',
  wootzWalletConfirmTransactionAmountFee: 'Amount + fee',
  wootzWalletConfirmTransactionNoData: 'No data.',
  wootzWalletConfirmTransactionNext: 'next',
  wootzWalletConfirmTransactionFirst: 'first',
  wootzWalletConfirmTransactions: 'transactions',
  wootzWalletConfirmTransactionAccountCreationFee:
    'The associated token account does not exist yet. ' +
    'A small amount of SOL will be spent to create and fund it.',
  wootzWalletAllowSpendCurrentAllowance: 'Current allowance',
  wootzWalletAllowSpendProposedAllowance: 'Proposed allowance',
  wootzWalletTransactionGasLimit: 'Gas Limit',
  wootzWalletTransactionGasPremium: 'Gas Premium',
  wootzWalletTransactionGasFeeCap: 'Gas Fee Cap',
  wootzWalletNetworkFees: 'Network fees',
  wootzWalletTransactionMayIncludeAccountCreationFee:
    'This transaction may include an account creation fee',

  // Wallet Main Panel
  wootzWalletPanelTitle: 'Wootz Wallet',
  wootzWalletPanelConnected: 'Connected',
  wootzWalletPanelBlocked: 'Blocked',
  wootzWalletTitle: 'Wallet',

  // Wallet Welcome Panel
  wootzWalletWelcomePanelDescription:
    'Use this panel to securely access Web3 and all your crypto assets.',

  // Site Permissions Panel
  wootzWalletSitePermissionsAccounts: '$1 accounts connected',
  wootzWalletSitePermissionsDisconnect: 'Disconnect',
  wootzWalletSitePermissionsSwitch: 'Switch',
  wootzWalletSitePermissionsNewAccount: 'New account',
  wootzWalletSitePermissionsTrust: 'Trust',
  wootzWalletSitePermissionsRevoke: 'Revoke',

  // DApp Connection Panel
  wootzWalletChangeNetwork: 'Change network',
  wootzWalletChangeAccount: 'Change account',
  wootzWalletActive: 'Active',
  wootzWalletNotConnected: 'Not connected',
  wootzWalletConnectedAccounts: 'Connected accounts',
  wootzWalletAvailableAccounts: 'Available accounts',

  // Transaction Detail Box
  wootzWalletTransactionDetailBoxFunction: 'FUNCTION TYPE',
  wootzWalletTransactionDetailBoxHex: 'HEX DATA',
  wootzWalletTransactionDetailBoxBytes: 'BYTES',

  // Connect With Site Panel
  wootzWalletConnectWithSiteNext: 'Next',
  wootzWalletConnectWallet: 'Connect wallet',
  wootzWalletConnectWithSite: 'or connect with:',
  wootzWalletConnectPermittedLabel: 'This app will be able to:',
  wootzWalletConnectNotPermittedLabel: 'It will not be able to:',
  wootzWalletConnectPermissionBalanceAndActivity:
    'Check wallet balance and activity',
  wootzWalletConnectPermissionRequestApproval:
    'Request approval for transactions and signatures',
  wootzWalletConnectPermissionAddress: 'View your permitted wallet address',
  wootzWalletConnectPermissionMoveFunds: 'Move funds without your permission',
  wootzWalletConnectTrustWarning: 'Make sure you trust this site.',

  // Permission Duration
  wootzWalletPermissionDuration: 'Permission duration',
  wootzWalletPermissionUntilClose: 'Until I close this site',
  wootzWalletPermissionOneDay: 'For 24 hours',
  wootzWalletPermissionOneWeek: 'For 1 week',
  wootzWalletPermissionForever: 'Forever',

  // Import from Legacy Wallet
  wootzWalletCryptoWalletsDetected: 'Existing crypto wallets detected',
  wootzWalletCryptoWalletsDescriptionTwo:
    'If youd rather skip the import and keep the old Crypto Wallets ' +
    'experience, just navigate to the Wootz Browser $1Settings$2 and ' +
    'change the default back to Crypto Wallets. You can also import, ' +
    'try the new Wootz Wallet, and change back at any time.',
  wootzWalletImportWootzLegacyDescription:
    'Enter your existing crypto wallets password to import to ' +
    'Wootz Wallet. Enjoy a faster and more secure way to manage crypto ' +
    'assets and interact with Web3 DApps.',
  wootzWalletImportWootzLegacyInput: 'Type Crypto wallets password',

  // Connect Hardware Wallet Panel
  wootzWalletConnectHardwarePanelConnected: '$1 connected',
  wootzWalletConnectHardwarePanelDisconnected: '$1 disconnected',
  wootzWalletConnectHardwarePanelInstructions: 'Instructions',
  wootzWalletConnectHardwarePanelConnect: 'Connect your $1',
  wootzWalletConnectHardwarePanelConfirmation:
    'Hardware wallet requires transaction confirmation on device.',
  wootzWalletConnectHardwarePanelOpenApp:
    'Hardware wallet requires $1 App opened on $2',

  // Transaction History Panel (Empty)
  wootzWalletNoTransactionsYet: 'No transaction history',
  wootzWalletNoTransactionsYetDescription:
    'Transactions made with your Wootz Wallet will appear here.',

  // Transaction List Item
  wootzWalletTransactionSent: 'sent',
  wootzWalletTransactionReceived: 'received',
  wootzWalletTransactionExplorerMissing:
    'Block explorer URL' + ' is not available.',
  wootzWalletTransactionExplorer: 'View on block explorer',
  wootzWalletTransactionCopyHash: 'Copy transaction hash',
  wootzWalletTransactionSpeedup: 'Speedup',
  wootzWalletTransactionCancel: 'Cancel transaction',
  wootzWalletTransactionRetry: 'Retry transaction',
  wootzWalletTransactionPlaceholder: 'Transactions will appear here',
  wootzWalletTransactionApproveUnlimited: 'Unlimited',
  wootzWalletApprovalTransactionIntent: 'approve',

  // Transaction Simulation
  wootzWalletScanningTransaction: 'Scanning transaction',
  wootzWalletTransactionPreviewFailed: 'Transaction preview failed',

  // Transaction Simulation Event Grouping Headers
  wootzWalletEstimatedBalanceChange: 'Estimated balance change',
  wootzWalletApprovalDetails: 'Approval details',
  wootzWalletAuthorityChange: 'Authority change',
  wootzWalletOwnershipChange: 'Account ownership change',

  // Transaction Simulation Events
  wootzWalletReceive: 'Receive',
  wootzWalletFrom: 'From',
  wootzWalletUnlimitedAssetAmount: 'Unlimited $1',
  wootzWalletTokenIsUnverified: 'This token is unverified',
  wootzWalletTokenIsVerified: 'This token is verified',
  wootzWalletTokenIsVerifiedByLists: 'This token is verified on $1 lists',
  wootzWalletSpenderAddress: 'Spender: $1',
  wootzWalletNoChangesDetected: 'No changes detected',

  // Transaction Simulation Warnings
  wootzWalletSimulationWarningApprovalToEoa:
    'You’re giving someone else permission to transfer tokens on your behalf.',
  wootzWalletSimulationWarningBulkApprovalsRequest:
    'This requests permission to transfer many tokens, ' +
    'which is usually an attempt at theft.',
  wootzWalletSimulationWarningCopyCatDomain: 'This site may be a fake.',
  wootzWalletSimulationWarningDanglingApproval:
    'This DApp will be able to move tokens from your account later on.',
  wootzWalletSimulationWarningKnownMalicious:
    'This site is flagged as high risk and will ' +
    'likely attempt to steal your funds.',
  wootzWalletSimulationWarningNewDomain:
    'This domain hasn’t been verified yet. ' +
    'Continue only if you trust the site.',
  wootzWalletSimulationWarningPoisonedAddress:
    'Double-check this address' +
    ' as it may be impersonating a previous contact of yours.',
  wootzWalletSimulationWarningSetOwnerAuthority:
    'This transfers ownership of your token balance to someone else.',
  wootzWalletSimulationWarningSuspectedMalicious:
    'This site is flagged as unsafe and/or malicious. ' +
    'Continuing may result in losses.',
  wootzWalletSimulationWarningTooManyTransactionsCritical:
    'This will approve over 200 actions. ' +
    'Unless planned, this may be malicious.',
  wootzWalletSimulationWarningTooManyTransactions:
    'This requests over 200 approvals' +
    '; which is usually an attempt to drain your balances.',
  wootzWalletSimulationWarningTradeForNothing:
    'You’re exchanging tokens for nothing in return. ' +
    'This is likely a mistake.',
  wootzWalletSimulationWarningTransferringErc20ToOwnContract:
    'You’re sending funds to a token’s own address' +
    ' where they’ll be lost forever.',
  wootzWalletSimulationWarningUserAccountOwnerChange:
    'This transfers ownership of your account and balances to someone else.',
  wootzWalletTransactionSimulationNotAvailableForNetwork:
    'Transaction preview is not supported on this network.',
  wootzWalletTransactionSimulationOptedInNotice:
    "You're seeing this message because " +
    'you opted into the transaction preview service.',

  // Transaction Simulation Errors
  wootzWalletSimulationErrorInsufficientFundsForFee:
    'Insufficient funds to cover the network gas fee.',
  wootzWalletSimulationErrorInsufficientFunds:
    'Insufficient funds to complete the transaction.',
  wootzWalletSimulationErrorTransactionReverted:
    'Transaction is likely to fail and revert. ' +
    'Attempting will still incur gas costs.',
  wootzWalletSimulationUnexpectedError:
    'Unexpected error with transaction preview. ' + 'Approving may be unsafe.',

  // Sign Message Simulation/Scanning
  wootzWalletEvmMessageScanningNotSupported:
    'EVM message scanning is not currently supported',
  wootzWalletWhatIsMessageScanning: "What's EVM message scanning?",
  wootzWalletEvmMessageScanningFeatureSafetyExplanation:
    'It ensures safety and accuracy by examining ' +
    'the messages sent from the Ethereum Virtual Machine ' +
    'that interact with your smart contract or any actions.',
  wootzWalletEvmMessageScanningFeatureAccuracyExplanation:
    "It's like double-checking a letter " +
    "to make sure it's just right before signing it.",

  // Asset Detail Accounts (Empty)
  wootzWalletNoAccountsWithABalance: 'No available accounts',
  wootzWalletNoAccountsWithABalanceDescription:
    'Accounts with a balance will appear here.',

  // Edit Gas
  wootzWalletEditGasTitle1: 'Max priority fee',
  wootzWalletEditGasTitle2: 'Edit gas',
  wootzWalletEditGasDescription:
    'While not a guarantee, miners will likely prioritize your ' +
    'transaction if you pay a higher fee.',
  wootzWalletEditGasLow: 'Low',
  wootzWalletEditGasOptimal: 'Optimal',
  wootzWalletEditGasHigh: 'High',
  wootzWalletEditGasLimit: 'Gas limit',
  wootzWalletEditGasPerGasPrice: 'Per-gas price (Gwei)',
  wootzWalletEditGasPerTipLimit: 'Per-gas tip limit (Gwei)',
  wootzWalletEditGasPerPriceLimit: 'Per-gas price limit (Gwei)',
  wootzWalletEditGasMaxFee: 'Max fee',
  wootzWalletEditGasMaximumFee: 'Maximum fee',
  wootzWalletEditGasBaseFee: 'Current base fee',
  wootzWalletEditGasGwei: 'Gwei',
  wootzWalletEditGasSetCustom: 'Set custom',
  wootzWalletEditGasSetSuggested: 'Set suggested',
  wootzWalletEditGasZeroGasPriceWarning:
    'Transaction may not be propagated in the network.',
  wootzWalletEditGasLimitError: 'Gas limit must be an integer greater than 0',
  wootzWalletGasFeeLimitLowerThanBaseFeeWarning:
    'Fee limit is set lower than the base fee. ' +
    'Your transaction may take a long time or fail.',

  // Advanced transaction settings
  wootzWalletAdvancedTransactionSettings: 'Advanced settings',
  wootzWalletAdvancedTransactionSettingsPlaceholder:
    'Enter custom ' + 'nonce value',
  wootzWalletEditNonce: 'Nonce',
  wootzWalletEditNonceInfo:
    'The nonce value will be auto-determined if this field is not specified.',

  // Edit permissions
  wootzWalletEditPermissionsTitle: 'Edit permissions',
  wootzWalletEditPermissionsDescription:
    'Spend limit permission allows $1 to withdraw and spend up to the ' +
    'following amount:',
  wootzWalletEditPermissionsButton: 'Edit permissions',
  wootzWalletEditPermissionsProposedAllowance: 'Proposed allowance',
  wootzWalletEditPermissionsCustomAllowance: 'Custom allowance',

  // Send Input Errors
  wootzWalletNotValidFilAddress: 'Not a valid FIL address',
  wootzWalletNotValidEthAddress: 'Not a valid ETH address',
  wootzWalletNotValidSolAddress: 'Not a valid SOL address',
  wootzWalletNotValidAddress: 'Not a valid $1 address.',
  wootzWalletNotDomain: "Domain doesn't have a linked $1 address.",
  wootzWalletSameAddressError: 'The receiving address is your own address.',
  wootzWalletContractAddressError:
    'The receiving address is a tokens contract address.',
  wootzWalletFailedChecksumTitle: 'Address doesn’t look correct',
  wootzWalletFailedChecksumDescription:
    'Check your address to make sure it’s the right address ' +
    '(e.g. letters with lower or upper case).',
  wootzWalletHowToSolve: 'How can I solve it?',
  wootzWalletAddressMissingChecksumInfoWarning:
    'This address cannot be verified (missing checksum). Proceed?',
  wootzWalletNotValidChecksumAddressError:
    'Address did not pass verification (invalid checksum). ' +
    'Please try again, replacing lowercase letters with uppercase.',
  wootzWalletMissingGasLimitError: 'Missing gas limit',
  wootzWalletZeroBalanceError: 'Amount must be greater than 0',
  wootzWalletAddressRequiredError: 'To address is required',
  wootzWalletChecksumModalTitle: 'How can I find the right address?',
  wootzWalletChecksumModalDescription:
    'Wootz prevents users from sending funds to the wrong address due to ' +
    'incorrect capitalization. Make sure that it’s the correct address by ' +
    'following the instructions below.',
  wootzWalletChecksumModalStepOneTitle: '1. Visit',
  wootzWalletChecksumModalStepOneDescription:
    'Visit etherscan and paste the wallet address you want to send ' +
    'tokens. Then enter.',
  wootzWalletChecksumModalStepTwoTitle: '2. Copy and enter ETH address',
  wootzWalletChecksumModalStepTwoDescription:
    'Copy and enter the correct address. ' +
    'You can see that some characters have been converted correctly.',
  wootzWalletChecksumModalNeedHelp: 'Need more help?',

  // Transaction Queue Strings
  wootzWalletQueueOf: 'of',
  wootzWalletQueueNext: 'next',
  wootzWalletQueueFirst: 'first',
  wootzWalletQueueRejectAll: 'Reject transactions',

  // Add Suggested Token Panel
  wootzWalletAddSuggestedTokenTitle: 'Add suggested token',
  wootzWalletAddSuggestedTokenDescription:
    'Would you like to import this token?',

  // Transaction Detail Panel
  wootzWalletRecentTransactions: 'Recent transactions',
  wootzWalletTransactionDetailHash: 'Transaction hash',
  wootzWalletTransactionDetailNetwork: 'Network',

  // Transactions Status
  wootzWalletTransactionStatusUnapproved: 'Unapproved',
  wootzWalletTransactionStatusApproved: 'Approved',
  wootzWalletTransactionStatusRejected: 'Rejected',
  wootzWalletTransactionStatusSubmitted: 'Submitted',
  wootzWalletTransactionStatusConfirmed: 'Confirmed',
  wootzWalletTransactionStatusError: 'Error',
  wootzWalletTransactionStatusDropped: 'Dropped',
  wootzWalletTransactionStatusSigned: 'Signed',

  // Transaction Details
  wootzWalletOn: 'On',

  // NFT Details Page
  wootzWalletNFTDetailBlockchain: 'Blockchain',
  wootzWalletNFTDetailTokenStandard: 'Token standard',
  wootzWalletNFTDetailTokenID: 'Token ID',
  wootzWalletNFTDetailContractAddress: 'Contract address',
  wootzWalletNFTDetailDescription: 'Description',
  wootzWalletNFTDetailImageAddress: 'Image URL',
  wootzWalletNFTDetailsNotAvailable: 'Not available yet',
  wootzWalletNFTDetailsOverview: 'Overview',
  wootzWalletNFTDetailsOwnedBy: 'Owned by',
  wootzWalletNFTDetailsViewAccount: 'View account',
  wootzWalletNFTDetailsPinningStatusLabel: 'IPFS pinning status',
  wootzWalletNFTDetailsCid: 'CID',
  wootzWalletNFTDetailsProperties: 'Properties',

  // Sweepstakes
  wootzWalletSweepstakesTitle: 'Wootz Swap-stakes',
  wootzWalletSweepstakesDescription:
    '7 days of crypto giveaways, ~$500k in prizes.',
  wootzWalletSweepstakesCallToAction: 'Enter now!',

  // Market Data Filters
  wootzWalletMarketDataAllAssetsFilter: 'All Assets',
  wootzWalletMarketDataTradableFilter: 'Tradable',
  wootzWalletMarketDataAssetsColumn: 'Assets',
  wootzWalletMarketDataPriceColumn: 'Price',
  wootzWalletMarketData24HrColumn: '24Hr',
  wootzWalletMarketDataMarketCapColumn: 'Cap',
  wootzWalletMarketDataVolumeColumn: 'Volume',
  wootzWalletMarketDataBuyDepositColumn: 'Buy/Deposit',

  // Market Coin Detail Screen
  wootzWalletInformation: 'Information',
  wootzWalletRankStat: 'Rank',
  wootzWalletVolumeStat: '24h Volume',
  wootzWalletMarketCapStat: 'Market Cap',

  // Network Filter
  wootzWalletNetworkFilterAll: 'All Networks',
  wootzWalletNetworkFilterSecondary: 'Secondary networks',
  wootzWalletNetworkFilterTestNetworks: 'Test networks',

  // Asset Filter
  wootzWalletAssetFilterLowToHigh: 'Low to High',
  wootzWalletAssetFilterHighToLow: 'High to Low',
  wootzWalletAssetFilterAToZ: 'A to Z',
  wootzWalletAssetFilterZToA: 'Z to A',

  // Asset Group By
  wootzWalletNone: 'None',
  wootzWalletNetworks: 'Networks',
  wootzWalletPortfolioGroupByTitle: 'Group by',
  wootzWalletGroupByCollection: 'Group by collection',
  wootzWalletPortfolioGroupByDescription: 'Group assets by',

  // Portfolio Filters
  wootzWalletPortfolioFiltersTitle: 'Filters and display settings',
  wootzWalletPortfolioNftsFiltersTitle: 'NFT display settings',
  wootzWalletSortAssets: 'Sort assets',
  wootzWalletSortAssetsDescription: 'Sort by fiat value or name',
  wootzWalletHideSmallBalances: 'Hide small balances',
  wootzWalletHideSmallBalancesDescription: 'Assets with value less than $1',
  wootzWalletSelectAccounts: 'Select accounts',
  wootzWalletSelectNetworks: 'Select networks',
  wootzWalletSelectAll: 'Select all',
  wootzWalletDeselectAll: 'Deselect all',
  wootzWalletPrimaryNetworks: 'Primary networks',
  wootzWalletETHAccountDescription: 'Ethereum + EVM Chains',
  wootzWalletSOLAccountDescription: 'Solana + SVM Chains',
  wootzWalletFILAccountDescription: 'Filecoin',
  wootzWalletBTCMainnetAccountDescription: 'Bitcoin Mainnet',
  wootzWalletBTCTestnetAccountDescription: 'Bitcoin Testnet',
  wootzWalletZECAccountDescription: 'Zcash Mainnet',
  wootzWalletZECTestnetAccountDescription: 'Zcash Testnet',
  wootzWalletShowNetworkLogoOnNftsTitle: 'Network Logo',
  wootzWalletShowNetworkLogoOnNftsDescription: 'Show network logo on NFTs',
  wootzWalletShowSpamNftsTitle: 'Spam NFTs',
  wootzWalletShowSpamNftsDescription: 'Show Spam NFTs',
  wootzWalletPortfolioSettings: 'Portfolio Settings',
  wootzWalletHideNotOwnedNfTs: 'Hide not owned NFTs',

  // Account Filter
  wootzWalletAccountFilterAllAccounts: 'All accounts',

  // Transaction post-confirmation

  // Submitted
  wootzWalletTransactionSubmittedTitle: 'Transaction submitted',
  wootzWalletTransactionSubmittedDescription:
    'Transaction has been successfully sent ' +
    'to the network and awaits confirmation.',

  // Failed
  wootzWalletTransactionFailedHeaderTitle: '$1 was returned to your wallet',
  wootzWalletTransactionFailedTitle: 'Transaction failed',
  wootzWalletTransactionFailedDescription:
    'Transaction was failed due to a large price movement. ' +
    'Increase slippage tolerance to succeed at a larger price movement.',
  wootzWalletTransactionFailedSwapNextCTA: 'New trade',
  wootzWalletTransactionFailedNextCTA: 'New transaction',
  wootzWalletTransactionFailedViewErrorCTA: 'View error',
  wootzWalletTransactionFailedReceiptCTA: 'Receipt',
  wootzWalletTransactionFailedModalTitle: 'Error message',
  wootzWalletTransactionFailedModalSubtitle:
    'Please save the error message for future reference.',

  // Complete
  wootzWalletTransactionCompleteSwapHeaderTitle: 'Swapped $1 to $2',
  wootzWalletTransactionCompleteTitle: 'Transaction complete!',
  wootzWalletTransactionCompleteDescription:
    'Transaction was successful. Please wait for confirmations, ' +
    'to avoid the risk of double-spend.',
  wootzWalletTransactionCompleteReceiptCTA: 'Receipt',

  // Confirming
  wootzWalletTransactionConfirmingTitle: 'Transaction is processing',
  // [FIXME]: change the wording after ETH2.
  wootzWalletTransactionConfirmingDescription:
    'Transaction was successfully included in a block. ' +
    'To avoid the risk of double spending, ' +
    'we recommend waiting for block confirmations.',
  wootzWalletTransactionConfirmingText: 'Confirming',

  // Transaction intents for confirmation panels
  wootzWalletTransactionIntentDappInteraction: 'Dapp interaction',
  wootzWalletTransactionIntentSend: 'Send $1',
  wootzWalletTransactionIntentSwap: 'Swap $1 to $2',

  // Solana ProgramID Names
  wootzWalletSolanaSystemProgram: 'System Program',
  wootzWalletSolanaConfigProgram: 'Config Program',
  wootzWalletSolanaStakeProgram: 'Stake Program',
  wootzWalletSolanaVoteProgram: 'Vote Program',
  wootzWalletSolanaBPFLoader: 'BPF Loader',
  wootzWalletSolanaEd25519Program: 'Ed25519 Program',
  wootzWalletSolanaSecp256k1Program: 'Secp256k1 Program',
  wootzWalletSolanaTokenProgram: 'Token Program',
  wootzWalletSolanaAssociatedTokenProgram: 'Associated Token Program',
  wootzWalletSolanaMetaDataProgram: 'MetaData Program',
  wootzWalletSolanaSysvarRentProgram: 'SysvarRent Program',

  // Solana Instruction Parameter Names
  wootzWalletSolanaAccounts: 'Accounts:',
  wootzWalletSolanaData: 'Data:',
  wootzWalletSolanaProgramID: 'Program ID:',
  wootzWalletSolanaMaxRetries: 'Max Retries:',
  wootzWalletSolanaPreflightCommitment: 'Preflight Commitment:',
  wootzWalletSolanaSkipPreflight: 'Skip Preflight:',
  wootzWalletSolanaAddressLookupTableAccount: 'Address Lookup Table Account:',
  wootzWalletSolanaAddressLookupTableIndex: 'Address Lookup Table Index:',

  // Help Center
  wootzWalletHelpCenter: 'Help center',
  wootzWalletHelpCenterText: 'Need help? See',

  // Remove Account Modal
  wootzWalletRemoveAccountModalTitle: 'Are you sure you want to remove "$1"?',

  // Bridge to Aurora
  wootzWalletAuroraModalTitle: 'Open the Rainbow Bridge app?',
  wootzWalletAuroraModalDescription:
    'Rainbow Bridge is an independent service that helps you bridge ' +
    'assets across networks, and use your crypto on other networks ' +
    'and DApp ecosystems. Bridging assets to other networks has some risks.',
  wootzWalletAuroraModalLearnMore: 'Learn more about using Rainbow Bridge',
  wootzWalletAuroraModalLearnMoreAboutRisk:
    'Learn more about mitigating risk on Rainbow Bridge',
  wootzWalletAuroraModalDontShowAgain: "Don't show again",
  wootzWalletAuroraModalOPenButtonText: 'Open the Rainbow Bridge app',
  wootzWalletBridgeToAuroraButton: 'Bridge to Aurora',

  // Input field labels
  wootzWalletInputLabelPassword: 'Password',

  // Wallet Welcome Perks
  wootzWalletPerksTokens: 'Buy, send, and swap 200+ crypto assets',
  wootzWalletMultiChain: 'Multi-chain & NFT support',
  wootzWalletPerksBrowserNative: 'Browser-native, no risky extensions',

  // Portfolio Asset More Popup
  wootzWalletPortfolioTokenDetailsMenuLabel: 'Token details',
  wootzWalletPortfolioViewOnExplorerMenuLabel: 'View on block explorer',
  wootzWalletPortfolioHideTokenMenuLabel: 'Hide token',
  wootzWalletHideTokenModalTitle: 'Hide token',

  // Token detail modals
  wootzWalletMakeTokenVisibleInstructions:
    'You can make this asset visible again in the future by clicking ' +
    'the "+ Visible assets" button at the bottom of the "Portfolio" tab',
  wootzWalletConfirmHidingToken: 'Hide',
  wootzWalletCancelHidingToken: 'Cancel',

  // Visible assets modal
  wootzWalletMyAssets: 'My assets',
  wootzWalletAvailableAssets: 'Available assets',
  wootzWalletDidntFindAssetEndOfList: "Didn't find your asset on the list?",
  wootzWalletDidntFindAssetInList:
    "If you didn't find your asset in this list, you can add it manually " +
    'by using the button below',
  wootzWalletAssetNotFound: 'Asset not found',

  // Request feature button
  wootzWalletRequestFeatureButtonText: 'Request feature',

  // Warnings
  wootzWalletNonAsciiCharactersInMessageWarning:
    'Non-ASCII characters detected!',
  wootzWalletFoundRisks: 'We found $1 risks.',
  wootzWalletFoundIssues: 'We found $1 issues.',

  // ASCII toggles
  wootzWalletViewEncodedMessage: 'View original message',
  wootzWalletViewDecodedMessage: 'View message in ASCII encoding',

  // NFTs Tab
  wootzNftsTabImportNft: 'Import NFT',
  wootzNftsTabEmptyStateHeading: 'No NFTs here yet.',
  wootzNftsTabEmptyStateSubHeading:
    'Ready to add some? Just click the button below to import.',
  wootzNftsTabEmptyStateDisclaimer:
    'Compatible with NFTs on Solana (SPL) and Ethereum (ERC-721).',
  wootzNftsTab: 'NFTs',
  wootzNftsTabHidden: 'Hidden',
  wootzNftsTabCollected: 'Collected',
  wootzNftsTabHide: 'Hide',
  wootzNftsTabUnhide: 'Unhide',
  wootzNftsTabEdit: 'Edit',
  wootzNftsTabRemove: "Don't show in wallet",

  // Add asset tabs
  wootzWalletAddAssetTokenTabTitle: 'Token',
  wootzWalletAddAssetNftTabTitle: 'NFT',
  wootzWalletNftFetchingError:
    'Something went wrong when fetching NFT details. Please try again later.',
  wootzWalletEditToken: 'Edit Token',

  // Add Custom Asset Form
  wootzWalletNetworkIsRequiredError: 'Network is required',
  wootzWalletTokenNameIsRequiredError: 'Token name is required',
  wootzWalletInvalidTokenContractAddressError:
    'Invalid or empty token contract address',
  wootzWalletTokenSymbolIsRequiredError: 'Token symbol is required',
  wootzWalletTokenDecimalsIsRequiredError:
    'Token decimals of precision value is required',
  wootzWalletTokenContractAddress: 'Token Contract Address',
  wootzWalletTokenDecimal: 'Token Decimal',
  wootzWalletTokenMintAddress: 'Mint address',
  wootzWalletTransactionHasFeeEstimatesError: 'Unable to fetch fee estimates',

  wootzWalletEditNftModalTitle: 'Edit NFT',
  wootzWalletNftMoveToSpam: 'Mark as junk',
  wootzWalletNftUnspam: 'Mark as not junk',

  // NFT Labels
  wootzWalletNftJunk: 'Junk',
  wootzWalletWatchOnly: 'Watch-only',

  // Add NFT modal
  wootzWalletAddNftModalTitle: 'Add NFT',
  wootzWalletAddNftModalDescription:
    "If you can't see an NFT automatically detected by Wootz, " +
    'or want to track one owned by others, you can import it manually.',
  wootzWalletWhatIsAnNftContractAddress:
    'The contract address when adding an NFT to a wallet is the unique ' +
    'address on the blockchain where the smart contract ' +
    'for the NFT collection resides. ' +
    'You can find the contract address from marketplace, or ' +
    'blockchain explorers, etc.',
  wootzWalletWhatIsAnNftTokenId:
    'The unique identifier for the specific NFT within the collection.',
  wootzWalletNftNameFieldExplanation:
    'The token name for an NFT refers to the specific name given to a ' +
    'non-fungible token within a particular collection or project. ' +
    'This name distinguishes one NFT from another within the same collection.',
  wootzWalletNftSymbolFieldExplanation:
    'The NFT symbol refers to the unique identifier ' +
    'used to distinguish the NFT collection or project.',
  wootzWalletFetchNftMetadataError: 'Unable to fetch NFT metadata',
  wootzWalletWatchThisNft: 'Watch this NFT',
  wootzWalletUnownedNftAlert:
    'You do not own this NFT. ' +
    'While you can still add it to your wallet, ' +
    'you will not be able to make any transactions with it.',

  // Remove NFT modal
  wootzWalletRemoveNftModalHeader: 'Remove from Wootz Wallet?',
  wootzWalletRemoveNftModalDescription:
    'NFT will be removed from Wootz Wallet but will remain on the ' +
    'blockchain. If you remove it, then change your mind, ' +
    "you'll need to import it again manually.",
  wootzWalletRemoveNftModalCancel: 'Cancel',
  wootzWalletRemoveNftModalConfirm: 'Remove',

  // NFT auto discovery modal
  wootzWalletEnableNftAutoDiscoveryModalHeader:
    'Want your NFTs displayed automatically?',
  wootzWalletEnableNftAutoDiscoveryModalDescription:
    'Wootz Wallet can use a third-party service to automatically display ' +
    'your NFTs. Wootz will share your wallet addresses with ' +
    '$1SimpleHash$2 to provide this service. $3Learn more.$4',
  wootzWalletEnableNftAutoDiscoveryModalConfirm: 'Yes, proceed',
  wootzWalletEnableNftAutoDiscoveryModalCancel:
    "No thanks, I'll do it manually",
  wootzWalletAutoDiscoveryEmptyStateHeading: 'No NFTs to display',
  wootzWalletAutoDiscoveryEmptyStateSubHeading:
    'Once an NFT is detected, it’ll be displayed here.',
  wootzWalletAutoDiscoveryEmptyStateFooter: 'Can’t see your NFTs?',
  wootzWalletAutoDiscoveryEmptyStateActions:
    '$1Refresh$2 or $3Import Manually$4',
  wootzWalletAutoDiscoveryEmptyStateRefresh: 'Refreshing',

  // Wootz Wallet Rewards
  wootzWalletUphold: 'Uphold',
  wootzWalletGemini: 'Gemini',
  wootzWalletZebpay: 'Zebpay',
  wootzWalletBitflyer: 'bitFlyer',
  wootzWalletLogIn: 'Log in',
  wootzWalletViewOn: 'View on $1',
  wootzWalletPlatforms: 'Platforms',
  wootzWalletTestNetworkAccount: 'Test Network Account',

  // Transaction Simulations Opt-in
  wootzWalletEnableTransactionSimulation: 'Enable transaction simulation',
  wootzWalletTransactionSimulationFeatureDescription:
    'Simulate how a transaction will behave before authorizing it',
  wootzWalletTransactionSimulationSeeEstimates:
    'Preview your estimated balance change',
  wootzWalletTransactionSimulationDetectMalicious:
    'Identify potentially malicious transactions',
  wootzWalletTransactionSimulationDetectPhishing:
    'Help detect phishing attempts',
  wootzWalletTransactionSimulationTerms:
    'This service is provided by $1Blowfish.xyz$2 and is subject to their $3Terms ' +
    'of Service$4 and $5Privacy Policy$6.',

  // Settings
  wootzWalletChangeAnytimeInSettings: 'Change anytime in $1Settings$2.',

  // Balance Details Modal
  wootzWalletAvailable: 'Available',
  wootzWalletAvailableBalanceDescription: 'Funds available for you to use.',
  wootzWalletPending: 'Pending',
  wootzWalletPendingBalanceDescription:
    'A pending change in your wallet balance.',
  wootzWalletTotalBalanceDescription:
    'Your available funds plus any not-yet-confirmed transactions.',
  wootzWalletUnavailableBalances: 'Some balances may be unavailable',

  // Misc. placeholders
  wootzWalletExempliGratia: 'e.g $1',

  // Explore
  wootzWalletWeb3: 'Web3',
  wootzWalletTopNavExplore: 'Explore',
  wootzWalletNoDappsFound: 'No DApps found',
  wootzWalletActiveWallets: 'Active wallets',
  wootzWalletVisitDapp: 'Visit $1',
  wootzWalletFilters: 'Filters',
  wootzWalletClearFilters: 'Clear filters',
  wootzWalletShowMore: 'Show more',
  wootzWalletDetails: 'Details'
})
