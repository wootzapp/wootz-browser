// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'
import { skipToken } from '@reduxjs/toolkit/query/react'
import Input, { InputEventDetail } from '@brave/leo/react/input'
import Alert from '@brave/leo/react/alert'

// utils
import { WootzWallet } from '../../../constants/types'
import Amount from '../../../utils/amount'
import { getLocale } from '$web-common/locale'
import {
  networkEntityAdapter,
  emptyNetworksRegistry
} from '../../../common/slices/entities/network.entity'
import withPlaceholderIcon from '../create-placeholder-icon'
import {
  getAssetIdKey,
  type GetBlockchainTokenIdArg
} from '../../../utils/asset-utils'

// hooks
import useGetTokenInfo from '../../../common/hooks/use-get-token-info'
import {
  useAddUserTokenMutation,
  useGetIsTokenOwnedByUserQuery,
  useGetNetworksRegistryQuery,
  useGetNftMetadataQuery,
  useUpdateUserTokenMutation
} from '../../../common/slices/api.slice'
import {
  useGetCustomAssetSupportedNetworks //
} from '../../../common/hooks/use_get_custom_asset_supported_networks'

// components
import { NetworksDropdown } from '../dropdowns/networks_dropdown'
import { FormErrorsList } from './form-errors-list'
import { NftIcon } from '../nft-icon/nft-icon'
import { InfoIconTooltip } from '../info_icon_tooltip/info_icon_tooltip'

// styles
import {
  ButtonRow,
  ButtonRowSpacer,
  ErrorText,
  FormWrapper,
  FullWidthFormColumn,
  InputLabel,
  InputLoadingIndicator,
  PreviewImageContainer,
  TokenNamePreviewText,
  TokenTickerPreviewText,
  DescriptionRow
} from './add-custom-token-form-styles'
import { Column, LeoSquaredButton, Row } from '../style'
import { Skeleton } from '../loading-skeleton/styles'
import Icon from '@brave/leo/react/icon'
import Tooltip from '../tooltip'

const NftIconWithPlaceholder = withPlaceholderIcon(NftIcon, {
  size: 'extra-big',
  marginLeft: 0,
  marginRight: 0
})

interface Props {
  selectedAsset?: WootzWallet.BlockchainToken
  contractAddress: string
  onHideForm: () => void
  onChangeContractAddress: (contractAddress: string) => void
}

