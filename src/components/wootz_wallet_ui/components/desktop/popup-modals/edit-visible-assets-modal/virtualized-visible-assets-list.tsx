// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.
import * as React from 'react'
import { VariableSizeList as List } from 'react-window'
import { useLocation } from 'react-router-dom'
import AutoSizer from 'react-virtualized-auto-sizer'
import Button from '@brave/leo/react/button'

// utils
import { getLocale } from '../../../../../common/locale'
import { getAssetIdKey } from '../../../../utils/asset-utils'

// types
import { WootzWallet, WalletRoutes } from '../../../../constants/types'

// components
import AssetWatchlistItem from '../../asset-watchlist-item'

// styles
import {
  Row,
  VerticalDivider,
  VerticalSpace,
  Text
} from '../../../shared/style'
import { AddIcon, AddButtonText, VirtualListStyle } from './style'
import { PaddedColumn } from '../style'
import { assetWatchListItemHeight } from '../../asset-watchlist-item/style'

interface VirtualizedTokensListProps {
  tokenList: WootzWallet.BlockchainToken[]
  onRemoveAsset: (token: WootzWallet.BlockchainToken) => void
  isAssetSelected: (token: WootzWallet.BlockchainToken) => boolean
  onClickAddCustomAsset: () => void
  onCheckWatchlistItem: (token: WootzWallet.BlockchainToken) => void
}

interface ListItemProps extends Omit<VirtualizedTokensListProps, 'tokenList'> {
  index: number
  data: WootzWallet.BlockchainToken
  style: React.CSSProperties
  setSize: (index: number, size: number) => void
  isLastIndex: boolean
}

const getListItemKey = (
  index: number,
  tokenList: WootzWallet.BlockchainToken[]
) => {
  return getAssetIdKey(tokenList[index])
}

const checkIsLastIndex = (
  index: number,
  tokenList: WootzWallet.BlockchainToken[]
) => {
  return index === tokenList.length - 1
}

const isRemovable = (token: WootzWallet.BlockchainToken): boolean => {
  return (
    // Native assets should not be removable.
    token.contractAddress !== '' &&
    // Any NFT should be removable.
    (token.isErc721 || token.isErc1155 || token.isNft)
  )
}

const ListItem = (props: ListItemProps) => {
  const {
    data,
    style,
    index,
    isLastIndex,
    isAssetSelected,
    onCheckWatchlistItem,
    onClickAddCustomAsset,
    onRemoveAsset,
    setSize
  } = props

  const handleSetSize = React.useCallback(
    (ref: HTMLDivElement | null) => {
      if (ref) {
        setSize(index, ref.getBoundingClientRect().height)
      }
    },
    [index, setSize]
  )

  return (
    <div style={style}>
      <AssetWatchlistItem
        ref={handleSetSize}
        isRemovable={isRemovable(data)}
        token={data}
        onRemoveAsset={onRemoveAsset}
        isSelected={isAssetSelected(data)}
        onSelectAsset={onCheckWatchlistItem}
      />
      {isLastIndex && (
        <PaddedColumn margin='8px 0px'>
          <VerticalDivider />
          <VerticalSpace space='14px' />
          <Text
            textSize='14px'
            textColor='text02'
          >
            {getLocale('wootzWalletDidntFindAssetEndOfList')}
          </Text>
          <VerticalSpace space='14px' />
          <Button
            onClick={onClickAddCustomAsset}
            kind='outline'
          >
            <Row>
              <AddIcon />
              <AddButtonText>
                {getLocale('wootzWalletWatchlistAddCustomAsset')}
              </AddButtonText>
            </Row>
          </Button>
        </PaddedColumn>
      )}
    </div>
  )
}

export const VirtualizedVisibleAssetsList = (
  props: VirtualizedTokensListProps
) => {
  const {
    tokenList,
    isAssetSelected,
    onCheckWatchlistItem,
    onClickAddCustomAsset,
    onRemoveAsset
  } = props
  // routing
  const { hash } = useLocation()

  const listRef = React.useRef<List | null>(null)
  const itemSizes = React.useRef<number[]>(
    new Array(tokenList.length).fill(assetWatchListItemHeight)
  )

  const setSize = React.useCallback((index: number, size: number) => {
    // Performance: Only update the sizeMap and reset cache if an actual value
    // changed
    if (itemSizes.current[index] !== size && size > -1) {
      itemSizes.current[index] = size
      if (listRef.current) {
        // Clear cached data and rerender
        listRef.current.resetAfterIndex(0)
      }
    }
  }, [])

  const getSize = React.useCallback((index: number) => {
    return itemSizes.current[index] || assetWatchListItemHeight
  }, [])

  return (
    <AutoSizer style={VirtualListStyle}>
      {function ({ width, height }: { width: number; height: number }) {
        return (
          <List
            ref={listRef}
            width={width}
            height={height}
            itemData={tokenList}
            itemCount={tokenList.length}
            itemSize={getSize}
            overscanCount={10}
            itemKey={getListItemKey}
            children={({ data, index, style }) => (
              <ListItem
                data={data[index]}
                onRemoveAsset={onRemoveAsset}
                isAssetSelected={isAssetSelected}
                onCheckWatchlistItem={onCheckWatchlistItem}
                onClickAddCustomAsset={onClickAddCustomAsset}
                setSize={setSize}
                isLastIndex={
                  hash !== WalletRoutes.AvailableAssetsHash &&
                  checkIsLastIndex(index, data)
                }
                index={index}
                style={style}
              />
            )}
          />
        )
      }}
    </AutoSizer>
  )
}
