// Copyright (c) 2024 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

// constants
import { WootzWallet } from '../../constants/types'

export const dappRadarChainNamesToChainIdMapping: Record<string, string> = {
  'aleph-zero': '', // NOT SUPPORTED
  'algorand': '', // NOT SUPPORTED
  'aptos': '', // NOT SUPPORTED
  'arbitrum': WootzWallet.ARBITRUM_MAINNET_CHAIN_ID,
  'astar': WootzWallet.ASTAR_CHAIN_ID,
  'astar-zkevm': WootzWallet.ASTAR_ZK_EVM_CHAIN_ID,
  'aurora': WootzWallet.AURORA_MAINNET_CHAIN_ID,
  'avalanche': WootzWallet.AVALANCHE_MAINNET_CHAIN_ID,
  'bahamut': WootzWallet.BAHAMUT_CHAIN_ID,
  'base': WootzWallet.BASE_MAINNET_CHAIN_ID,
  'blast': WootzWallet.BLAST_MAINNET_CHAIN_ID,
  'bnb-chain': WootzWallet.BNB_SMART_CHAIN_MAINNET_CHAIN_ID,
  'bttc': WootzWallet.BIT_TORRENT_CHAIN_MAINNET_CHAIN_ID,
  'cardano': '', // NOT SUPPORTED
  'celo': WootzWallet.CELO_MAINNET_CHAIN_ID,
  'chromia': '', // NOT SUPPORTED
  'core': WootzWallet.CORE_CHAIN_ID,
  'cronos': WootzWallet.CRONOS_MAINNET_CHAIN_ID,
  'cyber': WootzWallet.CYBER_MAINNET_CHAIN_ID,
  'defikingdoms': '', // NOT SUPPORTED
  'elysium': WootzWallet.ELYSIUM_MAINNET_CHAIN_ID,
  'eos': '', // NOT SUPPORTED
  'eosevm': WootzWallet.EOSEVM_NETWORK_CHAIN_ID,
  'ethereum': WootzWallet.MAINNET_CHAIN_ID,
  'fantom': WootzWallet.FANTOM_MAINNET_CHAIN_ID,
  'fio': '', // NOT SUPPORTED
  'flow': '', // NOT SUPPORTED
  'hedera': WootzWallet.HEDERA_MAINNET_CHAIN_ID,
  'hive': '', // NOT SUPPORTED
  'icp': '', // NOT SUPPORTED
  'immutablex': '', // NOT SUPPORTED
  'immutablezkevm': WootzWallet.IMMUTABLE_ZK_EVM_CHAIN_ID,
  'klaytn': WootzWallet.KLAYTN_MAINNET_CYPRESS_CHAIN_ID,
  'kroma': WootzWallet.KROMA_CHAIN_ID,
  'lightlink': WootzWallet.LIGHTLINK_PHOENIX_MAINNET_CHAIN_ID,
  'linea': WootzWallet.LINEA_CHAIN_ID,
  'mooi': '', // NOT SUPPORTED
  'moonbeam': WootzWallet.MOONBEAM_CHAIN_ID,
  'moonriver': WootzWallet.MOONRIVER_CHAIN_ID,
  'near': WootzWallet.NEAR_MAINNET_CHAIN_ID,
  'oasis': WootzWallet.OASIS_EMERALD_CHAIN_ID,
  'oasis-sapphire': WootzWallet.OASIS_SAPPHIRE_CHAIN_ID,
  'oasys-chain-verse': '', // NOT SUPPORTED
  'oasys-defi-verse': '', // NOT SUPPORTED
  'oasys-gesoten-verse': '', // NOT SUPPORTED
  'oasys-home-verse': '', // NOT SUPPORTED
  'oasys-mainnet': WootzWallet.OASYS_MAINNET_CHAIN_ID,
  'oasys-mch-verse': '', // NOT SUPPORTED
  'oasys-saakuru-verse': '', // NOT SUPPORTED
  'oasys-tcg-verse': '', // NOT SUPPORTED
  'oasys-yooldo-verse': '', // NOT SUPPORTED
  'ontology': WootzWallet.ONTOLOGY_MAINNET_CHAIN_ID,
  'opbnb': WootzWallet.OP_BNB_MAINNET_CHAIN_ID,
  'optimism': WootzWallet.OPTIMISM_MAINNET_CHAIN_ID,
  'other': '', // NOT SUPPORTED
  'polygon': WootzWallet.POLYGON_MAINNET_CHAIN_ID,
  'rangers': WootzWallet.RANGERS_PROTOCOL_MAINNET_CHAIN_ID,
  'ronin': WootzWallet.RONIN_CHAIN_ID,
  'shiden': WootzWallet.SHIDEN_CHAIN_ID,
  'skale-calypso': WootzWallet.SKALE_CALYPSO_HUB_CHAIN_ID,
  'skale-europa': WootzWallet.SKALE_EUROPA_HUB_CHAIN_ID,
  'skale-exorde': '', // NOT SUPPORTED
  'skale-nebula': WootzWallet.SKALE_NEBULA_HUB_CHAIN_ID,
  'skale-omnus': '', // NOT SUPPORTED
  'skale-razor': '', // NOT SUPPORTED
  'skale-strayshot': '', // NOT SUPPORTED
  'skale-titan': WootzWallet.SKALE_TITAN_HUB_CHAIN_ID,
  'solana': WootzWallet.SOLANA_MAINNET,
  'stacks': '', // NOT SUPPORTED
  'stargaze': '', // NOT SUPPORTED
  'steem': '', // NOT SUPPORTED
  'telos': '', // NOT SUPPORTED
  'telosevm': WootzWallet.TELOS_EVM_MAINNET_CHAIN_ID,
  'tezos': '', // NOT SUPPORTED
  'theta': WootzWallet.THETA_MAINNET_CHAIN_ID,
  'thundercore': WootzWallet.THUNDER_CORE_MAINNET_CHAIN_ID,
  'ton': '', // NOT SUPPORTED
  'tron': '', // NOT SUPPORTED
  'wax': '', // NOT SUPPORTED
  'wemix': WootzWallet.WEMIX_MAINNET_CHAIN_ID,
  'xai': WootzWallet.XAI_MAINNET_CHAIN_ID,
  'xrpl': '', // NOT SUPPORTED
  'zetachain': WootzWallet.ZETA_CHAIN_MAINNET_CHAIN_ID,
  'zilliqa': WootzWallet.ZILLIQA_EVM_CHAIN_ID,
  'zksync-era': WootzWallet.ZK_SYNC_ERA_CHAIN_ID
}