export const AddNftForm = (props: Props) => {
  const {
    selectedAsset,
    contractAddress: tokenContractAddress,
    onHideForm,
    onChangeContractAddress
  } = props

  const { data: networksRegistry = emptyNetworksRegistry } =
    useGetNetworksRegistryQuery()
  const selectedAssetNetwork = selectedAsset
    ? networksRegistry.entities[networkEntityAdapter.selectId(selectedAsset)]
    : undefined

  // state
  const [showNetworkDropDown, setShowNetworkDropDown] =
    React.useState<boolean>(false)
  const [hasError, setHasError] = React.useState<boolean>(false)

  // Form States
  const [customTokenName, setCustomTokenName] = React.useState(
    selectedAsset?.name || ''
  )
  const [customTokenID, setCustomTokenID] = React.useState<string | undefined>(
    selectedAsset?.tokenId
  )
  const [customTokenSymbol, setCustomTokenSymbol] = React.useState(
    selectedAsset?.symbol || ''
  )
  const [customAssetsNetwork, setCustomAssetsNetwork] = React.useState<
    WootzWallet.NetworkInfo | undefined
  >(selectedAssetNetwork)

  // mutations
  const [addUserToken] = useAddUserTokenMutation()
  const [updateUserToken] = useUpdateUserTokenMutation()

  // queries
  const {
    tokenInfo: matchedTokenInfo,
    isLoading: isTokenInfoLoading,
    isError: hasGetTokenInfoError
  } = useGetTokenInfo(
    customAssetsNetwork &&
      tokenContractAddress &&
      (customAssetsNetwork.coin === WootzWallet.CoinType.ETH
        ? !!customTokenID
        : true)
      ? {
          contractAddress: tokenContractAddress,
          tokenId: customTokenID,
          network: {
            chainId: customAssetsNetwork.chainId,
            coin: customAssetsNetwork.coin
          }
        }
      : skipToken
  )

  const networkList = useGetCustomAssetSupportedNetworks()

  const metadataLookupArg: GetBlockchainTokenIdArg | undefined =
    React.useMemo(() => {
      if (!customAssetsNetwork || !tokenContractAddress) {
        return undefined
      }

      return {
        chainId: customAssetsNetwork.chainId,
        coin: customAssetsNetwork.coin,
        contractAddress: tokenContractAddress,
        tokenId:
          customAssetsNetwork.coin !== WootzWallet.CoinType.SOL && customTokenID
            ? new Amount(customTokenID).toHex()
            : '',
        isErc721:
          !!customTokenID &&
          customAssetsNetwork.coin !== WootzWallet.CoinType.SOL,
        isNft: true
      }
    }, [customAssetsNetwork, tokenContractAddress, customTokenID])

  // TODO: need symbol in response in order to simplify adding SOL NFTs
  const {
    data: nftMetadata,
    isFetching: isFetchingNftMetadata,
    isError: hasNftMetadataError
  } = useGetNftMetadataQuery(
    metadataLookupArg &&
      (metadataLookupArg.coin === WootzWallet.CoinType.SOL ||
        metadataLookupArg.tokenId)
      ? metadataLookupArg
      : skipToken
  )

  const { data: userOwnsNft, isFetching: isFetchingBalanceCheck } =
    useGetIsTokenOwnedByUserQuery(metadataLookupArg ?? skipToken)

  // computed
  const customAssetsNetworkError = !customAssetsNetwork?.chainId
  const tokenNameError = !customTokenName
  const tokenSymbolError = !customTokenSymbol
  const tokenIdError =
    selectedAssetNetwork?.coin === WootzWallet.CoinType.ETH && !customTokenID
  const tokenContractAddressError =
    tokenContractAddress === '' ||
    (customAssetsNetwork?.coin !== WootzWallet.CoinType.SOL &&
      !tokenContractAddress.toLowerCase().startsWith('0x'))

  const buttonDisabled =
    isTokenInfoLoading ||
    tokenNameError ||
    tokenSymbolError ||
    tokenContractAddressError ||
    customAssetsNetworkError

  // memos
  const formErrors = React.useMemo(() => {
    return [
      tokenContractAddressError &&
        getLocale('wootzWalletInvalidTokenContractAddressError'),
      customAssetsNetworkError &&
        getLocale('wootzWalletNetworkIsRequiredError'),
      tokenNameError && getLocale('wootzWalletTokenNameIsRequiredError'),
      tokenSymbolError && getLocale('wootzWalletTokenSymbolIsRequiredError'),
      tokenIdError && getLocale('wootzWalletWatchListTokenIdError')
    ]
  }, [
    tokenContractAddressError,
    customAssetsNetworkError,
    tokenNameError,
    tokenSymbolError,
    tokenIdError
  ])

  const metadataAssetInfo:
    | (GetBlockchainTokenIdArg &
        Pick<WootzWallet.BlockchainToken, 'logo' | 'name' | 'symbol'>)
    | undefined = React.useMemo(() => {
    return metadataLookupArg &&
      !isFetchingNftMetadata &&
      (metadataLookupArg?.coin === WootzWallet.CoinType.SOL || customTokenID)
      ? {
          ...metadataLookupArg,
          logo:
            nftMetadata?.imageURL ||
            matchedTokenInfo?.logo ||
            selectedAsset?.logo ||
            '',
          name: customTokenName,
          // TODO: response from core currently doesn't have symbol
          symbol: customTokenSymbol
        }
      : undefined
  }, [
    customTokenID,
    customTokenName,
    customTokenSymbol,
    isFetchingNftMetadata,
    matchedTokenInfo?.logo,
    nftMetadata?.imageURL,
    selectedAsset?.logo,
    metadataLookupArg
  ])

  // methods
  const resetBaseInputFields = React.useCallback(() => {
    setCustomTokenID(undefined)
  }, [])

  const resetInputFields = React.useCallback(() => {
    resetBaseInputFields()
    onChangeContractAddress('')
  }, [resetBaseInputFields, onChangeContractAddress])

  const handleTokenNameChanged = React.useCallback(
    (detail: InputEventDetail) => {
      setHasError(false)
      setCustomTokenName(detail.value)
    },
    []
  )

  const handleTokenIDChanged = React.useCallback((detail: InputEventDetail) => {
    setHasError(false)
    setCustomTokenID(detail.value)
  }, [])

  const handleTokenSymbolChanged = React.useCallback(
    (detail: InputEventDetail) => {
      setHasError(false)
      setCustomTokenSymbol(detail.value)
    },
    []
  )

  const handleTokenAddressChanged = React.useCallback(
    (event: InputEventDetail) => {
      setHasError(false)
      onChangeContractAddress(event.value)
    },
    [onChangeContractAddress]
  )

  const addOrUpdateToken = React.useCallback(async () => {
    if (!customAssetsNetwork) {
      return
    }

    const updatedToken: WootzWallet.BlockchainToken = {
      name: customTokenName,
      symbol: customTokenSymbol,
      decimals: 0,
      coingeckoId: '',
      logo: matchedTokenInfo?.logo || selectedAsset?.logo || '',
      isCompressed: false, // isCompressed will be set by the backend
      isErc20: false,
      isErc1155: false,
      splTokenProgram: WootzWallet.SPLTokenProgram.kUnknown,
      isSpam: false,
      visible: true,
      chainId: customAssetsNetwork.chainId,
      coin: customAssetsNetwork.coin,
      contractAddress: tokenContractAddress,
      tokenId:
        customAssetsNetwork.coin !== WootzWallet.CoinType.SOL && customTokenID
          ? new Amount(customTokenID).toHex()
          : '',
      isNft: true,
      isErc721:
        matchedTokenInfo?.isErc721 ??
        (!!customTokenID &&
          customAssetsNetwork.coin !== WootzWallet.CoinType.SOL)
    }

    if (selectedAsset) {
      // remove existing token and add new one
      await updateUserToken({
        existingToken: selectedAsset,
        updatedToken
      }).unwrap()
      onHideForm()
      return
    }

    try {
      await addUserToken(updatedToken).unwrap()
      onHideForm()
    } catch (error) {
      setHasError(true)
    }
  }, [
    customAssetsNetwork,
    customTokenName,
    customTokenSymbol,
    matchedTokenInfo,
    selectedAsset,
    tokenContractAddress,
    customTokenID,
    updateUserToken,
    onHideForm,
    addUserToken
  ])

  const onHideNetworkDropDown = React.useCallback(() => {
    if (showNetworkDropDown) {
      setShowNetworkDropDown(false)
    }
  }, [showNetworkDropDown])

  const onSelectCustomNetwork = React.useCallback(
    (network: WootzWallet.NetworkInfo) => {
      setCustomAssetsNetwork(network)
      onHideNetworkDropDown()
    },
    [onHideNetworkDropDown]
  )

  const onClickCancel = React.useCallback(() => {
    resetInputFields()
    onHideForm()
  }, [resetInputFields, onHideForm])

  // effects
  React.useEffect(() => {
    // wait for new data before syncing
    if (isFetchingNftMetadata || isTokenInfoLoading) {
      return
    }

    // sync form fields with found token info
    if (!hasGetTokenInfoError && matchedTokenInfo?.name) {
      setCustomTokenName(matchedTokenInfo.name)
    }

    if (!hasNftMetadataError && nftMetadata?.contractInformation.name) {
      setCustomTokenName(nftMetadata.contractInformation.name)
    }

    if (!hasGetTokenInfoError && matchedTokenInfo?.symbol) {
      setCustomTokenSymbol(matchedTokenInfo.symbol)
    }
  }, [
    hasGetTokenInfoError,
    hasNftMetadataError,
    isFetchingNftMetadata,
    isTokenInfoLoading,
    matchedTokenInfo?.name,
    matchedTokenInfo?.symbol,
    nftMetadata?.contractInformation.name
  ])

  // render
  return (
    <>
      <FormWrapper onClick={onHideNetworkDropDown}>
        {!selectedAsset && (
          <FullWidthFormColumn>
            <DescriptionRow>
              {getLocale('wootzWalletAddNftModalDescription')}
            </DescriptionRow>
          </FullWidthFormColumn>
        )}

        <FullWidthFormColumn>
          <NetworksDropdown
            placeholder={getLocale('wootzWalletSelectNetwork')}
            networks={networkList}
            onSelectNetwork={onSelectCustomNetwork}
            selectedNetwork={customAssetsNetwork}
            showAllNetworksOption={false}
            label={
              <InputLabel>{getLocale('wootzWalletSelectNetwork')}</InputLabel>
            }
          />
        </FullWidthFormColumn>

        <FullWidthFormColumn>
          <Input
            value={tokenContractAddress}
            onInput={handleTokenAddressChanged}
            placeholder={getLocale('wootzWalletExempliGratia').replace(
              '$1',
              '0xbd3531da5cf5857e7cfaa92426877b022e612cf8'
            )}
          >
            <Row
              gap='4px'
              justifyContent='flex-start'
            >
              <InputLabel>
                {customAssetsNetwork?.coin === WootzWallet.CoinType.SOL
                  ? getLocale('wootzWalletTokenMintAddress')
                  : getLocale('wootzWalletNFTDetailContractAddress')}
              </InputLabel>
              <InfoIconTooltip
                placement='bottom'
                text={getLocale('wootzWalletWhatIsAnNftContractAddress')}
              />
            </Row>
          </Input>
        </FullWidthFormColumn>

        {customAssetsNetwork &&
          customAssetsNetwork?.coin !== WootzWallet.CoinType.SOL && (
            <FullWidthFormColumn>
              <Input
                value={
                  customTokenID
                    ? new Amount(customTokenID).format(undefined, false)
                    : ''
                }
                onInput={handleTokenIDChanged}
                type='number'
                placeholder={getLocale('wootzWalletExempliGratia').replace(
                  '$1',
                  '1234'
                )}
              >
                <Row
                  gap='4px'
                  justifyContent='flex-start'
                >
                  <InputLabel>
                    {getLocale('wootzWalletNFTDetailTokenID')}
                  </InputLabel>
                  <InfoIconTooltip
                    placement='bottom'
                    text={getLocale('wootzWalletWhatIsAnNftTokenId')}
                  />
                </Row>
              </Input>
            </FullWidthFormColumn>
          )}

        <FullWidthFormColumn>
          <Input
            value={customTokenName}
            onInput={handleTokenNameChanged}
            type='text'
            placeholder={getLocale('wootzWalletExempliGratia').replace(
              '$1',
              'Pudgy Penguin #1234'
            )}
          >
            <Row
              gap='4px'
              justifyContent='flex-start'
            >
              <InputLabel>
                {getLocale('wootzWalletWatchListTokenName')}
              </InputLabel>
              <InfoIconTooltip
                placement='bottom'
                text={getLocale('wootzWalletNftNameFieldExplanation')}
              />
            </Row>

            <div slot='left-icon'>
              {(isFetchingNftMetadata || isTokenInfoLoading) && (
                <InputLoadingIndicator />
              )}
            </div>
          </Input>
        </FullWidthFormColumn>

        <FullWidthFormColumn>
          <Input
            value={customTokenSymbol}
            onInput={handleTokenSymbolChanged}
            disabled={
              // prevent edits when data has been found on-chain
              !hasGetTokenInfoError && !!matchedTokenInfo?.symbol
            }
            type='text'
            placeholder={getLocale('wootzWalletExempliGratia').replace(
              '$1',
              'PPG'
            )}
          >
            <Row
              gap='4px'
              justifyContent='flex-start'
            >
              <InputLabel>
                {getLocale('wootzWalletWatchListTokenSymbol')}
              </InputLabel>
              <InfoIconTooltip
                placement='bottom'
                text={getLocale('wootzWalletNftSymbolFieldExplanation')}
              />
            </Row>
            <div slot='left-icon'>
              {(isFetchingNftMetadata || isTokenInfoLoading) && (
                <InputLoadingIndicator />
              )}
            </div>
          </Input>
        </FullWidthFormColumn>

        {metadataLookupArg?.coin === WootzWallet.CoinType.ETH &&
          !metadataLookupArg?.tokenId && (
            <ErrorText>
              {getLocale('wootzWalletWatchListTokenIdError')}
            </ErrorText>
          )}

        {hasError ? (
          <ErrorText>{getLocale('wootzWalletWatchListError')}</ErrorText>
        ) : (
          <Column
            fullWidth
            alignItems='center'
            justifyContent='center'
          >
            {isFetchingNftMetadata ? (
              <Column
                fullWidth
                gap={'4px'}
              >
                <PreviewImageContainer>
                  <Skeleton
                    width={96}
                    height={96}
                    enableAnimation
                  />
                </PreviewImageContainer>

                <Skeleton
                  width={96}
                  height={22}
                  enableAnimation
                />
                <Skeleton
                  width={96}
                  height={22}
                  enableAnimation
                />
              </Column>
            ) : (
              <>
                {hasNftMetadataError ? (
                  <Column>
                    <ErrorText>
                      {getLocale('wootzWalletFetchNftMetadataError')}
                    </ErrorText>
                  </Column>
                ) : (
                  <>
                    {metadataAssetInfo && (
                      <Column
                        fullWidth
                        gap={'16px'}
                      >
                        <Row
                          alignItems='center'
                          gap='16px'
                        >
                          {selectedAsset && (
                            <>
                              <Column>
                                <PreviewImageContainer>
                                  <NftIconWithPlaceholder
                                    key={getAssetIdKey(selectedAsset)}
                                    asset={selectedAsset}
                                    responsive
                                  />
                                </PreviewImageContainer>

                                <TokenNamePreviewText>
                                  {selectedAsset.name}
                                </TokenNamePreviewText>
                                <TokenTickerPreviewText>
                                  {selectedAsset.symbol}
                                </TokenTickerPreviewText>
                              </Column>

                              <Column>
                                <Icon name={'carat-right'} />
                              </Column>
                            </>
                          )}

                          <Column>
                            <PreviewImageContainer>
                              <NftIconWithPlaceholder
                                key={getAssetIdKey(metadataAssetInfo)}
                                asset={metadataAssetInfo}
                                responsive
                              />
                            </PreviewImageContainer>

                            <TokenNamePreviewText>
                              {customTokenName}
                            </TokenNamePreviewText>
                            <TokenTickerPreviewText>
                              {metadataAssetInfo.symbol}
                            </TokenTickerPreviewText>
                          </Column>
                        </Row>

                        {!userOwnsNft && (
                          <Alert
                            mode='simple'
                            type='info'
                          >
                            {getLocale('wootzWalletUnownedNftAlert')}
                          </Alert>
                        )}
                      </Column>
                    )}
                  </>
                )}
              </>
            )}
          </Column>
        )}

        <ButtonRowSpacer />
      </FormWrapper>

      <ButtonRow gap='16px'>
        <LeoSquaredButton
          onClick={onClickCancel}
          kind='plain-faint'
        >
          {getLocale('wootzWalletButtonCancel')}
        </LeoSquaredButton>

        <Tooltip
          text={<FormErrorsList errors={formErrors} />}
          isVisible={buttonDisabled}
          maxWidth={120}
          verticalPosition='above'
        >
          <Row>
            <LeoSquaredButton
              onClick={addOrUpdateToken}
              isDisabled={buttonDisabled}
              isLoading={
                isTokenInfoLoading ||
                isFetchingBalanceCheck ||
                isFetchingNftMetadata
              }
            >
              {!userOwnsNft
                ? getLocale('wootzWalletWatchThisNft')
                : selectedAsset
                ? getLocale('wootzWalletButtonSaveChanges')
                : getLocale('wootzWalletWatchListAdd')}
            </LeoSquaredButton>
          </Row>
        </Tooltip>
      </ButtonRow>
    </>
  )
}
