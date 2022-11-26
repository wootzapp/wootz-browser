import 'dart:collection';
import 'dart:convert';
import 'dart:ffi';
import 'package:cryptowallet/google_drive/drive.dart';
import 'package:cryptowallet/utils/json_viewer.dart';
import 'package:get/get.dart' hide Response;
import 'dart:io';
import 'dart:math';
import 'package:awesome_dialog/awesome_dialog.dart';
import 'package:cardano_wallet_sdk/cardano_wallet_sdk.dart' as cardano;
import 'package:cryptowallet/main.dart';
import 'package:cryptowallet/screens/security.dart';
import 'package:cryptowallet/utils/bitcoin_util.dart';
import 'package:device_info_plus/device_info_plus.dart';
import 'package:eth_sig_util/util/utils.dart';
import 'package:flutter_js/flutter_js.dart';
import 'package:flutter_windowmanager/flutter_windowmanager.dart';
import 'package:hive/hive.dart';
import 'package:html/parser.dart' as html;
import 'package:bs58check/bs58check.dart' as bs58check;
import 'package:ntcdcrypto/ntcdcrypto.dart';
import 'package:stellar_flutter_sdk/stellar_flutter_sdk.dart' as stellar
    hide Row;
import 'package:cached_network_image/cached_network_image.dart';
import 'package:cryptowallet/utils/slide_up_panel.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:http/http.dart';
import 'package:image_picker/image_picker.dart';
import 'package:mime/mime.dart';
import 'package:validators/validators.dart';
import 'package:web3dart/web3dart.dart' as web3;
import 'package:web3dart/web3dart.dart';
import 'package:path/path.dart';
import 'package:http_parser/http_parser.dart';
import 'package:bip39/bip39.dart' as bip39;
import 'package:local_auth/local_auth.dart';
import 'package:bitcoin_flutter/bitcoin_flutter.dart';
import 'package:bitcoin_flutter/bitcoin_flutter.dart' as bitcoin;
import 'package:bip32/bip32.dart' as bip32;
import 'package:http/http.dart' as http;
import 'package:solana/solana.dart' as solana;
import 'package:hive_flutter/hive_flutter.dart';

import 'package:hex/hex.dart';

import '../components/loader.dart';
import '../eip/eip681.dart';
import '../google_drive/file.dart';
import '../screens/build_row.dart';
import '../screens/dapp.dart';
import 'alt_ens.dart';
import 'app_config.dart';
import 'pos_networks.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';

// crypto decimals
const etherDecimals = 18;
const bitCoinDecimals = 8;
const cardanoDecimals = 6;
const fileCoinDecimals = 18;
const solanaDecimals = 9;
const satoshiDustAmount = 546;
const stellarDecimals = 6;
const int maxFeeGuessForCardano = 200000;

// time
const Duration networkTimeOutDuration = Duration(seconds: 15);
const Duration httpPollingDelay = Duration(seconds: 15);

// useful ether addresses
const zeroAddress = '0x0000000000000000000000000000000000000000';
const deadAddress = '0x000000000000000000000000000000000000dEaD';
const nativeTokenAddress = '0xEeeeeEeeeEeEeeEeEeEeeEEEeeeeEeeeeeeeEEeE';
const ensInterfaceAddress = '0x00000000000C2E074eC69A0dFb2997BA6C7d2e1e';

// third party urls
const coinGeckoSupportedCurrencies =
    'https://api.coingecko.com/api/v3/simple/supported_vs_currencies';
const scamDbUrl = 'https://api.cryptoscamdb.org/v1/check/';
// abi's

const uniswapAbi2 =
    '''[{"inputs":[{"internalType":"address","name":"_factoryV2","type":"address"},{"internalType":"address","name":"factoryV3","type":"address"},{"internalType":"address","name":"_positionManager","type":"address"},{"internalType":"address","name":"_WETH9","type":"address"}],"stateMutability":"nonpayable","type":"constructor"},{"inputs":[],"name":"WETH9","outputs":[{"internalType":"address","name":"","type":"address"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"address","name":"token","type":"address"}],"name":"approveMax","outputs":[],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"address","name":"token","type":"address"}],"name":"approveMaxMinusOne","outputs":[],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"address","name":"token","type":"address"}],"name":"approveZeroThenMax","outputs":[],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"address","name":"token","type":"address"}],"name":"approveZeroThenMaxMinusOne","outputs":[],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"bytes","name":"data","type":"bytes"}],"name":"callPositionManager","outputs":[{"internalType":"bytes","name":"result","type":"bytes"}],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"bytes[]","name":"paths","type":"bytes[]"},{"internalType":"uint128[]","name":"amounts","type":"uint128[]"},{"internalType":"uint24","name":"maximumTickDivergence","type":"uint24"},{"internalType":"uint32","name":"secondsAgo","type":"uint32"}],"name":"checkOracleSlippage","outputs":[],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"bytes","name":"path","type":"bytes"},{"internalType":"uint24","name":"maximumTickDivergence","type":"uint24"},{"internalType":"uint32","name":"secondsAgo","type":"uint32"}],"name":"checkOracleSlippage","outputs":[],"stateMutability":"view","type":"function"},{"inputs":[{"components":[{"internalType":"bytes","name":"path","type":"bytes"},{"internalType":"address","name":"recipient","type":"address"},{"internalType":"uint256","name":"amountIn","type":"uint256"},{"internalType":"uint256","name":"amountOutMinimum","type":"uint256"}],"internalType":"struct IV3SwapRouter.ExactInputParams","name":"params","type":"tuple"}],"name":"exactInput","outputs":[{"internalType":"uint256","name":"amountOut","type":"uint256"}],"stateMutability":"payable","type":"function"},{"inputs":[{"components":[{"internalType":"address","name":"tokenIn","type":"address"},{"internalType":"address","name":"tokenOut","type":"address"},{"internalType":"uint24","name":"fee","type":"uint24"},{"internalType":"address","name":"recipient","type":"address"},{"internalType":"uint256","name":"amountIn","type":"uint256"},{"internalType":"uint256","name":"amountOutMinimum","type":"uint256"},{"internalType":"uint160","name":"sqrtPriceLimitX96","type":"uint160"}],"internalType":"struct IV3SwapRouter.ExactInputSingleParams","name":"params","type":"tuple"}],"name":"exactInputSingle","outputs":[{"internalType":"uint256","name":"amountOut","type":"uint256"}],"stateMutability":"payable","type":"function"},{"inputs":[{"components":[{"internalType":"bytes","name":"path","type":"bytes"},{"internalType":"address","name":"recipient","type":"address"},{"internalType":"uint256","name":"amountOut","type":"uint256"},{"internalType":"uint256","name":"amountInMaximum","type":"uint256"}],"internalType":"struct IV3SwapRouter.ExactOutputParams","name":"params","type":"tuple"}],"name":"exactOutput","outputs":[{"internalType":"uint256","name":"amountIn","type":"uint256"}],"stateMutability":"payable","type":"function"},{"inputs":[{"components":[{"internalType":"address","name":"tokenIn","type":"address"},{"internalType":"address","name":"tokenOut","type":"address"},{"internalType":"uint24","name":"fee","type":"uint24"},{"internalType":"address","name":"recipient","type":"address"},{"internalType":"uint256","name":"amountOut","type":"uint256"},{"internalType":"uint256","name":"amountInMaximum","type":"uint256"},{"internalType":"uint160","name":"sqrtPriceLimitX96","type":"uint160"}],"internalType":"struct IV3SwapRouter.ExactOutputSingleParams","name":"params","type":"tuple"}],"name":"exactOutputSingle","outputs":[{"internalType":"uint256","name":"amountIn","type":"uint256"}],"stateMutability":"payable","type":"function"},{"inputs":[],"name":"factory","outputs":[{"internalType":"address","name":"","type":"address"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"factoryV2","outputs":[{"internalType":"address","name":"","type":"address"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"address","name":"token","type":"address"},{"internalType":"uint256","name":"amount","type":"uint256"}],"name":"getApprovalType","outputs":[{"internalType":"enum IApproveAndCall.ApprovalType","name":"","type":"uint8"}],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"components":[{"internalType":"address","name":"token0","type":"address"},{"internalType":"address","name":"token1","type":"address"},{"internalType":"uint256","name":"tokenId","type":"uint256"},{"internalType":"uint256","name":"amount0Min","type":"uint256"},{"internalType":"uint256","name":"amount1Min","type":"uint256"}],"internalType":"struct IApproveAndCall.IncreaseLiquidityParams","name":"params","type":"tuple"}],"name":"increaseLiquidity","outputs":[{"internalType":"bytes","name":"result","type":"bytes"}],"stateMutability":"payable","type":"function"},{"inputs":[{"components":[{"internalType":"address","name":"token0","type":"address"},{"internalType":"address","name":"token1","type":"address"},{"internalType":"uint24","name":"fee","type":"uint24"},{"internalType":"int24","name":"tickLower","type":"int24"},{"internalType":"int24","name":"tickUpper","type":"int24"},{"internalType":"uint256","name":"amount0Min","type":"uint256"},{"internalType":"uint256","name":"amount1Min","type":"uint256"},{"internalType":"address","name":"recipient","type":"address"}],"internalType":"struct IApproveAndCall.MintParams","name":"params","type":"tuple"}],"name":"mint","outputs":[{"internalType":"bytes","name":"result","type":"bytes"}],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"bytes32","name":"previousBlockhash","type":"bytes32"},{"internalType":"bytes[]","name":"data","type":"bytes[]"}],"name":"multicall","outputs":[{"internalType":"bytes[]","name":"","type":"bytes[]"}],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"uint256","name":"deadline","type":"uint256"},{"internalType":"bytes[]","name":"data","type":"bytes[]"}],"name":"multicall","outputs":[{"internalType":"bytes[]","name":"","type":"bytes[]"}],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"bytes[]","name":"data","type":"bytes[]"}],"name":"multicall","outputs":[{"internalType":"bytes[]","name":"results","type":"bytes[]"}],"stateMutability":"payable","type":"function"},{"inputs":[],"name":"positionManager","outputs":[{"internalType":"address","name":"","type":"address"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"address","name":"token","type":"address"},{"internalType":"uint256","name":"value","type":"uint256"}],"name":"pull","outputs":[],"stateMutability":"payable","type":"function"},{"inputs":[],"name":"refundETH","outputs":[],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"address","name":"token","type":"address"},{"internalType":"uint256","name":"value","type":"uint256"},{"internalType":"uint256","name":"deadline","type":"uint256"},{"internalType":"uint8","name":"v","type":"uint8"},{"internalType":"bytes32","name":"r","type":"bytes32"},{"internalType":"bytes32","name":"s","type":"bytes32"}],"name":"selfPermit","outputs":[],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"address","name":"token","type":"address"},{"internalType":"uint256","name":"nonce","type":"uint256"},{"internalType":"uint256","name":"expiry","type":"uint256"},{"internalType":"uint8","name":"v","type":"uint8"},{"internalType":"bytes32","name":"r","type":"bytes32"},{"internalType":"bytes32","name":"s","type":"bytes32"}],"name":"selfPermitAllowed","outputs":[],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"address","name":"token","type":"address"},{"internalType":"uint256","name":"nonce","type":"uint256"},{"internalType":"uint256","name":"expiry","type":"uint256"},{"internalType":"uint8","name":"v","type":"uint8"},{"internalType":"bytes32","name":"r","type":"bytes32"},{"internalType":"bytes32","name":"s","type":"bytes32"}],"name":"selfPermitAllowedIfNecessary","outputs":[],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"address","name":"token","type":"address"},{"internalType":"uint256","name":"value","type":"uint256"},{"internalType":"uint256","name":"deadline","type":"uint256"},{"internalType":"uint8","name":"v","type":"uint8"},{"internalType":"bytes32","name":"r","type":"bytes32"},{"internalType":"bytes32","name":"s","type":"bytes32"}],"name":"selfPermitIfNecessary","outputs":[],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"uint256","name":"amountIn","type":"uint256"},{"internalType":"uint256","name":"amountOutMin","type":"uint256"},{"internalType":"address[]","name":"path","type":"address[]"},{"internalType":"address","name":"to","type":"address"}],"name":"swapExactTokensForTokens","outputs":[{"internalType":"uint256","name":"amountOut","type":"uint256"}],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"uint256","name":"amountOut","type":"uint256"},{"internalType":"uint256","name":"amountInMax","type":"uint256"},{"internalType":"address[]","name":"path","type":"address[]"},{"internalType":"address","name":"to","type":"address"}],"name":"swapTokensForExactTokens","outputs":[{"internalType":"uint256","name":"amountIn","type":"uint256"}],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"address","name":"token","type":"address"},{"internalType":"uint256","name":"amountMinimum","type":"uint256"},{"internalType":"address","name":"recipient","type":"address"}],"name":"sweepToken","outputs":[],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"address","name":"token","type":"address"},{"internalType":"uint256","name":"amountMinimum","type":"uint256"}],"name":"sweepToken","outputs":[],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"address","name":"token","type":"address"},{"internalType":"uint256","name":"amountMinimum","type":"uint256"},{"internalType":"uint256","name":"feeBips","type":"uint256"},{"internalType":"address","name":"feeRecipient","type":"address"}],"name":"sweepTokenWithFee","outputs":[],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"address","name":"token","type":"address"},{"internalType":"uint256","name":"amountMinimum","type":"uint256"},{"internalType":"address","name":"recipient","type":"address"},{"internalType":"uint256","name":"feeBips","type":"uint256"},{"internalType":"address","name":"feeRecipient","type":"address"}],"name":"sweepTokenWithFee","outputs":[],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"int256","name":"amount0Delta","type":"int256"},{"internalType":"int256","name":"amount1Delta","type":"int256"},{"internalType":"bytes","name":"_data","type":"bytes"}],"name":"uniswapV3SwapCallback","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"uint256","name":"amountMinimum","type":"uint256"},{"internalType":"address","name":"recipient","type":"address"}],"name":"unwrapWETH9","outputs":[],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"uint256","name":"amountMinimum","type":"uint256"}],"name":"unwrapWETH9","outputs":[],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"uint256","name":"amountMinimum","type":"uint256"},{"internalType":"address","name":"recipient","type":"address"},{"internalType":"uint256","name":"feeBips","type":"uint256"},{"internalType":"address","name":"feeRecipient","type":"address"}],"name":"unwrapWETH9WithFee","outputs":[],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"uint256","name":"amountMinimum","type":"uint256"},{"internalType":"uint256","name":"feeBips","type":"uint256"},{"internalType":"address","name":"feeRecipient","type":"address"}],"name":"unwrapWETH9WithFee","outputs":[],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"uint256","name":"value","type":"uint256"}],"name":"wrapETH","outputs":[],"stateMutability":"payable","type":"function"},{"stateMutability":"payable","type":"receive"}]''';
const wrappedEthAbi =
    '''[{"constant":true,"inputs":[],"name":"name","outputs":[{"name":"","type":"string"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":false,"inputs":[{"name":"guy","type":"address"},{"name":"wad","type":"uint256"}],"name":"approve","outputs":[{"name":"","type":"bool"}],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":true,"inputs":[],"name":"totalSupply","outputs":[{"name":"","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":false,"inputs":[{"name":"src","type":"address"},{"name":"dst","type":"address"},{"name":"wad","type":"uint256"}],"name":"transferFrom","outputs":[{"name":"","type":"bool"}],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":false,"inputs":[{"name":"wad","type":"uint256"}],"name":"withdraw","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":true,"inputs":[],"name":"decimals","outputs":[{"name":"","type":"uint8"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[{"name":"","type":"address"}],"name":"balanceOf","outputs":[{"name":"","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[],"name":"symbol","outputs":[{"name":"","type":"string"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":false,"inputs":[{"name":"dst","type":"address"},{"name":"wad","type":"uint256"}],"name":"transfer","outputs":[{"name":"","type":"bool"}],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":false,"inputs":[],"name":"deposit","outputs":[],"payable":true,"stateMutability":"payable","type":"function"},{"constant":true,"inputs":[{"name":"","type":"address"},{"name":"","type":"address"}],"name":"allowance","outputs":[{"name":"","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"payable":true,"stateMutability":"payable","type":"fallback"},{"anonymous":false,"inputs":[{"indexed":true,"name":"src","type":"address"},{"indexed":true,"name":"guy","type":"address"},{"indexed":false,"name":"wad","type":"uint256"}],"name":"Approval","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"name":"src","type":"address"},{"indexed":true,"name":"dst","type":"address"},{"indexed":false,"name":"wad","type":"uint256"}],"name":"Transfer","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"name":"dst","type":"address"},{"indexed":false,"name":"wad","type":"uint256"}],"name":"Deposit","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"name":"src","type":"address"},{"indexed":false,"name":"wad","type":"uint256"}],"name":"Withdrawal","type":"event"}]''';
const oneInchAbi =
    '''[{"anonymous":false,"inputs":[{"indexed":false,"internalType":"string","name":"reason","type":"string"}],"name":"Error","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"internalType":"address","name":"previousOwner","type":"address"},{"indexed":true,"internalType":"address","name":"newOwner","type":"address"}],"name":"OwnershipTransferred","type":"event"},{"anonymous":false,"inputs":[{"indexed":false,"internalType":"address","name":"sender","type":"address"},{"indexed":false,"internalType":"contract IERC20","name":"srcToken","type":"address"},{"indexed":false,"internalType":"contract IERC20","name":"dstToken","type":"address"},{"indexed":false,"internalType":"address","name":"dstReceiver","type":"address"},{"indexed":false,"internalType":"uint256","name":"spentAmount","type":"uint256"},{"indexed":false,"internalType":"uint256","name":"returnAmount","type":"uint256"}],"name":"Swapped","type":"event"},{"inputs":[],"name":"destroy","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"contract IAggregationExecutor","name":"caller","type":"address"},{"components":[{"internalType":"contract IERC20","name":"srcToken","type":"address"},{"internalType":"contract IERC20","name":"dstToken","type":"address"},{"internalType":"address","name":"srcReceiver","type":"address"},{"internalType":"address","name":"dstReceiver","type":"address"},{"internalType":"uint256","name":"amount","type":"uint256"},{"internalType":"uint256","name":"minReturnAmount","type":"uint256"},{"internalType":"uint256","name":"flags","type":"uint256"},{"internalType":"bytes","name":"permit","type":"bytes"}],"internalType":"struct AggregationRouterV3.SwapDescription","name":"desc","type":"tuple"},{"internalType":"bytes","name":"data","type":"bytes"}],"name":"discountedSwap","outputs":[{"internalType":"uint256","name":"returnAmount","type":"uint256"},{"internalType":"uint256","name":"gasLeft","type":"uint256"},{"internalType":"uint256","name":"chiSpent","type":"uint256"}],"stateMutability":"payable","type":"function"},{"inputs":[],"name":"owner","outputs":[{"internalType":"address","name":"","type":"address"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"renounceOwnership","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"contract IERC20","name":"token","type":"address"},{"internalType":"uint256","name":"amount","type":"uint256"}],"name":"rescueFunds","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"contract IAggregationExecutor","name":"caller","type":"address"},{"components":[{"internalType":"contract IERC20","name":"srcToken","type":"address"},{"internalType":"contract IERC20","name":"dstToken","type":"address"},{"internalType":"address","name":"srcReceiver","type":"address"},{"internalType":"address","name":"dstReceiver","type":"address"},{"internalType":"uint256","name":"amount","type":"uint256"},{"internalType":"uint256","name":"minReturnAmount","type":"uint256"},{"internalType":"uint256","name":"flags","type":"uint256"},{"internalType":"bytes","name":"permit","type":"bytes"}],"internalType":"struct AggregationRouterV3.SwapDescription","name":"desc","type":"tuple"},{"internalType":"bytes","name":"data","type":"bytes"}],"name":"swap","outputs":[{"internalType":"uint256","name":"returnAmount","type":"uint256"},{"internalType":"uint256","name":"gasLeft","type":"uint256"}],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"address","name":"newOwner","type":"address"}],"name":"transferOwnership","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"contract IERC20","name":"srcToken","type":"address"},{"internalType":"uint256","name":"amount","type":"uint256"},{"internalType":"uint256","name":"minReturn","type":"uint256"},{"internalType":"bytes32[]","name":"","type":"bytes32[]"}],"name":"unoswap","outputs":[{"internalType":"uint256","name":"returnAmount","type":"uint256"}],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"contract IERC20","name":"srcToken","type":"address"},{"internalType":"uint256","name":"amount","type":"uint256"},{"internalType":"uint256","name":"minReturn","type":"uint256"},{"internalType":"bytes32[]","name":"pools","type":"bytes32[]"},{"internalType":"bytes","name":"permit","type":"bytes"}],"name":"unoswapWithPermit","outputs":[{"internalType":"uint256","name":"returnAmount","type":"uint256"}],"stateMutability":"payable","type":"function"},{"stateMutability":"payable","type":"receive"}]''';
const erc1155Abi =
    '''[{"inputs":[{"internalType":"string","name":"uri_","type":"string"}],"stateMutability":"nonpayable","type":"constructor"},{"anonymous":false,"inputs":[{"indexed":true,"internalType":"address","name":"account","type":"address"},{"indexed":true,"internalType":"address","name":"operator","type":"address"},{"indexed":false,"internalType":"bool","name":"approved","type":"bool"}],"name":"ApprovalForAll","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"internalType":"address","name":"operator","type":"address"},{"indexed":true,"internalType":"address","name":"from","type":"address"},{"indexed":true,"internalType":"address","name":"to","type":"address"},{"indexed":false,"internalType":"uint256[]","name":"ids","type":"uint256[]"},{"indexed":false,"internalType":"uint256[]","name":"values","type":"uint256[]"}],"name":"TransferBatch","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"internalType":"address","name":"operator","type":"address"},{"indexed":true,"internalType":"address","name":"from","type":"address"},{"indexed":true,"internalType":"address","name":"to","type":"address"},{"indexed":false,"internalType":"uint256","name":"id","type":"uint256"},{"indexed":false,"internalType":"uint256","name":"value","type":"uint256"}],"name":"TransferSingle","type":"event"},{"anonymous":false,"inputs":[{"indexed":false,"internalType":"string","name":"value","type":"string"},{"indexed":true,"internalType":"uint256","name":"id","type":"uint256"}],"name":"URI","type":"event"},{"inputs":[{"internalType":"address","name":"account","type":"address"},{"internalType":"uint256","name":"id","type":"uint256"}],"name":"balanceOf","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"address[]","name":"accounts","type":"address[]"},{"internalType":"uint256[]","name":"ids","type":"uint256[]"}],"name":"balanceOfBatch","outputs":[{"internalType":"uint256[]","name":"","type":"uint256[]"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"address","name":"account","type":"address"},{"internalType":"address","name":"operator","type":"address"}],"name":"isApprovedForAll","outputs":[{"internalType":"bool","name":"","type":"bool"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"address","name":"from","type":"address"},{"internalType":"address","name":"to","type":"address"},{"internalType":"uint256[]","name":"ids","type":"uint256[]"},{"internalType":"uint256[]","name":"amounts","type":"uint256[]"},{"internalType":"bytes","name":"data","type":"bytes"}],"name":"safeBatchTransferFrom","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"address","name":"from","type":"address"},{"internalType":"address","name":"to","type":"address"},{"internalType":"uint256","name":"id","type":"uint256"},{"internalType":"uint256","name":"amount","type":"uint256"},{"internalType":"bytes","name":"data","type":"bytes"}],"name":"safeTransferFrom","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"address","name":"operator","type":"address"},{"internalType":"bool","name":"approved","type":"bool"}],"name":"setApprovalForAll","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"bytes4","name":"interfaceId","type":"bytes4"}],"name":"supportsInterface","outputs":[{"internalType":"bool","name":"","type":"bool"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"uint256","name":"","type":"uint256"}],"name":"uri","outputs":[{"internalType":"string","name":"","type":"string"}],"stateMutability":"view","type":"function"}]''';
const erc721Abi =
    '''[{"inputs":[],"stateMutability":"nonpayable","type":"constructor"},{"anonymous":false,"inputs":[{"indexed":true,"internalType":"address","name":"owner","type":"address"},{"indexed":true,"internalType":"address","name":"approved","type":"address"},{"indexed":true,"internalType":"uint256","name":"tokenId","type":"uint256"}],"name":"Approval","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"internalType":"address","name":"owner","type":"address"},{"indexed":true,"internalType":"address","name":"operator","type":"address"},{"indexed":false,"internalType":"bool","name":"approved","type":"bool"}],"name":"ApprovalForAll","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"internalType":"address","name":"from","type":"address"},{"indexed":true,"internalType":"address","name":"to","type":"address"},{"indexed":true,"internalType":"uint256","name":"tokenId","type":"uint256"}],"name":"Transfer","type":"event"},{"inputs":[],"name":"TOKEN_LIMIT","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"admin","outputs":[{"internalType":"address payable","name":"","type":"address"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"uint256","name":"tokenId","type":"uint256"},{"internalType":"uint256","name":"priceInWei","type":"uint256"}],"name":"allowBuy","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"address","name":"to","type":"address"},{"internalType":"uint256","name":"tokenId","type":"uint256"}],"name":"approve","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"address","name":"owner","type":"address"}],"name":"balanceOf","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"uint256","name":"tokenId","type":"uint256"}],"name":"burn","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"uint256","name":"_tokenId","type":"uint256"}],"name":"buy","outputs":[],"stateMutability":"payable","type":"function"},{"inputs":[],"name":"createTokenId","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"uint256","name":"_tokenId","type":"uint256"}],"name":"disallowBuy","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"uint256","name":"tokenId","type":"uint256"}],"name":"getApproved","outputs":[{"internalType":"address","name":"","type":"address"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"getMintingPrice","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"address","name":"owner","type":"address"},{"internalType":"address","name":"operator","type":"address"}],"name":"isApprovedForAll","outputs":[{"internalType":"bool","name":"","type":"bool"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"uint256","name":"tokenId","type":"uint256"}],"name":"mint","outputs":[],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"uint256","name":"","type":"uint256"}],"name":"mintedNFTPrices","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"mintingPrice","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"name","outputs":[{"internalType":"string","name":"","type":"string"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"uint256","name":"tokenId","type":"uint256"}],"name":"ownerOf","outputs":[{"internalType":"address","name":"","type":"address"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"address","name":"from","type":"address"},{"internalType":"address","name":"to","type":"address"},{"internalType":"uint256","name":"tokenId","type":"uint256"}],"name":"safeTransferFrom","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"address","name":"from","type":"address"},{"internalType":"address","name":"to","type":"address"},{"internalType":"uint256","name":"tokenId","type":"uint256"},{"internalType":"bytes","name":"_data","type":"bytes"}],"name":"safeTransferFrom","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"address","name":"operator","type":"address"},{"internalType":"bool","name":"approved","type":"bool"}],"name":"setApprovalForAll","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"uint256","name":"_mintingPrice","type":"uint256"}],"name":"setMintingPrice","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"string","name":"baseURL","type":"string"}],"name":"setTokenBaseURL","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"bytes4","name":"interfaceId","type":"bytes4"}],"name":"supportsInterface","outputs":[{"internalType":"bool","name":"","type":"bool"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"symbol","outputs":[{"internalType":"string","name":"","type":"string"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"tokenBaseURL","outputs":[{"internalType":"string","name":"","type":"string"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"uint256","name":"index","type":"uint256"}],"name":"tokenByIndex","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"uint256","name":"","type":"uint256"}],"name":"tokenForSale","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"address","name":"owner","type":"address"},{"internalType":"uint256","name":"index","type":"uint256"}],"name":"tokenOfOwnerByIndex","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"uint256","name":"tokenId","type":"uint256"}],"name":"tokenURI","outputs":[{"internalType":"string","name":"","type":"string"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"totalItemForSale","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"totalSupply","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"trans","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"address","name":"from","type":"address"},{"internalType":"address","name":"to","type":"address"},{"internalType":"uint256","name":"tokenId","type":"uint256"}],"name":"transferFrom","outputs":[],"stateMutability":"nonpayable","type":"function"}]''';
const erc20Abi =
    '''[{"inputs":[{"internalType":"string","name":"name_","type":"string"},{"internalType":"string","name":"symbol_","type":"string"}],"stateMutability":"nonpayable","type":"constructor"},{"anonymous":false,"inputs":[{"indexed":true,"internalType":"address","name":"owner","type":"address"},{"indexed":true,"internalType":"address","name":"spender","type":"address"},{"indexed":false,"internalType":"uint256","name":"value","type":"uint256"}],"name":"Approval","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"internalType":"address","name":"from","type":"address"},{"indexed":true,"internalType":"address","name":"to","type":"address"},{"indexed":false,"internalType":"uint256","name":"value","type":"uint256"}],"name":"Transfer","type":"event"},{"inputs":[{"internalType":"address","name":"owner","type":"address"},{"internalType":"address","name":"spender","type":"address"}],"name":"allowance","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"address","name":"spender","type":"address"},{"internalType":"uint256","name":"amount","type":"uint256"}],"name":"approve","outputs":[{"internalType":"bool","name":"","type":"bool"}],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"address","name":"account","type":"address"}],"name":"balanceOf","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"decimals","outputs":[{"internalType":"uint8","name":"","type":"uint8"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"address","name":"spender","type":"address"},{"internalType":"uint256","name":"subtractedValue","type":"uint256"}],"name":"decreaseAllowance","outputs":[{"internalType":"bool","name":"","type":"bool"}],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"address","name":"spender","type":"address"},{"internalType":"uint256","name":"addedValue","type":"uint256"}],"name":"increaseAllowance","outputs":[{"internalType":"bool","name":"","type":"bool"}],"stateMutability":"nonpayable","type":"function"},{"inputs":[],"name":"name","outputs":[{"internalType":"string","name":"","type":"string"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"symbol","outputs":[{"internalType":"string","name":"","type":"string"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"totalSupply","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"address","name":"recipient","type":"address"},{"internalType":"uint256","name":"amount","type":"uint256"}],"name":"transfer","outputs":[{"internalType":"bool","name":"","type":"bool"}],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"address","name":"sender","type":"address"},{"internalType":"address","name":"recipient","type":"address"},{"internalType":"uint256","name":"amount","type":"uint256"}],"name":"transferFrom","outputs":[{"internalType":"bool","name":"","type":"bool"}],"stateMutability":"nonpayable","type":"function"}]''';
const tokenSaleAbi =
    '''[{"constant":false,"inputs":[{"name":"_isAirdropRunning","type":"bool"}],"name":"setAirdropActivation","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":false,"inputs":[{"name":"_refer","type":"address"}],"name":"getAirdrop","outputs":[{"name":"success","type":"bool"}],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":true,"inputs":[],"name":"name","outputs":[{"name":"","type":"string"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[],"name":"saleCap","outputs":[{"name":"","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":false,"inputs":[{"name":"spender","type":"address"},{"name":"tokens","type":"uint256"}],"name":"approve","outputs":[{"name":"success","type":"bool"}],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":true,"inputs":[],"name":"getBalance","outputs":[{"name":"","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[],"name":"totalSupply","outputs":[{"name":"","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[],"name":"saleTot","outputs":[{"name":"","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":false,"inputs":[{"name":"from","type":"address"},{"name":"to","type":"address"},{"name":"tokens","type":"uint256"}],"name":"transferFrom","outputs":[{"name":"success","type":"bool"}],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":true,"inputs":[],"name":"decimals","outputs":[{"name":"","type":"uint8"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[],"name":"privateSaletokensSold","outputs":[{"name":"","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":false,"inputs":[{"name":"_value","type":"uint256"}],"name":"burn","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":false,"inputs":[{"name":"_saleChunk","type":"uint256"},{"name":"_salePrice","type":"uint256"},{"name":"_saleCap","type":"uint256"},{"name":"_sDivisionInt","type":"uint256"}],"name":"startSale","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":true,"inputs":[],"name":"viewSale","outputs":[{"name":"SaleCap","type":"uint256"},{"name":"SaleCount","type":"uint256"},{"name":"ChunkSize","type":"uint256"},{"name":"SalePrice","type":"uint256"},{"name":"privateSaletokensSold","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":false,"inputs":[{"name":"_refer","type":"address"}],"name":"tokenSale","outputs":[{"name":"success","type":"bool"}],"payable":true,"stateMutability":"payable","type":"function"},{"constant":true,"inputs":[],"name":"sDivisionInt","outputs":[{"name":"","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[],"name":"saleChunk","outputs":[{"name":"","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[{"name":"tokenOwner","type":"address"}],"name":"balanceOf","outputs":[{"name":"balance","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":false,"inputs":[],"name":"tran","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":false,"inputs":[],"name":"acceptOwnership","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":false,"inputs":[{"name":"_airdropAmt","type":"uint256"},{"name":"_airdropCap","type":"uint256"},{"name":"_aDivisionInt","type":"uint256"}],"name":"startAirdrop","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":true,"inputs":[],"name":"isSaleRunning","outputs":[{"name":"","type":"bool"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[],"name":"airdropCap","outputs":[{"name":"","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[],"name":"owner","outputs":[{"name":"","type":"address"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[],"name":"symbol","outputs":[{"name":"","type":"string"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":false,"inputs":[{"name":"to","type":"address"},{"name":"tokens","type":"uint256"}],"name":"transfer","outputs":[{"name":"success","type":"bool"}],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":false,"inputs":[],"name":"txnToken","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":false,"inputs":[{"name":"spender","type":"address"},{"name":"tokens","type":"uint256"},{"name":"data","type":"bytes"}],"name":"approveAndCall","outputs":[{"name":"success","type":"bool"}],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":true,"inputs":[],"name":"airdropAmt","outputs":[{"name":"","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[],"name":"newOwner","outputs":[{"name":"","type":"address"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[],"name":"viewAirdrop","outputs":[{"name":"DropCap","type":"uint256"},{"name":"DropCount","type":"uint256"},{"name":"DropAmount","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[{"name":"tokenOwner","type":"address"},{"name":"spender","type":"address"}],"name":"allowance","outputs":[{"name":"remaining","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[],"name":"airdropTot","outputs":[{"name":"","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":false,"inputs":[{"name":"_isSaleRunning","type":"bool"}],"name":"setSaleActivation","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":true,"inputs":[],"name":"aDivisionInt","outputs":[{"name":"","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":false,"inputs":[{"name":"_newOwner","type":"address"}],"name":"transferOwnership","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":true,"inputs":[],"name":"salePrice","outputs":[{"name":"","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[],"name":"isAirdropRunning","outputs":[{"name":"","type":"bool"}],"payable":false,"stateMutability":"view","type":"function"},{"payable":true,"stateMutability":"payable","type":"fallback"},{"anonymous":false,"inputs":[{"indexed":true,"name":"_from","type":"address"},{"indexed":true,"name":"_to","type":"address"}],"name":"OwnershipTransferred","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"name":"from","type":"address"},{"indexed":true,"name":"to","type":"address"},{"indexed":false,"name":"tokens","type":"uint256"}],"name":"Transfer","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"name":"tokenOwner","type":"address"},{"indexed":true,"name":"spender","type":"address"},{"indexed":false,"name":"tokens","type":"uint256"}],"name":"Approval","type":"event"}]''';
const tokenStakingAbi =
    '''[{"inputs":[{"internalType":"address","name":"_rewardNftCollection","type":"address"},{"internalType":"contract IERC20","name":"_rewardToken","type":"address"}],"stateMutability":"nonpayable","type":"constructor"},{"anonymous":false,"inputs":[{"indexed":true,"internalType":"address","name":"previousOwner","type":"address"},{"indexed":true,"internalType":"address","name":"newOwner","type":"address"}],"name":"OwnershipTransferred","type":"event"},{"inputs":[{"internalType":"address","name":"_staker","type":"address"}],"name":"availableRewards","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"balance","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"claimRewards","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[],"name":"commission","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"address","name":"_staker","type":"address"}],"name":"getStakerDetails","outputs":[{"internalType":"uint256","name":"_amountStaked","type":"uint256"},{"internalType":"uint256","name":"_lastUpdate","type":"uint256"},{"internalType":"uint256","name":"_unclaimedRewards","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"getTotalRewardsClaimed","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"minimumStakeAmount","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"minimumStakePeriod","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"owner","outputs":[{"internalType":"address","name":"","type":"address"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"renounceOwnership","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[],"name":"rewardNftCollection","outputs":[{"internalType":"address","name":"","type":"address"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"rewardPool","outputs":[{"internalType":"address","name":"","type":"address"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"rewardToken","outputs":[{"internalType":"contract IERC20","name":"","type":"address"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"uint256","name":"_commission","type":"uint256"}],"name":"setCommission","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"uint256","name":"_minimumStakeAmount","type":"uint256"}],"name":"setMinimumStake","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"uint256","name":"_minimumStakePeriod","type":"uint256"}],"name":"setMinimumStakePeriod","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"address","name":"_rewardNftCollection","type":"address"}],"name":"setRewardNftCollection","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"uint256","name":"_rewardPerHour","type":"uint256"}],"name":"setRewardPerHour","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"address","name":"_rewardPool","type":"address"}],"name":"setRewardPool","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"contract IERC20","name":"_rewardToken","type":"address"}],"name":"setRewardToken","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"uint256","name":"amount","type":"uint256"}],"name":"stake","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"address","name":"","type":"address"}],"name":"stakers","outputs":[{"internalType":"uint256","name":"amountStaked","type":"uint256"},{"internalType":"uint256","name":"timeOfLastUpdate","type":"uint256"},{"internalType":"uint256","name":"unclaimedRewards","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"totalAmountStaked","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"totalRewardClaimed","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"address","name":"_newOwner","type":"address"}],"name":"tranferOwner","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"address","name":"newOwner","type":"address"}],"name":"transferOwnership","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[],"name":"unStake","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"uint256","name":"amount","type":"uint256"}],"name":"withdraw","outputs":[],"stateMutability":"nonpayable","type":"function"}]''';
const unstoppableDomainAbi =
    '''[{"inputs":[{"internalType":"contract IUNSRegistry","name":"unsRegistry","type":"address"},{"internalType":"contract ICNSRegistry","name":"cnsRegistry","type":"address"}],"stateMutability":"nonpayable","type":"constructor"},{"inputs":[],"name":"NAME","outputs":[{"internalType":"string","name":"","type":"string"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"VERSION","outputs":[{"internalType":"string","name":"","type":"string"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"address","name":"owner","type":"address"}],"name":"balanceOf","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"uint256","name":"tokenId","type":"uint256"},{"internalType":"string","name":"label","type":"string"}],"name":"childIdOf","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"uint256","name":"tokenId","type":"uint256"}],"name":"exists","outputs":[{"internalType":"bool","name":"","type":"bool"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"string","name":"key","type":"string"},{"internalType":"uint256","name":"tokenId","type":"uint256"}],"name":"get","outputs":[{"internalType":"string","name":"value","type":"string"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"uint256","name":"tokenId","type":"uint256"}],"name":"getApproved","outputs":[{"internalType":"address","name":"","type":"address"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"uint256","name":"keyHash","type":"uint256"},{"internalType":"uint256","name":"tokenId","type":"uint256"}],"name":"getByHash","outputs":[{"internalType":"string","name":"key","type":"string"},{"internalType":"string","name":"value","type":"string"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"string[]","name":"keys","type":"string[]"},{"internalType":"uint256","name":"tokenId","type":"uint256"}],"name":"getData","outputs":[{"internalType":"address","name":"resolver","type":"address"},{"internalType":"address","name":"owner","type":"address"},{"internalType":"string[]","name":"values","type":"string[]"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"uint256[]","name":"keyHashes","type":"uint256[]"},{"internalType":"uint256","name":"tokenId","type":"uint256"}],"name":"getDataByHash","outputs":[{"internalType":"address","name":"resolver","type":"address"},{"internalType":"address","name":"owner","type":"address"},{"internalType":"string[]","name":"keys","type":"string[]"},{"internalType":"string[]","name":"values","type":"string[]"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"uint256[]","name":"keyHashes","type":"uint256[]"},{"internalType":"uint256[]","name":"tokenIds","type":"uint256[]"}],"name":"getDataByHashForMany","outputs":[{"internalType":"address[]","name":"resolvers","type":"address[]"},{"internalType":"address[]","name":"owners","type":"address[]"},{"internalType":"string[][]","name":"keys","type":"string[][]"},{"internalType":"string[][]","name":"values","type":"string[][]"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"string[]","name":"keys","type":"string[]"},{"internalType":"uint256[]","name":"tokenIds","type":"uint256[]"}],"name":"getDataForMany","outputs":[{"internalType":"address[]","name":"resolvers","type":"address[]"},{"internalType":"address[]","name":"owners","type":"address[]"},{"internalType":"string[][]","name":"values","type":"string[][]"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"string[]","name":"keys","type":"string[]"},{"internalType":"uint256","name":"tokenId","type":"uint256"}],"name":"getMany","outputs":[{"internalType":"string[]","name":"values","type":"string[]"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"uint256[]","name":"keyHashes","type":"uint256[]"},{"internalType":"uint256","name":"tokenId","type":"uint256"}],"name":"getManyByHash","outputs":[{"internalType":"string[]","name":"keys","type":"string[]"},{"internalType":"string[]","name":"values","type":"string[]"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"address","name":"","type":"address"},{"internalType":"address","name":"","type":"address"}],"name":"isApprovedForAll","outputs":[{"internalType":"bool","name":"","type":"bool"}],"stateMutability":"pure","type":"function"},{"inputs":[{"internalType":"address","name":"spender","type":"address"},{"internalType":"uint256","name":"tokenId","type":"uint256"}],"name":"isApprovedOrOwner","outputs":[{"internalType":"bool","name":"","type":"bool"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"bytes[]","name":"data","type":"bytes[]"}],"name":"multicall","outputs":[{"internalType":"bytes[]","name":"results","type":"bytes[]"}],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"uint256","name":"tokenId","type":"uint256"}],"name":"ownerOf","outputs":[{"internalType":"address","name":"","type":"address"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"uint256[]","name":"tokenIds","type":"uint256[]"}],"name":"ownerOfForMany","outputs":[{"internalType":"address[]","name":"owners","type":"address[]"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"uint256","name":"tokenId","type":"uint256"}],"name":"registryOf","outputs":[{"internalType":"address","name":"","type":"address"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"uint256","name":"tokenId","type":"uint256"}],"name":"resolverOf","outputs":[{"internalType":"address","name":"","type":"address"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"address","name":"addr","type":"address"}],"name":"reverseOf","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"bytes4","name":"interfaceId","type":"bytes4"}],"name":"supportsInterface","outputs":[{"internalType":"bool","name":"","type":"bool"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"uint256","name":"tokenId","type":"uint256"}],"name":"tokenURI","outputs":[{"internalType":"string","name":"","type":"string"}],"stateMutability":"view","type":"function"}]''';
const ensResolver =
    '''[{"inputs":[{"internalType":"contract ENS","name":"_ens","type":"address"}],"payable":false,"stateMutability":"nonpayable","type":"constructor"},{"anonymous":false,"inputs":[{"indexed":true,"internalType":"bytes32","name":"node","type":"bytes32"},{"indexed":true,"internalType":"uint256","name":"contentType","type":"uint256"}],"name":"ABIChanged","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"internalType":"bytes32","name":"node","type":"bytes32"},{"indexed":false,"internalType":"address","name":"a","type":"address"}],"name":"AddrChanged","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"internalType":"bytes32","name":"node","type":"bytes32"},{"indexed":false,"internalType":"uint256","name":"coinType","type":"uint256"},{"indexed":false,"internalType":"bytes","name":"newAddress","type":"bytes"}],"name":"AddressChanged","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"internalType":"bytes32","name":"node","type":"bytes32"},{"indexed":true,"internalType":"address","name":"owner","type":"address"},{"indexed":true,"internalType":"address","name":"target","type":"address"},{"indexed":false,"internalType":"bool","name":"isAuthorised","type":"bool"}],"name":"AuthorisationChanged","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"internalType":"bytes32","name":"node","type":"bytes32"},{"indexed":false,"internalType":"bytes","name":"hash","type":"bytes"}],"name":"ContenthashChanged","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"internalType":"bytes32","name":"node","type":"bytes32"},{"indexed":false,"internalType":"bytes","name":"name","type":"bytes"},{"indexed":false,"internalType":"uint16","name":"resource","type":"uint16"},{"indexed":false,"internalType":"bytes","name":"record","type":"bytes"}],"name":"DNSRecordChanged","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"internalType":"bytes32","name":"node","type":"bytes32"},{"indexed":false,"internalType":"bytes","name":"name","type":"bytes"},{"indexed":false,"internalType":"uint16","name":"resource","type":"uint16"}],"name":"DNSRecordDeleted","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"internalType":"bytes32","name":"node","type":"bytes32"}],"name":"DNSZoneCleared","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"internalType":"bytes32","name":"node","type":"bytes32"},{"indexed":true,"internalType":"bytes4","name":"interfaceID","type":"bytes4"},{"indexed":false,"internalType":"address","name":"implementer","type":"address"}],"name":"InterfaceChanged","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"internalType":"bytes32","name":"node","type":"bytes32"},{"indexed":false,"internalType":"string","name":"name","type":"string"}],"name":"NameChanged","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"internalType":"bytes32","name":"node","type":"bytes32"},{"indexed":false,"internalType":"bytes32","name":"x","type":"bytes32"},{"indexed":false,"internalType":"bytes32","name":"y","type":"bytes32"}],"name":"PubkeyChanged","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"internalType":"bytes32","name":"node","type":"bytes32"},{"indexed":true,"internalType":"string","name":"indexedKey","type":"string"},{"indexed":false,"internalType":"string","name":"key","type":"string"}],"name":"TextChanged","type":"event"},{"constant":true,"inputs":[{"internalType":"bytes32","name":"node","type":"bytes32"},{"internalType":"uint256","name":"contentTypes","type":"uint256"}],"name":"ABI","outputs":[{"internalType":"uint256","name":"","type":"uint256"},{"internalType":"bytes","name":"","type":"bytes"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[{"internalType":"bytes32","name":"node","type":"bytes32"}],"name":"addr","outputs":[{"internalType":"address payable","name":"","type":"address"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[{"internalType":"bytes32","name":"node","type":"bytes32"},{"internalType":"uint256","name":"coinType","type":"uint256"}],"name":"addr","outputs":[{"internalType":"bytes","name":"","type":"bytes"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[{"internalType":"bytes32","name":"","type":"bytes32"},{"internalType":"address","name":"","type":"address"},{"internalType":"address","name":"","type":"address"}],"name":"authorisations","outputs":[{"internalType":"bool","name":"","type":"bool"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":false,"inputs":[{"internalType":"bytes32","name":"node","type":"bytes32"}],"name":"clearDNSZone","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":true,"inputs":[{"internalType":"bytes32","name":"node","type":"bytes32"}],"name":"contenthash","outputs":[{"internalType":"bytes","name":"","type":"bytes"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[{"internalType":"bytes32","name":"node","type":"bytes32"},{"internalType":"bytes32","name":"name","type":"bytes32"},{"internalType":"uint16","name":"resource","type":"uint16"}],"name":"dnsRecord","outputs":[{"internalType":"bytes","name":"","type":"bytes"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[{"internalType":"bytes32","name":"node","type":"bytes32"},{"internalType":"bytes32","name":"name","type":"bytes32"}],"name":"hasDNSRecords","outputs":[{"internalType":"bool","name":"","type":"bool"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[{"internalType":"bytes32","name":"node","type":"bytes32"},{"internalType":"bytes4","name":"interfaceID","type":"bytes4"}],"name":"interfaceImplementer","outputs":[{"internalType":"address","name":"","type":"address"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":false,"inputs":[{"internalType":"bytes[]","name":"data","type":"bytes[]"}],"name":"multicall","outputs":[{"internalType":"bytes[]","name":"results","type":"bytes[]"}],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":true,"inputs":[{"internalType":"bytes32","name":"node","type":"bytes32"}],"name":"name","outputs":[{"internalType":"string","name":"","type":"string"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[{"internalType":"bytes32","name":"node","type":"bytes32"}],"name":"pubkey","outputs":[{"internalType":"bytes32","name":"x","type":"bytes32"},{"internalType":"bytes32","name":"y","type":"bytes32"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":false,"inputs":[{"internalType":"bytes32","name":"node","type":"bytes32"},{"internalType":"uint256","name":"contentType","type":"uint256"},{"internalType":"bytes","name":"data","type":"bytes"}],"name":"setABI","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":false,"inputs":[{"internalType":"bytes32","name":"node","type":"bytes32"},{"internalType":"uint256","name":"coinType","type":"uint256"},{"internalType":"bytes","name":"a","type":"bytes"}],"name":"setAddr","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":false,"inputs":[{"internalType":"bytes32","name":"node","type":"bytes32"},{"internalType":"address","name":"a","type":"address"}],"name":"setAddr","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":false,"inputs":[{"internalType":"bytes32","name":"node","type":"bytes32"},{"internalType":"address","name":"target","type":"address"},{"internalType":"bool","name":"isAuthorised","type":"bool"}],"name":"setAuthorisation","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":false,"inputs":[{"internalType":"bytes32","name":"node","type":"bytes32"},{"internalType":"bytes","name":"hash","type":"bytes"}],"name":"setContenthash","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":false,"inputs":[{"internalType":"bytes32","name":"node","type":"bytes32"},{"internalType":"bytes","name":"data","type":"bytes"}],"name":"setDNSRecords","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":false,"inputs":[{"internalType":"bytes32","name":"node","type":"bytes32"},{"internalType":"bytes4","name":"interfaceID","type":"bytes4"},{"internalType":"address","name":"implementer","type":"address"}],"name":"setInterface","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":false,"inputs":[{"internalType":"bytes32","name":"node","type":"bytes32"},{"internalType":"string","name":"name","type":"string"}],"name":"setName","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":false,"inputs":[{"internalType":"bytes32","name":"node","type":"bytes32"},{"internalType":"bytes32","name":"x","type":"bytes32"},{"internalType":"bytes32","name":"y","type":"bytes32"}],"name":"setPubkey","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":false,"inputs":[{"internalType":"bytes32","name":"node","type":"bytes32"},{"internalType":"string","name":"key","type":"string"},{"internalType":"string","name":"value","type":"string"}],"name":"setText","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":true,"inputs":[{"internalType":"bytes4","name":"interfaceID","type":"bytes4"}],"name":"supportsInterface","outputs":[{"internalType":"bool","name":"","type":"bool"}],"payable":false,"stateMutability":"pure","type":"function"},{"constant":true,"inputs":[{"internalType":"bytes32","name":"node","type":"bytes32"},{"internalType":"string","name":"key","type":"string"}],"name":"text","outputs":[{"internalType":"string","name":"","type":"string"}],"payable":false,"stateMutability":"view","type":"function"}]''';
const ensInterface =
    '''[{"anonymous":false,"inputs":[{"indexed":true,"internalType":"bytes32","name":"node","type":"bytes32"},{"indexed":true,"internalType":"bytes32","name":"label","type":"bytes32"},{"indexed":false,"internalType":"address","name":"owner","type":"address"}],"name":"NewOwner","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"internalType":"bytes32","name":"node","type":"bytes32"},{"indexed":false,"internalType":"address","name":"resolver","type":"address"}],"name":"NewResolver","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"internalType":"bytes32","name":"node","type":"bytes32"},{"indexed":false,"internalType":"uint64","name":"ttl","type":"uint64"}],"name":"NewTTL","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"internalType":"bytes32","name":"node","type":"bytes32"},{"indexed":false,"internalType":"address","name":"owner","type":"address"}],"name":"Transfer","type":"event"},{"inputs":[{"internalType":"bytes32","name":"node","type":"bytes32"}],"name":"owner","outputs":[{"internalType":"address","name":"","type":"address"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"bytes32","name":"node","type":"bytes32"}],"name":"resolver","outputs":[{"internalType":"address","name":"","type":"address"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"bytes32","name":"node","type":"bytes32"},{"internalType":"address","name":"owner","type":"address"}],"name":"setOwner","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"bytes32","name":"node","type":"bytes32"},{"internalType":"address","name":"resolver","type":"address"}],"name":"setResolver","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"bytes32","name":"node","type":"bytes32"},{"internalType":"bytes32","name":"label","type":"bytes32"},{"internalType":"address","name":"owner","type":"address"}],"name":"setSubnodeOwner","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"bytes32","name":"node","type":"bytes32"},{"internalType":"uint64","name":"ttl","type":"uint64"}],"name":"setTTL","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"bytes32","name":"node","type":"bytes32"}],"name":"ttl","outputs":[{"internalType":"uint64","name":"","type":"uint64"}],"stateMutability":"view","type":"function"}]''';
const uniswapAbi =
    '''[{"inputs":[{"internalType":"address","name":"_factory","type":"address"},{"internalType":"address","name":"_WETH","type":"address"}],"stateMutability":"nonpayable","type":"constructor"},{"inputs":[],"name":"WETH","outputs":[{"internalType":"address","name":"","type":"address"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"address","name":"tokenA","type":"address"},{"internalType":"address","name":"tokenB","type":"address"},{"internalType":"uint256","name":"amountADesired","type":"uint256"},{"internalType":"uint256","name":"amountBDesired","type":"uint256"},{"internalType":"uint256","name":"amountAMin","type":"uint256"},{"internalType":"uint256","name":"amountBMin","type":"uint256"},{"internalType":"address","name":"to","type":"address"},{"internalType":"uint256","name":"deadline","type":"uint256"}],"name":"addLiquidity","outputs":[{"internalType":"uint256","name":"amountA","type":"uint256"},{"internalType":"uint256","name":"amountB","type":"uint256"},{"internalType":"uint256","name":"liquidity","type":"uint256"}],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"address","name":"token","type":"address"},{"internalType":"uint256","name":"amountTokenDesired","type":"uint256"},{"internalType":"uint256","name":"amountTokenMin","type":"uint256"},{"internalType":"uint256","name":"amountETHMin","type":"uint256"},{"internalType":"address","name":"to","type":"address"},{"internalType":"uint256","name":"deadline","type":"uint256"}],"name":"addLiquidityETH","outputs":[{"internalType":"uint256","name":"amountToken","type":"uint256"},{"internalType":"uint256","name":"amountETH","type":"uint256"},{"internalType":"uint256","name":"liquidity","type":"uint256"}],"stateMutability":"payable","type":"function"},{"inputs":[],"name":"factory","outputs":[{"internalType":"address","name":"","type":"address"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"uint256","name":"amountOut","type":"uint256"},{"internalType":"uint256","name":"reserveIn","type":"uint256"},{"internalType":"uint256","name":"reserveOut","type":"uint256"}],"name":"getAmountIn","outputs":[{"internalType":"uint256","name":"amountIn","type":"uint256"}],"stateMutability":"pure","type":"function"},{"inputs":[{"internalType":"uint256","name":"amountIn","type":"uint256"},{"internalType":"uint256","name":"reserveIn","type":"uint256"},{"internalType":"uint256","name":"reserveOut","type":"uint256"}],"name":"getAmountOut","outputs":[{"internalType":"uint256","name":"amountOut","type":"uint256"}],"stateMutability":"pure","type":"function"},{"inputs":[{"internalType":"uint256","name":"amountOut","type":"uint256"},{"internalType":"address[]","name":"path","type":"address[]"}],"name":"getAmountsIn","outputs":[{"internalType":"uint256[]","name":"amounts","type":"uint256[]"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"uint256","name":"amountIn","type":"uint256"},{"internalType":"address[]","name":"path","type":"address[]"}],"name":"getAmountsOut","outputs":[{"internalType":"uint256[]","name":"amounts","type":"uint256[]"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"uint256","name":"amountA","type":"uint256"},{"internalType":"uint256","name":"reserveA","type":"uint256"},{"internalType":"uint256","name":"reserveB","type":"uint256"}],"name":"quote","outputs":[{"internalType":"uint256","name":"amountB","type":"uint256"}],"stateMutability":"pure","type":"function"},{"inputs":[{"internalType":"address","name":"tokenA","type":"address"},{"internalType":"address","name":"tokenB","type":"address"},{"internalType":"uint256","name":"liquidity","type":"uint256"},{"internalType":"uint256","name":"amountAMin","type":"uint256"},{"internalType":"uint256","name":"amountBMin","type":"uint256"},{"internalType":"address","name":"to","type":"address"},{"internalType":"uint256","name":"deadline","type":"uint256"}],"name":"removeLiquidity","outputs":[{"internalType":"uint256","name":"amountA","type":"uint256"},{"internalType":"uint256","name":"amountB","type":"uint256"}],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"address","name":"token","type":"address"},{"internalType":"uint256","name":"liquidity","type":"uint256"},{"internalType":"uint256","name":"amountTokenMin","type":"uint256"},{"internalType":"uint256","name":"amountETHMin","type":"uint256"},{"internalType":"address","name":"to","type":"address"},{"internalType":"uint256","name":"deadline","type":"uint256"}],"name":"removeLiquidityETH","outputs":[{"internalType":"uint256","name":"amountToken","type":"uint256"},{"internalType":"uint256","name":"amountETH","type":"uint256"}],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"address","name":"token","type":"address"},{"internalType":"uint256","name":"liquidity","type":"uint256"},{"internalType":"uint256","name":"amountTokenMin","type":"uint256"},{"internalType":"uint256","name":"amountETHMin","type":"uint256"},{"internalType":"address","name":"to","type":"address"},{"internalType":"uint256","name":"deadline","type":"uint256"}],"name":"removeLiquidityETHSupportingFeeOnTransferTokens","outputs":[{"internalType":"uint256","name":"amountETH","type":"uint256"}],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"address","name":"token","type":"address"},{"internalType":"uint256","name":"liquidity","type":"uint256"},{"internalType":"uint256","name":"amountTokenMin","type":"uint256"},{"internalType":"uint256","name":"amountETHMin","type":"uint256"},{"internalType":"address","name":"to","type":"address"},{"internalType":"uint256","name":"deadline","type":"uint256"},{"internalType":"bool","name":"approveMax","type":"bool"},{"internalType":"uint8","name":"v","type":"uint8"},{"internalType":"bytes32","name":"r","type":"bytes32"},{"internalType":"bytes32","name":"s","type":"bytes32"}],"name":"removeLiquidityETHWithPermit","outputs":[{"internalType":"uint256","name":"amountToken","type":"uint256"},{"internalType":"uint256","name":"amountETH","type":"uint256"}],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"address","name":"token","type":"address"},{"internalType":"uint256","name":"liquidity","type":"uint256"},{"internalType":"uint256","name":"amountTokenMin","type":"uint256"},{"internalType":"uint256","name":"amountETHMin","type":"uint256"},{"internalType":"address","name":"to","type":"address"},{"internalType":"uint256","name":"deadline","type":"uint256"},{"internalType":"bool","name":"approveMax","type":"bool"},{"internalType":"uint8","name":"v","type":"uint8"},{"internalType":"bytes32","name":"r","type":"bytes32"},{"internalType":"bytes32","name":"s","type":"bytes32"}],"name":"removeLiquidityETHWithPermitSupportingFeeOnTransferTokens","outputs":[{"internalType":"uint256","name":"amountETH","type":"uint256"}],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"address","name":"tokenA","type":"address"},{"internalType":"address","name":"tokenB","type":"address"},{"internalType":"uint256","name":"liquidity","type":"uint256"},{"internalType":"uint256","name":"amountAMin","type":"uint256"},{"internalType":"uint256","name":"amountBMin","type":"uint256"},{"internalType":"address","name":"to","type":"address"},{"internalType":"uint256","name":"deadline","type":"uint256"},{"internalType":"bool","name":"approveMax","type":"bool"},{"internalType":"uint8","name":"v","type":"uint8"},{"internalType":"bytes32","name":"r","type":"bytes32"},{"internalType":"bytes32","name":"s","type":"bytes32"}],"name":"removeLiquidityWithPermit","outputs":[{"internalType":"uint256","name":"amountA","type":"uint256"},{"internalType":"uint256","name":"amountB","type":"uint256"}],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"uint256","name":"amountOut","type":"uint256"},{"internalType":"address[]","name":"path","type":"address[]"},{"internalType":"address","name":"to","type":"address"},{"internalType":"uint256","name":"deadline","type":"uint256"}],"name":"swapETHForExactTokens","outputs":[{"internalType":"uint256[]","name":"amounts","type":"uint256[]"}],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"uint256","name":"amountOutMin","type":"uint256"},{"internalType":"address[]","name":"path","type":"address[]"},{"internalType":"address","name":"to","type":"address"},{"internalType":"uint256","name":"deadline","type":"uint256"}],"name":"swapExactETHForTokens","outputs":[{"internalType":"uint256[]","name":"amounts","type":"uint256[]"}],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"uint256","name":"amountOutMin","type":"uint256"},{"internalType":"address[]","name":"path","type":"address[]"},{"internalType":"address","name":"to","type":"address"},{"internalType":"uint256","name":"deadline","type":"uint256"}],"name":"swapExactETHForTokensSupportingFeeOnTransferTokens","outputs":[],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"uint256","name":"amountIn","type":"uint256"},{"internalType":"uint256","name":"amountOutMin","type":"uint256"},{"internalType":"address[]","name":"path","type":"address[]"},{"internalType":"address","name":"to","type":"address"},{"internalType":"uint256","name":"deadline","type":"uint256"}],"name":"swapExactTokensForETH","outputs":[{"internalType":"uint256[]","name":"amounts","type":"uint256[]"}],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"uint256","name":"amountIn","type":"uint256"},{"internalType":"uint256","name":"amountOutMin","type":"uint256"},{"internalType":"address[]","name":"path","type":"address[]"},{"internalType":"address","name":"to","type":"address"},{"internalType":"uint256","name":"deadline","type":"uint256"}],"name":"swapExactTokensForETHSupportingFeeOnTransferTokens","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"uint256","name":"amountIn","type":"uint256"},{"internalType":"uint256","name":"amountOutMin","type":"uint256"},{"internalType":"address[]","name":"path","type":"address[]"},{"internalType":"address","name":"to","type":"address"},{"internalType":"uint256","name":"deadline","type":"uint256"}],"name":"swapExactTokensForTokens","outputs":[{"internalType":"uint256[]","name":"amounts","type":"uint256[]"}],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"uint256","name":"amountIn","type":"uint256"},{"internalType":"uint256","name":"amountOutMin","type":"uint256"},{"internalType":"address[]","name":"path","type":"address[]"},{"internalType":"address","name":"to","type":"address"},{"internalType":"uint256","name":"deadline","type":"uint256"}],"name":"swapExactTokensForTokensSupportingFeeOnTransferTokens","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"uint256","name":"amountOut","type":"uint256"},{"internalType":"uint256","name":"amountInMax","type":"uint256"},{"internalType":"address[]","name":"path","type":"address[]"},{"internalType":"address","name":"to","type":"address"},{"internalType":"uint256","name":"deadline","type":"uint256"}],"name":"swapTokensForExactETH","outputs":[{"internalType":"uint256[]","name":"amounts","type":"uint256[]"}],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"uint256","name":"amountOut","type":"uint256"},{"internalType":"uint256","name":"amountInMax","type":"uint256"},{"internalType":"address[]","name":"path","type":"address[]"},{"internalType":"address","name":"to","type":"address"},{"internalType":"uint256","name":"deadline","type":"uint256"}],"name":"swapTokensForExactTokens","outputs":[{"internalType":"uint256[]","name":"amounts","type":"uint256[]"}],"stateMutability":"nonpayable","type":"function"},{"stateMutability":"payable","type":"receive"}]''';

enum SolanaClusters {
  mainNet,
  devNet,
  testNet,
}

solana.SolanaClient getSolanaClient(SolanaClusters solanaClusterType) {
  solanaClusterType ??= SolanaClusters.mainNet;

  String solanaRpcUrl = '';
  String solanaWebSocket = '';
  switch (solanaClusterType) {
    case SolanaClusters.mainNet:
      solanaRpcUrl = 'https://solana-api.projectserum.com';
      solanaWebSocket = 'wss://solana-api.projectserum.com';
      break;
    case SolanaClusters.devNet:
      solanaRpcUrl = 'https://api.devnet.solana.com';
      solanaWebSocket = 'wss://api.devnet.solana.com';
      break;
    case SolanaClusters.testNet:
      solanaRpcUrl = 'https://api.testnet.solana.com';
      solanaWebSocket = 'wss://api.testnet.solana.com';
      break;
    default:
  }

  return solana.SolanaClient(
    rpcUrl: Uri.parse(solanaRpcUrl),
    websocketUrl: Uri.parse(solanaWebSocket),
  );
}

Future<Map> viewUserTokens(
  int chainId,
  String address, {
  bool skipNetworkRequest,
}) async {
  final pref = Hive.box(secureStorageKey);
  final tokenListKey = 'tokenListKey_$chainId-$address';

  final tokenList = pref.get(tokenListKey);
  Map userTokens = {
    'msg': 'could not fetch tokens',
    'success': false,
  };
  if (tokenList != null) {
    userTokens = {'msg': json.decode(tokenList) as Map, 'success': true};
  }

  if (skipNetworkRequest) return userTokens;
  try {
    final blockChainDetails = getEVMBlockchains()
        .values
        .toList()
        .where((element) => element['chainId'] == chainId)
        .toList()[0];

    String blockExplorerUrl =
        blockChainDetails['blockExplorer'].toString().replaceFirst(
              'tx/$transactionhashTemplateKey',
              'address/$address',
            );

    final response = (await get(Uri.parse(blockExplorerUrl))).body;

    final document = html.parse(response);

    final tokenList = {
      'erc721': [],
      'erc1155': [],
      'erc20': [],
    };
    String currentTokenId = "";

    document.querySelectorAll('.list-custom').forEach((element) {
      final previousElement = element.previousElementSibling;
      final tokenName = element.querySelector('.list-name').text;
      final tokenAmount = element.querySelector('.list-amount').text;
      final tokenAddress = element
          .querySelector('a')
          .attributes['href']
          .split('/token/')[1]
          .split('?')[0];

      // get previous element
      if (previousElement.className.contains('list-custom-divider')) {
        currentTokenId = previousElement.querySelector('strong').text;
      }

      if (currentTokenId.contains("20")) {
        tokenList['erc20'].add({
          'tokenName': tokenName,
          'tokenAmount': tokenAmount,
          'tokenAddress': tokenAddress,
        });
      } else if (currentTokenId.contains("1155")) {
        tokenList['erc1155'].add({
          'tokenName': tokenName,
          'tokenAmount': tokenAmount,
          'tokenAddress': tokenAddress,
        });
      } else if (currentTokenId.contains("721")) {
        tokenList['erc721'].add({
          'tokenName': tokenName,
          'tokenAmount': tokenAmount,
          'tokenAddress': tokenAddress,
        });
      }
    });
    await pref.put(tokenListKey, json.encode(tokenList));
    return {'msg': tokenList, 'success': true};
  } catch (e) {
    return userTokens;
  }
}

ipfsTohttp(String url) {
  if (url == null) {
    return '';
  }
  url = url.trim();
  return url.startsWith('ipfs://')
      ? 'https://ipfs.io/ipfs/${url.replaceFirst('ipfs://', '')}'
      : url;
}

String localHostToIpAddress(String url) {
  Uri uri = Uri.parse(url);
  const localhostNames = ['localhost', '127.0.0.1', '::1', '[::1]'];

  if (localhostNames.contains(uri.host)) {
    uri = uri.replace(
      host: Platform.isAndroid ? '10.0.2.2' : '127.0.0.1',
    );
  }
  return uri.toString();
}

buildSwapUi({
  List tokenList,
  Function onSelect,
  BuildContext context,
}) async {
  final searchCoinController = TextEditingController();

  await slideUpPanel(
    context,
    StatefulBuilder(
      builder: ((context, __) {
        List<Widget> listToken = [];
        tokenList
            .where((element) {
              if (searchCoinController.text == '') {
                return true;
              } else {
                return element['name']
                        .toString()
                        .toLowerCase()
                        .contains(searchCoinController.text.toLowerCase()) ||
                    element['symbol']
                        .toString()
                        .toLowerCase()
                        .contains(searchCoinController.text.toLowerCase());
              }
            })
            .take(50)
            .toList()
            .forEach((element) {
              listToken.add(
                InkWell(
                  onTap: () {
                    onSelect(element);
                    Get.back();
                  },
                  child: Row(children: [
                    Flexible(
                      child: CachedNetworkImage(
                        imageBuilder: (context, imageProvider) => Container(
                          width: 70,
                          height: 70,
                          decoration: BoxDecoration(
                            shape: BoxShape.circle,
                            image: DecorationImage(
                              image: imageProvider,
                              fit: BoxFit.cover,
                            ),
                          ),
                        ),
                        imageUrl: ipfsTohttp(element['logoURI']),
                        fit: BoxFit.cover,
                        placeholder: (context, url) => Column(
                          mainAxisAlignment: MainAxisAlignment.center,
                          mainAxisSize: MainAxisSize.min,
                          children: const [
                            SizedBox(
                              width: 20,
                              height: 20,
                              child: Loader(
                                color: appPrimaryColor,
                              ),
                            )
                          ],
                        ),
                        errorWidget: (context, url, error) => const Icon(
                          Icons.error,
                          color: Colors.red,
                        ),
                        width: 40,
                        height: 40,
                      ),
                    ),
                    const SizedBox(
                      width: 20,
                    ),
                    Flexible(
                      child: Column(
                        mainAxisAlignment: MainAxisAlignment.start,
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: [
                          //
                          Text(
                            ellipsify(str: element['name'] ?? ''),
                            style: //bold
                                const TextStyle(
                              fontSize: 16,
                              fontWeight: FontWeight.w500,
                              overflow: TextOverflow.fade,
                            ),
                          ),
                          const SizedBox(
                            height: 5,
                          ),
                          Text(
                            ellipsify(str: element['symbol'] ?? ''),
                            style: const TextStyle(
                              overflow: TextOverflow.fade,
                              color: Colors.grey,
                              fontSize: 14,
                            ),
                          ),
                        ],
                      ),
                    ),
                  ]),
                ),
              );

              listToken.add(const SizedBox(
                height: 20,
              ));
            });

        return Container(
          color: Colors.transparent,
          child: SafeArea(
            child: SingleChildScrollView(
              child: Padding(
                padding: const EdgeInsets.all(25),
                child: Column(
                    crossAxisAlignment: CrossAxisAlignment.center,
                    children: [
                      Row(
                        children: [
                          GestureDetector(
                            onTap: () {
                              if (Navigator.canPop(context)) {
                                Get.back();
                              }
                            },
                            child: const Icon(
                              Icons.arrow_back,
                              size: 30,
                            ),
                          ),
                          const SizedBox(
                            width: 20,
                          ),
                          Text(
                            AppLocalizations.of(context).selectTokens,
                            style: const TextStyle(fontSize: 18),
                          ),
                        ],
                      ),
                      const SizedBox(
                        height: 20,
                      ),
                      Container(
                        decoration: BoxDecoration(
                          borderRadius: BorderRadius.circular(10),
                          // border color
                          border: Border.all(
                            color: const Color(0xff2A7FE2),
                            width: 1,
                          ),
                        ),
                        child: Padding(
                          padding: const EdgeInsets.fromLTRB(30, 0, 0, 0),
                          child: TextFormField(
                            controller: searchCoinController,
                            onChanged: (value) async {
                              __(() {});
                            },
                            textInputAction: TextInputAction.search,
                            decoration: InputDecoration(
                              border: InputBorder.none,
                              suffixIcon: IconButton(
                                onPressed: () {},
                                icon: const Icon(
                                  Icons.search,
                                ),
                              ),
                              hintText: AppLocalizations.of(context).searchCoin,
                              hintStyle: const TextStyle(),
                            ),
                          ),
                        ),
                      ),
                      const SizedBox(
                        height: 20,
                      ),
                      ...listToken
                    ]),
              ),
            ),
          ),
        );
      }),
    ),
  );
}

Future<bool> authenticateIsAvailable() async {
  final localAuth = LocalAuthentication();
  final isAvailable = await localAuth.canCheckBiometrics;
  final isDeviceSupported = await localAuth.isDeviceSupported();
  return isAvailable && isDeviceSupported;
}

Future<bool> localAuthentication() async {
  final localAuth = LocalAuthentication();
  bool didAuthenticate = false;
  if (await authenticateIsAvailable()) {
    didAuthenticate = await localAuth.authenticate(
      localizedReason: 'Your authentication is needed.',
    );
  }

  return didAuthenticate ?? false;
}

Future<bool> authenticate(BuildContext context,
    {bool disableGoBack_, bool useLocalAuth}) async {
  bool didAuthenticate = false;
  await disEnableScreenShot();
  if (useLocalAuth ?? true) {
    didAuthenticate = await localAuthentication();
  }
  if (!didAuthenticate) {
    didAuthenticate = await Get.to(
      Security(
        isEnterPin: true,
        useLocalAuth: useLocalAuth,
      ),
    );
  }

  await enableScreenShot();

  return didAuthenticate ?? false;
}

Future getAddTokenKey() async {
  final pref = Hive.box(secureStorageKey);
  final String mnemonicHash = sha3(pref.get(currentMmenomicKey));
  return 'userTokenList$mnemonicHash'.toLowerCase();
}

Future<Map> oneInchSwapUrlResponse({
  String fromTokenAddress,
  String toTokenAddress,
  double amountInWei,
  String fromAddress,
  double slippage,
  int chainId,
}) async {
  try {
    slippage = slippage ?? 0.1;
    String url =
        "https://api.1inch.exchange/v3.0/$chainId/swap?fromTokenAddress=$fromTokenAddress&toTokenAddress=$toTokenAddress&amount=${BigInt.from(amountInWei).toString()}&fromAddress=$fromAddress&slippage=$slippage&disableEstimate=true";

    final response = await get(Uri.parse(url)).timeout(networkTimeOutDuration);

    final responseBody = jsonDecode(response.body) as Map;
    if (response.statusCode ~/ 100 == 4 || response.statusCode ~/ 100 == 5) {
      throw Exception(responseBody);
    }
    return responseBody;
  } catch (e) {
    return null;
  }
}

Future<Map> approveTokenFor1inch(
    int chainId, double amountInWei, String contractAddr) async {
  String url =
      "https://api.1inch.exchange/v3.0/$chainId/approve/calldata?tokenAddress=$contractAddr&amount=${BigInt.from(amountInWei).toString()}";

  final response = jsonDecode((await get(Uri.parse(url))).body) as Map;
  return response;
}

Map getBitCoinPOSBlockchains() {
  Map blockChains = {
    'Bitcoin': {
      'symbol': 'BTC',
      'default': 'BTC',
      'blockExplorer':
          'https://live.blockcypher.com/btc/tx/$transactionhashTemplateKey',
      'image': 'assets/bitcoin.jpg',
      'POSNetwork': bitcoin.bitcoin,
      'P2WPKH': true,
      'derivationPath': "m/84'/0'/0'/0/0"
    },
    'Litecoin': {
      'symbol': 'LTC',
      'default': 'LTC',
      'blockExplorer':
          'https://live.blockcypher.com/ltc/tx/$transactionhashTemplateKey',
      'image': 'assets/litecoin.png',
      'POSNetwork': litecoin,
      'P2WPKH': true,
      'derivationPath': "m/84'/2'/0'/0/0"
    },
    'Dash': {
      'symbol': 'DASH',
      'default': 'DASH',
      'blockExplorer':
          'https://live.blockcypher.com/dash/tx/$transactionhashTemplateKey',
      'image': 'assets/dash.png',
      'POSNetwork': dash,
      'P2WPKH': false,
      'derivationPath': "m/44'/5'/0'/0/0"
    },
    'ZCash': {
      'symbol': 'ZEC',
      'default': 'ZEC',
      'blockExplorer':
          'https://zcashblockexplorer.com/transactions/$transactionhashTemplateKey',
      'image': 'assets/zcash.png',
      'POSNetwork': zcash,
      'P2WPKH': false,
      'derivationPath': "m/44'/133'/0'/0/0"
    },
    'Dogecoin': {
      'symbol': 'DOGE',
      'default': 'DOGE',
      'blockExplorer':
          'https://live.blockcypher.com/doge/tx/$transactionhashTemplateKey',
      'image': 'assets/dogecoin.png',
      'POSNetwork': dogecoin,
      'P2WPKH': false,
      'derivationPath': "m/44'/3'/0'/0/0"
    }
  };

  if (enableTestNet) {
    blockChains['Bitcoin(Test)'] = {
      'symbol': 'BTC',
      'default': 'BTC',
      'blockExplorer':
          'https://www.blockchain.com/btc-testnet/tx/$transactionhashTemplateKey',
      'image': 'assets/bitcoin.jpg',
      'POSNetwork': testnet,
      'P2WPKH': false,
      'derivationPath': "m/44'/0'/0'/0/0"
    };
  }

  return blockChains;
}

Map getEVMBlockchains() {
  Map blockChains = {
    'Ethereum': {
      "rpc": 'https://mainnet.infura.io/v3/$infuraApiKey',
      'chainId': 1,
      'blockExplorer': 'https://etherscan.io/tx/$transactionhashTemplateKey',
      'symbol': 'ETH',
      'default': 'ETH',
      'image': 'assets/ethereum_logo.png',
      'coinType': 60
    },
    'Smart Chain': {
      "rpc": 'https://bsc-dataseed.binance.org/',
      'chainId': 56,
      'blockExplorer': 'https://bscscan.com/tx/$transactionhashTemplateKey',
      'symbol': 'BNB',
      'default': 'BNB',
      'image': 'assets/smartchain.png',
      'coinType': 60
    },
    'Polygon Matic': {
      "rpc": 'https://polygon-rpc.com',
      'chainId': 137,
      'blockExplorer': 'https://polygonscan.com/tx/$transactionhashTemplateKey',
      'symbol': 'MATIC',
      'default': 'MATIC',
      'image': 'assets/polygon.png',
      'coinType': 60
    },
    'Avalanche': {
      "rpc": 'https://api.avax.network/ext/bc/C/rpc',
      'chainId': 43114,
      'blockExplorer': 'https://snowtrace.io/tx/$transactionhashTemplateKey',
      'symbol': 'AVAX',
      'default': 'AVAX',
      'image': 'assets/avalanche.png',
      'coinType': 60
    },
    'Fantom': {
      "rpc": 'https://rpc.ftm.tools/',
      'chainId': 250,
      'blockExplorer': 'https://ftmscan.com/tx/$transactionhashTemplateKey',
      'symbol': 'FTM',
      'default': 'FTM',
      'image': 'assets/fantom.png',
      'coinType': 60
    },
    'Arbitrum': {
      "rpc": 'https://arb1.arbitrum.io/rpc',
      'chainId': 42161,
      'blockExplorer': 'https://arbiscan.io/tx/$transactionhashTemplateKey',
      'symbol': 'ETH',
      'default': 'ETH',
      'image': 'assets/arbitrum.jpg',
      'coinType': 60
    },
    'Optimism': {
      "rpc": 'https://mainnet.optimism.io',
      'chainId': 10,
      'blockExplorer':
          'https://optimistic.etherscan.io/tx/$transactionhashTemplateKey',
      'symbol': 'ETH',
      'default': 'ETH',
      'image': 'assets/optimism.png',
      'coinType': 60
    },
    'Ethereum Classic': {
      'symbol': 'ETC',
      'default': 'ETH',
      'blockExplorer':
          'https://blockscout.com/etc/mainnet/tx/$transactionhashTemplateKey',
      'rpc': 'https://www.ethercluster.com/etc',
      'chainId': 61,
      'image': 'assets/ethereum-classic.png',
      'coinType': 61
    },
    'Cronos': {
      "rpc": 'https://evm.cronos.org',
      'chainId': 25,
      'blockExplorer': 'https://cronoscan.com/tx/$transactionhashTemplateKey',
      'symbol': 'CRO',
      'default': 'CRO',
      'image': 'assets/cronos.png',
      'coinType': 60
    },
    'Milkomeda Cardano': {
      "rpc": ' https://rpc-mainnet-cardano-evm.c1.milkomeda.com',
      'chainId': 2001,
      'blockExplorer':
          'https://explorer-mainnet-cardano-evm.c1.milkomeda.com/tx/$transactionhashTemplateKey',
      'symbol': 'MilkADA',
      'default': 'MilkADA',
      'image': 'assets/milko-cardano.jpeg',
      'coinType': 60
    },
    'Huobi Chain': {
      "rpc": 'https://http-mainnet-node.huobichain.com/',
      'chainId': 128,
      'blockExplorer': 'https://hecoinfo.com/tx/$transactionhashTemplateKey',
      'symbol': 'HT',
      'default': 'HT',
      'image': 'assets/huobi.png',
      'coinType': 60
    },
    'Kucoin Chain': {
      "rpc": 'https://rpc-mainnet.kcc.network',
      'chainId': 321,
      'blockExplorer': 'https://explorer.kcc.io/tx/$transactionhashTemplateKey',
      'symbol': 'KCS',
      'default': 'KCS',
      'image': 'assets/kucoin.jpeg',
      'coinType': 60
    },
    'Elastos': {
      "rpc": 'https://api.elastos.io/eth',
      'chainId': 20,
      'blockExplorer':
          'https://explorer.elaeth.io/tx/$transactionhashTemplateKey',
      'symbol': 'ELA',
      'default': 'ELA',
      'image': 'assets/elastos.png',
      'coinType': 60
    },
    'xDai': {
      "rpc": 'https://rpc.xdaichain.com/',
      'chainId': 100,
      'blockExplorer':
          'https://blockscout.com/xdai/mainnet/tx/$transactionhashTemplateKey',
      'symbol': 'XDAI',
      'default': 'XDAI',
      'image': 'assets/xdai.jpg',
      'coinType': 60
    },
    'Ubiq': {
      "rpc": 'https://rpc.octano.dev/',
      'chainId': 8,
      'blockExplorer': 'https://ubiqscan.io/tx/$transactionhashTemplateKey',
      'symbol': 'UBQ',
      'default': 'UBQ',
      'image': 'assets/ubiq.png',
      'coinType': 60
    },
    'Celo': {
      "rpc": 'https://rpc.ankr.com/celo',
      'chainId': 42220,
      'blockExplorer':
          'https://explorer.celo.org/tx/$transactionhashTemplateKey',
      'symbol': 'CELO',
      'default': 'CELO',
      'image': 'assets/celo.png',
      'coinType': 60
    },
    'Fuse': {
      "rpc": 'https://rpc.fuse.io',
      'chainId': 122,
      'blockExplorer':
          'https://explorer.fuse.io/tx/$transactionhashTemplateKey',
      'symbol': 'FUSE',
      'default': 'FUSE',
      'image': 'assets/fuse.png',
      'coinType': 60
    },
    'Aurora': {
      "rpc": 'https://mainnet.aurora.dev',
      'chainId': 1313161554,
      'blockExplorer': 'https://aurorascan.dev/tx/$transactionhashTemplateKey',
      'symbol': 'ETH',
      'default': 'ETH',
      'image': 'assets/aurora.png',
      'coinType': 60
    },
    'Thunder Token': {
      "rpc": 'https://mainnet-rpc.thundercore.com',
      'chainId': 108,
      'blockExplorer':
          'https://viewblock.io/thundercore/tx/$transactionhashTemplateKey',
      'symbol': 'TT',
      'default': 'TT',
      'image': 'assets/thunder-token.jpeg',
      'coinType': 1001
    },
    'GoChain': {
      "rpc": 'https://rpc.gochain.io',
      'chainId': 60,
      'blockExplorer':
          'https://explorer.gochain.io/tx/$transactionhashTemplateKey',
      'symbol': 'GO',
      'default': 'GO',
      'image': 'assets/go-chain.png',
      'coinType': 6060
    },
  };

  if (enableTestNet) {
    blockChains['Smart Chain(Testnet)'] = {
      "rpc": 'https://data-seed-prebsc-1-s1.binance.org:8545/',
      'chainId': 97,
      'blockExplorer':
          'https://testnet.bscscan.com/tx/$transactionhashTemplateKey',
      'symbol': 'BNB',
      'default': 'BNB',
      'image': 'assets/smartchain.png',
      'coinType': 60
    };
    blockChains["Polygon (Mumbai)"] = {
      "rpc": "https://rpc-mumbai.maticvigil.com",
      "chainId": 80001,
      "blockExplorer":
          "https://mumbai.polygonscan.com/tx/$transactionhashTemplateKey",
      "symbol": "MATIC",
      "default": "MATIC",
      "image": "assets/polygon.png",
      'coinType': 60
    };
    blockChains['Ethereum(Goerli)'] = {
      "rpc": 'https://goerli.infura.io/v3/$infuraApiKey',
      'chainId': 5,
      'blockExplorer':
          'https://goerli.etherscan.io/tx/$transactionhashTemplateKey',
      'symbol': 'ETH',
      'default': 'ETH',
      'image': 'assets/ethereum_logo.png',
      'coinType': 60
    };
  }

  return blockChains;
}

Map getSolanaBlockChains() {
  Map blockChains = {
    'Solana': {
      'symbol': 'SOL',
      'default': 'SOL',
      'blockExplorer':
          'https://explorer.solana.com/tx/$transactionhashTemplateKey',
      'image': 'assets/solana.webp',
      'solanaCluster': SolanaClusters.mainNet,
    }
  };
  if (enableTestNet) {
    blockChains['Solana(Devnet)'] = {
      'symbol': 'SOL',
      'default': 'SOL',
      'blockExplorer':
          'https://explorer.solana.com/tx/$transactionhashTemplateKey?cluster=devnet',
      'image': 'assets/solana.webp',
      'solanaCluster': SolanaClusters.devNet,
    };
  }
  return blockChains;
}

Map getStellarBlockChains() {
  Map blockChains = {
    'Stellar': {
      'symbol': 'XLM',
      'default': 'XLM',
      'blockExplorer':
          'https://stellarchain.io/transactions/$transactionhashTemplateKey',
      'image': 'assets/stellar.png', // sdk stellar
      'sdk': stellar.StellarSDK.PUBLIC,
      'cluster': stellar.Network.PUBLIC
    }
  };
  if (enableTestNet) {
    blockChains['Stellar(Testnet)'] = {
      'symbol': 'XLM',
      'default': 'XLM',
      'blockExplorer':
          'https://testnet.stellarchain.io/transactions/$transactionhashTemplateKey',
      'image': 'assets/stellar.png',
      'sdk': stellar.StellarSDK.TESTNET,
      'cluster': stellar.Network.TESTNET
    };
  }
  return blockChains;
}

Map getFilecoinBlockChains() {
  // FIXME:
  return {};
  Map blockChains = {
    'Filecoin': {
      'symbol': 'FIL',
      'default': 'FIL',
      'blockExplorer':
          'https://filscan.io/tipset/message-detail?cid=$transactionhashTemplateKey',
      'image': 'assets/filecoin.png',
      'baseUrl': 'https://api.fivetoken.io/api/uip7oegcgxr6ovab296tpqoh',
      'prefix': 'f'
    }
  };
  if (enableTestNet) {
    blockChains['Filecoin(Testnet)'] = {
      'symbol': 'FIL',
      'default': 'FIL',
      'blockExplorer':
          'https://calibration.filscan.io/tipset/message-detail?cid=$transactionhashTemplateKey',
      'image': 'assets/filecoin.png',
      'baseUrl':
          'https://api.calibration.fivetoken.io/api/uip7oegcgxr6ovab296tpqoh',
      'prefix': 't'
    };
  }
  return blockChains;
}

Map getCardanoBlockChains() {
  Map blockChains = {
    'Cardano': {
      'symbol': 'ADA',
      'default': 'ADA',
      'blockExplorer':
          'https://cardanoscan.io/transaction/$transactionhashTemplateKey',
      'image': 'assets/cardano.png',
      'cardano_network': cardano.NetworkId.mainnet,
      'blockFrostKey': 'mainnetpgkQqXqQ4HjK6gzUKaHW6VU9jcmcKEbd'
    }
  };
  if (enableTestNet) {
    blockChains['Cardano(Prepod)'] = {
      'symbol': 'ADA',
      'default': 'ADA',
      'blockExplorer':
          'https://preprod.cardanoscan.io/transaction/$transactionhashTemplateKey',
      'image': 'assets/cardano.png',
      'cardano_network': cardano.NetworkId.testnet,
      'blockFrostKey': 'preprodmpCaCFGCxLihVPPxXxqEvEnp7dyFmG6J'
    };
  }
  return blockChains;
}

const coinGeckCryptoSymbolToID = {
  "BTC": "bitcoin",
  "ETH": "ethereum",
  "BNB": "binancecoin",
  "AVAX": "avalanche-2",
  "FTM": "fantom",
  "HT": "huobi-token",
  "MATIC": "matic-network",
  "KCS": "kucoin-shares",
  "ELA": "elastos",
  "TT": "thunder-token",
  "GO": "gochain",
  "XDAI": "xdai",
  "UBQ": "ubiq",
  "CELO": "celo",
  "FUSE": "fuse-network-token",
  "LTC": "litecoin",
  "DOGE": "dogecoin",
  "CRO": "crypto-com-chain",
  "SOL": 'solana',
  'ETC': "ethereum-classic",
  'FIL': 'filecoin',
  'XRP': 'ripple',
  'ADA': 'cardano',
  'MilkADA': 'cardano',
  'USDT': 'tether',
  'DOT': 'polkadot',
  'BCH': 'bitcoin-cash',
  'UNI': 'uniswap',
  'LINK': 'chainlink',
  'USDC': 'usd-coin',
  'XLM': 'stellar',
  'AAVE': 'aave',
  'DAI': 'dai',
  'CEL': 'celsius-degree-token',
  'NEXO': 'nexo',
  'TUSD': 'true-usd',
  'GUSD': 'gemini-dollar',
  'ZEC': 'zcash',
  'DASH': 'dash'
};
const moonPayApi =
    "https://buy.moonpay.com/?apiKey=pk_live_D4M9IUMtLoDQUpSA0qQnn8VmfusvoSSQ&baseCurrencyAmount=150&baseCurrencyCode=USD";
const moonPayCurrencyCode = {
  "BTC": "BTC",
  "BNB": "BNB_BSC",
  "ETH": "ETH",
  "AVAX": "AVAX_CCHAIN",
  "MATIC": "MATIC_POLYGON",
  "DOGE": "DOGE",
  "LTC": "LTC",
  "CELO": "CELO"
};

Future<web3.DeployedContract> getEnsResolverContract(
    String cryptoDomainName, web3.Web3Client client) async {
  cryptoDomainName = cryptoDomainName.trim();
  final nameHash_ = nameHash(cryptoDomainName);

  web3.DeployedContract ensInterfaceContract = web3.DeployedContract(
    web3.ContractAbi.fromJson(ensInterface, ''),
    web3.EthereumAddress.fromHex(ensInterfaceAddress),
  );

  final resolverAddr = (await client.call(
    contract: ensInterfaceContract,
    function: ensInterfaceContract.function('resolver'),
    params: [hexToBytes(nameHash_)],
  ))
      .first;

  return web3.DeployedContract(
    web3.ContractAbi.fromJson(ensResolver, ''),
    resolverAddr,
  );
}

Future ensToContentHashAndIPFS({String cryptoDomainName}) async {
  try {
    final rpcUrl = getEVMBlockchains()['Ethereum']['rpc'];
    final client = web3.Web3Client(rpcUrl, Client());
    final nameHash_ = nameHash(cryptoDomainName);
    web3.DeployedContract ensResolverContract =
        await getEnsResolverContract(cryptoDomainName, client);
    List<int> contentHashList = (await client.call(
      contract: ensResolverContract,
      function: ensResolverContract.function('contenthash'),
      params: [hexToBytes(nameHash_)],
    ))
        .first;

    String contentHash = bytesToHex(contentHashList);

    if (!contentHash.startsWith('0x')) {
      contentHash = "0x" + contentHash;
    }
    final ipfsCIDRegex = RegExp(
        r'^0xe3010170(([0-9a-f][0-9a-f])([0-9a-f][0-9a-f])([0-9a-f]*))$');

    final swarmRegex = RegExp(r'^0xe40101fa011b20([0-9a-f]*)$');

    final match = ipfsCIDRegex.firstMatch(contentHash);
    final swarmMatch = swarmRegex.firstMatch(contentHash);
    if (match != null) {
      final length = int.parse(match.group(3), radix: 16);
      if (match.group(4).length == length * 2) {
        final userFinalDecodedHash = match.group(1);
        return {
          'success': true,
          'msg': ipfsTohttp(
            "ipfs://${bs58check.base58.encode(HEX.decode(userFinalDecodedHash))}",
          )
        };
      }
      throw Exception('invalid IPFS checksum');
    } else if (swarmMatch != null) {
      if (swarmMatch.group(1).length == (32 * 2)) {
        return {'success': true, 'msg': "bzz://" + swarmMatch.group(2)};
      }
      throw Exception('invalid SWARM checksum');
    }

    throw Exception('invalid ENS checksum');
  } catch (e) {
    return {'success': false, 'msg': 'Error resolving ens'};
  }
}

Future<Map> ensToAddress({String cryptoDomainName}) async {
  try {
    cryptoDomainName = cryptoDomainName.toLowerCase();
    final rpcUrl = getEVMBlockchains()['Ethereum']['rpc'];
    final client = web3.Web3Client(rpcUrl, Client());
    final nameHash_ = nameHash(cryptoDomainName);
    web3.DeployedContract ensResolverContract =
        await getEnsResolverContract(cryptoDomainName, client);

    final userAddress = (await client.call(
      contract: ensResolverContract,
      function: ensResolverContract.findFunctionsByName('addr').toList()[0],
      params: [hexToBytes(nameHash_)],
    ))
        .first;

    return {
      'success': true,
      'msg': web3.EthereumAddress.fromHex(userAddress.toString()).hexEip55
    };
  } catch (e) {
    return {'success': false, 'msg': 'Error resolving ens'};
  }
}

Future<bool> saveToDrive(String fileName, String mnemonic) async {
  try {
    // final pref = Hive.box(secureStorageKey);
    SSS sss = SSS();
    List<String> arr = sss.create(2, 2, mnemonic, true);
    File savedFile = await FileReader.writeFile(fileName, arr[0]);
    await GoogleDrive().upload(savedFile, fileName);
    await savedFile.delete();
    return true;
  } catch (_) {
    return false;
  }
}

Future<void> initializeAllPrivateKeys(String mnemonic) async {
  for (String i in getEVMBlockchains().keys) {
    await getEthereumFromMemnomic(
      mnemonic,
      getEVMBlockchains()[i]['coinType'],
    );
  }
  for (String i in getBitCoinPOSBlockchains().keys) {
    await getBitcoinFromMemnomic(
      mnemonic,
      getBitCoinPOSBlockchains()[i]['POSNetwork'],
    );
  }
  for (String i in getFilecoinBlockChains().keys) {
    await getFileCoinFromMemnomic(
      mnemonic,
      getFilecoinBlockChains()[i]['prefix'],
    );
  }
  for (String i in getCardanoBlockChains().keys) {
    await getCardanoFromMemnomic(
      mnemonic,
      getCardanoBlockChains()[i]['cardano_network'],
    );
  }

  await getSolanaFromMemnomic(mnemonic);
  await getStellarFromMemnomic(mnemonic);
}

Future<Map> sendCardano(Map config) async {
  final walletBuilder = cardano.WalletBuilder()
    ..networkId = config['cardanoNetwork']
    ..mnemonic = config['mnemonic'].split(' ');

  if (config['cardanoNetwork'] == cardano.NetworkId.mainnet) {
    walletBuilder.mainnetAdapterKey = config['blockfrostForCardanoApiKey'];
  } else if (config['cardanoNetwork'] == cardano.NetworkId.testnet) {
    walletBuilder.testnetAdapterKey = config['blockfrostForCardanoApiKey'];
  }
  final result = await walletBuilder.buildAndSync();
  if (result.isErr()) {
    if (kDebugMode) {
      print(result.err());
    }
    return {};
  }

  cardano.Wallet userWallet = result.unwrap();

  final coinSelection = await cardano.largestFirst(
    unspentInputsAvailable: userWallet.unspentTransactions,
    outputsRequested: [
      cardano.MultiAssetRequest.lovelace(
        config['lovelaceToSend'] + maxFeeGuessForCardano,
      )
    ],
    ownedAddresses: userWallet.addresses.toSet(),
  );

  final builder = cardano.TransactionBuilder()
    ..wallet(userWallet)
    ..blockchainAdapter(userWallet.blockchainAdapter)
    ..toAddress(config['recipientAddress'])
    ..inputs(coinSelection.unwrap().inputs)
    ..value(
      cardano.ShelleyValue(
        coin: config['lovelaceToSend'],
        multiAssets: [],
      ),
    )
    ..changeAddress(config['senderAddress']);

  final txResult = await builder.buildAndSign();

  if (txResult.isErr()) {
    if (kDebugMode) {
      print(txResult.err());
    }
    return {};
  }

  final submitTrx = await userWallet.blockchainAdapter.submitTransaction(
    txResult.unwrap().serialize,
  );

  if (submitTrx.isErr()) {
    if (kDebugMode) {
      print(submitTrx.err());
    }
    return {};
  }

  final txHash = submitTrx.unwrap();
  return {'txid': txHash.replaceAll('"', '')};
}

Future<Map> sendSolana(
  String destinationAddress,
  int lamportToSend,
  SolanaClusters solanaClustersType,
) async {
  final mnemonic = Hive.box(secureStorageKey).get(currentMmenomicKey);

  final keyPair = await compute(
      calculateSolanaKey, {mnemonicKey: mnemonic, 'getSolanaKeys': true});

  final signature = await getSolanaClient(solanaClustersType).transferLamports(
    source: keyPair,
    destination: solana.Ed25519HDPublicKey.fromBase58(destinationAddress),
    lamports: lamportToSend,
  );

  return {'txid': signature};
}

Future<Map> getSolanaFromMemnomic(String mnemonic) async {
  final pref = Hive.box(secureStorageKey);
  const keyName = 'solanaDetail';
  List mmenomicMapping = [];
  if (pref.get(keyName) != null) {
    mmenomicMapping = jsonDecode(pref.get(keyName)) as List;
    for (int i = 0; i < mmenomicMapping.length; i++) {
      if (mmenomicMapping[i]['mmenomic'] == mnemonic) {
        return mmenomicMapping[i]['key'];
      }
    }
  }

  final keys = await compute(calculateSolanaKey, {mnemonicKey: mnemonic});
  mmenomicMapping.add({'key': keys, 'mmenomic': mnemonic});
  await pref.put(keyName, jsonEncode(mmenomicMapping));
  return keys;
}

Future<Map> getCardanoFromMemnomic(
  String mnemonic,
  cardano.NetworkId cardanoNetwork,
) async {
  final pref = Hive.box(secureStorageKey);

  final keyName = 'cardanoDetail${cardanoNetwork.name}';
  List mmenomicMapping = [];
  if (pref.get(keyName) != null) {
    mmenomicMapping = jsonDecode(pref.get(keyName)) as List;

    for (int i = 0; i < mmenomicMapping.length; i++) {
      if (mmenomicMapping[i]['mmenomic'] == mnemonic) {
        return mmenomicMapping[i]['key'];
      }
    }
  }
  final keys = await compute(calculateCardanoKey, {
    mnemonicKey: mnemonic,
    'network': cardanoNetwork,
  });

  mmenomicMapping.add({'key': keys, 'mmenomic': mnemonic});

  await pref.put(keyName, jsonEncode(mmenomicMapping));
  return keys;
}

Future<Map> getStellarFromMemnomic(String mnemonic) async {
  final pref = Hive.box(secureStorageKey);
  const keyName = 'stellarDetail';
  List mmenomicMapping = [];

  if (pref.get(keyName) != null) {
    mmenomicMapping = jsonDecode(pref.get(keyName)) as List;
    for (int i = 0; i < mmenomicMapping.length; i++) {
      if (mmenomicMapping[i]['mmenomic'] == mnemonic) {
        return mmenomicMapping[i]['key'];
      }
    }
  }
  final keys = await compute(calculateStellarKey, {mnemonicKey: mnemonic});
  mmenomicMapping.add({'key': keys, 'mmenomic': mnemonic});
  await pref.put(keyName, jsonEncode(mmenomicMapping));
  return keys;
}

Future<Map> getBitcoinFromMemnomic(
  String mnemonic,
  NetworkType networkType,
) async {
  final pref = Hive.box(secureStorageKey);
  final posDetails = getBitcoinDetailsFromNetwork(networkType);

  final keyName = sha3('bitcoinDetail${posDetails['POSNetwork']}');
  List mmenomicMapping = [];
  if (pref.get(keyName) != null) {
    mmenomicMapping = jsonDecode(pref.get(keyName)) as List;
    for (int i = 0; i < mmenomicMapping.length; i++) {
      if (mmenomicMapping[i]['mmenomic'] == mnemonic) {
        return mmenomicMapping[i]['key'];
      }
    }
  }
  final keys = await compute(
    calculateBitCoinKey,
    Map.from(posDetails)..addAll({mnemonicKey: mnemonic}),
  );
  mmenomicMapping.add({'key': keys, 'mmenomic': mnemonic});
  await pref.put(keyName, jsonEncode(mmenomicMapping));
  return keys;
}

// Future<String> fileCoinAddressFromCk(String ck, String addressPrefix) async {
//   final publickKey = await Flotus.secpPrivateToPublic(ck: ck);

//   String address = await Flotus.genAddress(pk: publickKey, t: 'secp');
//   return '$addressPrefix${address.substring(1)}';
// }

Future<Map> getFileCoinFromMemnomic(
  String mnemonic,
  String addressPrefix,
) async {
  final pref = Hive.box(secureStorageKey);
  final keyName = 'fileCoinDetail$addressPrefix';
  List mmenomicMapping = [];
  if (pref.get(keyName) != null) {
    mmenomicMapping = jsonDecode(pref.get(keyName)) as List;
    for (int i = 0; i < mmenomicMapping.length; i++) {
      if (mmenomicMapping[i]['mmenomic'] == mnemonic) {
        return mmenomicMapping[i]['key'];
      }
    }
  }
  final keys = await compute(calculateFileCoinKey, mnemonic);

  // String address = await fileCoinAddressFromCk(
  //   keys['ck'],
  //   addressPrefix,
  // );

  // keys['address'] = address;
  // mmenomicMapping.add({'key': keys, 'mmenomic': mnemonic});
  // await pref.put(keyName, jsonEncode(mmenomicMapping));
  return keys;
}

Map calculateBitCoinKey(Map config) {
  final seed = bip39.mnemonicToSeed(config[mnemonicKey]);
  final root = bip32.BIP32.fromSeed(seed);
  final node = root.derivePath(config['derivationPath']);

  String address;
  if (config['P2WPKH']) {
    address = P2WPKH(
      data: PaymentData(
        pubkey: node.publicKey,
      ),
      network: config['POSNetwork'],
    ).data.address;
  } else {
    address = P2PKH(
      data: PaymentData(
        pubkey: node.publicKey,
      ),
      network: config['POSNetwork'],
    ).data.address;
  }

  if (sha3(config['POSNetwork'].toString()) == sha3(zcash.toString())) {
    final baddr = [...bs58check.decode(address)];
    baddr.removeAt(0);

    final taddr = Uint8List(22);

    taddr.setAll(2, baddr);
    taddr.setAll(0, [0x1c, 0xb8]);

    address = bs58check.encode(taddr);
  }

  return {
    'address': address,
    'private_key': "0x${HEX.encode(node.privateKey)}"
  };
}

Map calculateFileCoinKey(String mnemonic) {
  final seed = bip39.mnemonicToSeed(mnemonic);
  final root = bip32.BIP32.fromSeed(seed);
  final node = root.derivePath("m/44'/461'/0'/0");
  final rs0 = node.derive(0);
  final ck = base64Encode(rs0.privateKey);

  return {"ck": ck};
}

String calculateEthereumKey(Map config) {
  final seed = bip39.mnemonicToSeed(config[mnemonicKey]);
  final root = bip32.BIP32.fromSeed(seed);
  return "0x${HEX.encode(root.derivePath("m/44'/${config['coinType']}'/0'/0/0").privateKey)}";
}

Future calculateSolanaKey(Map config) async {
  final List<int> seed = bip39.mnemonicToSeed(config[mnemonicKey]);

  final solana.Ed25519HDKeyPair keyPair =
      await solana.Ed25519HDKeyPair.fromSeedWithHdPath(
    seed: seed,
    hdPath: "m/44'/501'/0'",
  );

  if (config['getSolanaKeys'] != null && config['getSolanaKeys'] == true) {
    return keyPair;
  }

  return {
    'address': keyPair.address,
  };
}

Map calculateCardanoKey(Map config) {
  final wallet = cardano.HdWallet.fromMnemonic(config[mnemonicKey]);
  const cardanoAccountHardOffsetKey = 0x80000000;

  String userWalletAddress = wallet
      .deriveUnusedBaseAddressKit(
          networkId: config['network'],
          index: 0,
          account: cardanoAccountHardOffsetKey,
          role: 0,
          unusedCallback: (cardano.ShelleyAddress address) => true)
      .address
      .toString();

  return {
    'address': userWalletAddress,
  };
}

Future<Map> calculateStellarKey(Map config) async {
  final wallet = await stellar.Wallet.from(config[mnemonicKey]);
  final userWalletAddress = await wallet.getKeyPair(index: 0);
  return {
    'address': userWalletAddress.accountId,
    'private_key': userWalletAddress.secretSeed,
  };
}

Future<double> getCardanoAddressBalance(
  String address,
  cardano.NetworkId cardanoNetwork,
  String blockfrostForCardanoApiKey, {
  bool skipNetworkRequest = false,
}) async {
  final pref = Hive.box(secureStorageKey);
  final key = 'cardanoAddressBalance$address';

  final storedBalance = pref.get(key);

  double savedBalance = 0;

  if (storedBalance != null) {
    savedBalance = storedBalance;
  }

  if (skipNetworkRequest) return savedBalance;

  try {
    final cardanoBlockfrostBaseUrl =
        'https://cardano-${cardanoNetwork == cardano.NetworkId.mainnet ? 'mainnet' : 'preprod'}.blockfrost.io/api/v0/addresses/';
    final request = await get(
      Uri.parse('$cardanoBlockfrostBaseUrl$address'),
      headers: {'project_id': blockfrostForCardanoApiKey},
    );

    if (request.statusCode ~/ 100 == 4 || request.statusCode ~/ 100 == 5) {
      throw Exception('Request failed');
    }
    Map decodedData = jsonDecode(request.body);
    final balance = (decodedData['amount'] as List)
        .where((element) => element['unit'] == 'lovelace')
        .toList()[0]['quantity'];

    final balanceFromAdaToLoveLace =
        (BigInt.parse(balance) / BigInt.from(pow(10, cardanoDecimals)))
            .toDouble();
    await pref.put(key, balanceFromAdaToLoveLace);

    return balanceFromAdaToLoveLace;
  } catch (e) {
    return savedBalance;
  }
}

Future<double> getStellarAddressBalance(
  String address,
  stellar.StellarSDK sdk,
  stellar.Network cluster, {
  bool skipNetworkRequest = false,
}) async {
  final pref = Hive.box(secureStorageKey);

  final key = 'stellarAddressBalance$address${bytesToHex(cluster.networkId)}';

  final storedBalance = pref.get(key);

  double savedBalance = 0;

  if (storedBalance != null) {
    savedBalance = storedBalance;
  }

  if (skipNetworkRequest) return savedBalance;

  try {
    stellar.AccountResponse account = await sdk.accounts.account(address);

    for (stellar.Balance balance in account.balances) {
      if (balance.assetType == stellar.Asset.TYPE_NATIVE) {
        double balanceInStellar = double.parse(balance.balance);
        await pref.put(key, balanceInStellar);
        return balanceInStellar;
      }
    }
    return 0;
  } catch (e) {
    return savedBalance;
  }
}

Future<double> getSolanaAddressBalance(
  String address,
  SolanaClusters solanaClusterType, {
  bool skipNetworkRequest = false,
}) async {
  final pref = Hive.box(secureStorageKey);

  final key = 'solanaAddressBalance$address${solanaClusterType.toString()}';

  final storedBalance = pref.get(key);

  double savedBalance = 0;

  if (storedBalance != null) {
    savedBalance = storedBalance;
  }

  if (skipNetworkRequest) return savedBalance;

  try {
    final balanceInLamport =
        await getSolanaClient(solanaClusterType).rpcClient.getBalance(address);
    double balanceInSol = balanceInLamport / solana.lamportsPerSol;

    await pref.put(key, balanceInSol);

    return balanceInSol;
  } catch (e) {
    return savedBalance;
  }
}

Future<double> getFileCoinAddressBalance(
  String address, {
  String baseUrl,
  bool skipNetworkRequest = false,
}) async {
  final pref = Hive.box(secureStorageKey);
  final key = 'fileCoinAddressBalance$address$baseUrl';

  final storedBalance = pref.get(key);

  double savedBalance = 0;

  if (storedBalance != null) {
    savedBalance = storedBalance;
  }

  if (skipNetworkRequest) return savedBalance;

  try {
    final response = await http.get(Uri.parse(
        '$baseUrl/actor/balance?actor=${Uri.encodeQueryComponent(address)}'));
    final responseBody = response.body;
    if (response.statusCode ~/ 100 == 4 || response.statusCode ~/ 100 == 5) {
      throw Exception(responseBody);
    }

    double balanceInFileCoin = double.parse(
          jsonDecode(responseBody)['data']['balance'].toString(),
        ) /
        pow(10, fileCoinDecimals);

    await pref.put(key, balanceInFileCoin);

    return balanceInFileCoin;
  } catch (e) {
    return savedBalance;
  }
}

Future<String> getCryptoPrice({
  bool skipNetworkRequest = false,
}) async {
  String allCrypto = "";
  int currentIndex = 0;
  final listOfCoinGeckoValue = coinGeckCryptoSymbolToID.values;
  for (final value in listOfCoinGeckoValue) {
    if (currentIndex == listOfCoinGeckoValue.length - 1) {
      allCrypto += value;
    } else {
      allCrypto += value + ",";
    }
    currentIndex++;
  }
  const secondsToResendRequest = 15;

  final pref = Hive.box(secureStorageKey);
  final savedCryptoPrice = pref.get(coinGeckoCryptoPriceKey);

  if (savedCryptoPrice != null) {
    DateTime now = DateTime.now();
    final nowInSeconds = now.difference(MyApp.lastcoinGeckoData).inSeconds;

    final useCachedResponse = nowInSeconds < secondsToResendRequest;

    if (nowInSeconds > secondsToResendRequest) {
      MyApp.lastcoinGeckoData = DateTime.now();
      MyApp.getCoinGeckoData = true;
    }

    if (useCachedResponse || skipNetworkRequest) {
      return json.decode(savedCryptoPrice)['data'];
    }
  }

  try {
    String defaultCurrency = pref.get('defaultCurrency') ?? "usd";
    if (!MyApp.getCoinGeckoData) {
      return json.decode(savedCryptoPrice)['data'];
    }
    MyApp.getCoinGeckoData = false;
    MyApp.lastcoinGeckoData = DateTime.now();

    final dataUrl =
        'https://api.coingecko.com/api/v3/simple/price?ids=$allCrypto&vs_currencies=$defaultCurrency&include_24hr_change=true';

    final response =
        await get(Uri.parse(dataUrl)).timeout(networkTimeOutDuration);

    final responseBody = response.body;
    // check for http status code of 4** or 5**
    if (response.statusCode ~/ 100 == 4 || response.statusCode ~/ 100 == 5) {
      throw Exception(responseBody);
    }

    await pref.put(
      coinGeckoCryptoPriceKey,
      json.encode(
        {
          'data': responseBody,
        },
      ),
    );

    return responseBody;
  } catch (e) {
    if (savedCryptoPrice != null) {
      return json.decode(savedCryptoPrice)['data'];
    }
    return null;
  }
}

Future<String> contractDetailsKey(String rpc, String contractAddress) async {
  String mnemonic = Hive.box(secureStorageKey).get(currentMmenomicKey);
  final ethereumDetails = await getEthereumFromMemnomic(
    mnemonic,
    getEVMBlockchains()['Ethereum']['coinType'],
  );
  return '${rpc.toString().toLowerCase()}${contractAddress.toString().toLowerCase()}${ethereumDetails['eth_wallet_address']}|usertoken';
}

Future getErc20Allowance({
  String owner,
  String spender,
  String rpc,
  String contractAddress,
}) async {
  web3.Web3Client client = web3.Web3Client(
    rpc,
    Client(),
  );

  final contract = web3.DeployedContract(
      web3.ContractAbi.fromJson(erc20Abi, ''),
      web3.EthereumAddress.fromHex(contractAddress));

  final allowanceFunction = contract.function('allowance');

  final allowance = (await client
          .call(contract: contract, function: allowanceFunction, params: [
    web3.EthereumAddress.fromHex(owner),
    web3.EthereumAddress.fromHex(spender),
  ]))
      .first;

  return allowance;
}

Future<double> getERC20TokenBalance(
  Map tokenDetails, {
  bool skipNetworkRequest = false,
}) async {
  web3.Web3Client client = web3.Web3Client(
    tokenDetails['rpc'],
    Client(),
  );

  final pref = Hive.box(secureStorageKey);
  String mnemonic = pref.get(currentMmenomicKey);
  Map response = await getEthereumFromMemnomic(
    mnemonic,
    tokenDetails['coinType'],
  );

  final sendingAddress = web3.EthereumAddress.fromHex(
    response['eth_wallet_address'],
  );
  String elementDetailsKey = await contractDetailsKey(
    tokenDetails['rpc'],
    tokenDetails['contractAddress'],
  );

  String balanceKey = sha3('${elementDetailsKey}balance');

  final storedBalance = pref.get(balanceKey);

  double savedBalance = 0;

  if (storedBalance != null) {
    final crytoBalance = jsonDecode(pref.get(balanceKey));
    savedBalance = double.parse(crytoBalance['balance']) /
        pow(10, double.parse(crytoBalance['decimals']));
  }

  if (skipNetworkRequest) return savedBalance;

  try {
    final contract = web3.DeployedContract(
      web3.ContractAbi.fromJson(erc20Abi, ''),
      web3.EthereumAddress.fromHex(
        tokenDetails['contractAddress'],
      ),
    );

    final balanceFunction = contract.function('balanceOf');

    final decimalsFunction = contract.function('decimals');

    final decimals = (await client
            .call(contract: contract, function: decimalsFunction, params: []))
        .first
        .toString();

    final balance = (await client.call(
      contract: contract,
      function: balanceFunction,
      params: [sendingAddress],
    ))
        .first
        .toString();
    await pref.put(
      balanceKey,
      jsonEncode({
        'balance': balance,
        'decimals': decimals,
      }),
    );
    return double.parse(balance) / pow(10, double.parse(decimals));
  } catch (e) {
    return savedBalance;
  }
}

Future<double> getEtherTransactionFee(
  String rpc,
  Uint8List data,
  web3.EthereumAddress sender,
  web3.EthereumAddress to, {
  double value,
  EtherAmount gasPrice,
}) async {
  final client = web3.Web3Client(
    rpc,
    Client(),
  );

  final etherValue = value != null
      ? web3.EtherAmount.inWei(
          BigInt.from(value),
        )
      : null;

  if (gasPrice == null || gasPrice.getInWei == BigInt.from(0)) {
    gasPrice = await client.getGasPrice();
  }
  BigInt gasUnit;

  try {
    gasUnit = await client.estimateGas(
      sender: sender,
      to: to,
      data: data,
      value: etherValue,
    );
  } catch (_) {}

  if (gasUnit == null) {
    try {
      gasUnit = await client.estimateGas(
        sender: EthereumAddress.fromHex(zeroAddress),
        to: to,
        data: data,
        value: etherValue,
      );
    } catch (_) {}
  }

  if (gasUnit == null) {
    try {
      gasUnit = await client.estimateGas(
        sender: EthereumAddress.fromHex(deadAddress),
        to: to,
        data: data,
        value: etherValue,
      );
    } catch (e) {
      gasUnit = BigInt.from(0);
    }
  }
  return gasPrice.getInWei.toDouble() * gasUnit.toDouble();
}

Future<String> etherPrivateKeyToAddress(String privateKey) async {
  web3.EthPrivateKey ethereumPrivateKey =
      web3.EthPrivateKey.fromHex(privateKey);
  final uncheckedSumAddress = await ethereumPrivateKey.extractAddress();
  return web3.EthereumAddress.fromHex(uncheckedSumAddress.toString()).hexEip55;
}

Future<Map> getEthereumFromMemnomic(
  String mnemonic,
  int coinType,
) async {
  String key = 'ethereumDetails$coinType';

  final pref = Hive.box(secureStorageKey);
  List mmenomicMapping = [];
  if (pref.get(key) != null) {
    mmenomicMapping = jsonDecode(pref.get(key)) as List;
    for (int i = 0; i < mmenomicMapping.length; i++) {
      if (mmenomicMapping[i]['mmenomic'] == mnemonic) {
        return mmenomicMapping[i]['key'];
      }
    }
  }
  final privatekeyStr = await compute(
    calculateEthereumKey,
    {mnemonicKey: mnemonic, 'coinType': coinType},
  );

  final address = await etherPrivateKeyToAddress(privatekeyStr);

  final keys = {
    'eth_wallet_address': address,
    'eth_wallet_privateKey': privatekeyStr,
    mnemonicKey: mnemonic
  };
  mmenomicMapping.add({'key': keys, 'mmenomic': mnemonic});
  await pref.put(key, jsonEncode(mmenomicMapping));
  return keys;
}

Future<String> getCurrencyJson() async {
  return await rootBundle.loadString('json/currencies.json');
}

Future<double> totalCryptoBalance({
  String mnemonic,
  Map allCryptoPrice,
  String defaultCurrency,
  bool skipNetworkRequest = false,
}) async {
  double totalBalance = 0.0;

  for (String i in getBitCoinPOSBlockchains().keys) {
    final Map posBlockchain = getBitCoinPOSBlockchains()[i];
    double posBlockChainPrice =
        (allCryptoPrice[coinGeckCryptoSymbolToID[posBlockchain['symbol']]]
                [defaultCurrency.toLowerCase()] as num)
            .toDouble();

    final getBitcoinDetails = await getBitcoinFromMemnomic(
      mnemonic,
      posBlockchain['POSNetwork'],
    );

    double posBlockChainBalance = await getBitcoinAddressBalance(
      getBitcoinDetails['address'],
      posBlockchain['POSNetwork'],
      skipNetworkRequest: skipNetworkRequest,
    );

    totalBalance += posBlockChainPrice * posBlockChainBalance;
  }

  for (String i in getEVMBlockchains().keys) {
    final Map evmBlockchain = getEVMBlockchains()[i];
    final cryptoEVMCompPrice =
        (allCryptoPrice[coinGeckCryptoSymbolToID[evmBlockchain['symbol']]]
                [defaultCurrency.toLowerCase()] as num)
            .toDouble();
    final getEthereumDetails = await getEthereumFromMemnomic(
      mnemonic,
      evmBlockchain['coinType'],
    );
    final cryptoEVMCompBalance = await getEthereumAddressBalance(
      getEthereumDetails['eth_wallet_address'],
      evmBlockchain['rpc'],
      coinType: evmBlockchain['coinType'],
      skipNetworkRequest: skipNetworkRequest,
    );

    totalBalance += cryptoEVMCompBalance * cryptoEVMCompPrice;
  }
  for (String i in getStellarBlockChains().keys) {
    final Map stellarBlockchain = getStellarBlockChains()[i];
    final cryptoStellarCompPrice =
        (allCryptoPrice[coinGeckCryptoSymbolToID[stellarBlockchain['symbol']]]
                [defaultCurrency.toLowerCase()] as num)
            .toDouble();
    final getStellarDetails = await getStellarFromMemnomic(mnemonic);
    final cryptoStellarBalance = await getStellarAddressBalance(
      getStellarDetails['address'],
      stellarBlockchain['sdk'],
      stellarBlockchain['cluster'],
      skipNetworkRequest: skipNetworkRequest,
    );

    totalBalance += cryptoStellarBalance * cryptoStellarCompPrice;
  }
  for (String i in getSolanaBlockChains().keys) {
    final Map solanaBlockchain = getSolanaBlockChains()[i];
    final solanaPrice =
        (allCryptoPrice[coinGeckCryptoSymbolToID[solanaBlockchain['symbol']]]
                [defaultCurrency.toLowerCase()] as num)
            .toDouble();
    final getSolanaDetails = await getSolanaFromMemnomic(mnemonic);
    final solanaBalance = await getSolanaAddressBalance(
      getSolanaDetails['address'],
      solanaBlockchain['solanaCluster'],
      skipNetworkRequest: skipNetworkRequest,
    );

    totalBalance += solanaBalance * solanaPrice;
  }

  for (String i in getFilecoinBlockChains().keys) {
    final Map filecoinBlockchain = getFilecoinBlockChains()[i];
    final filecoinPrice =
        (allCryptoPrice[coinGeckCryptoSymbolToID[filecoinBlockchain['symbol']]]
                [defaultCurrency.toLowerCase()] as num)
            .toDouble();
    final getFileCoinDetails = await getFileCoinFromMemnomic(
      mnemonic,
      filecoinBlockchain['prefix'],
    );

    final filecoinBalance = await getFileCoinAddressBalance(
      getFileCoinDetails['address'],
      baseUrl: filecoinBlockchain['baseUrl'],
      skipNetworkRequest: skipNetworkRequest,
    );

    totalBalance += filecoinBalance * filecoinPrice;
  }

  for (String i in getCardanoBlockChains().keys) {
    final Map cardanoBlockchain = getCardanoBlockChains()[i];
    final getCardanoDetails = await getCardanoFromMemnomic(
      mnemonic,
      cardanoBlockchain['cardano_network'],
    );

    final cardanoBalance = await getCardanoAddressBalance(
      getCardanoDetails['address'],
      cardanoBlockchain['cardano_network'],
      cardanoBlockchain['blockFrostKey'],
      skipNetworkRequest: skipNetworkRequest,
    );

    final cardanoPrice =
        (allCryptoPrice[coinGeckCryptoSymbolToID[cardanoBlockchain['symbol']]]
                [defaultCurrency.toLowerCase()] as num)
            .toDouble();

    totalBalance += cardanoBalance * cardanoPrice;
  }

  return totalBalance;
}

Future<String> upload(
  File imageFile,
  String imagefileName,
  MediaType imageMediaType,
  String uploadURL,
  Map fieldsMap,
) async {
  try {
    final stream = http.ByteStream(imageFile.openRead())..cast();
    final length = await imageFile.length();

    final uri = Uri.parse(uploadURL);

    final request = http.MultipartRequest("POST", uri);
    for (final key in fieldsMap.keys) {
      request.fields[key] = fieldsMap[key];
    }

    final multipartFile = http.MultipartFile(imagefileName, stream, length,
        filename: basename(imageFile.path), contentType: imageMediaType);

    request.files.add(multipartFile);
    StreamedResponse response = await request.send();
    Uint8List responseData = await response.stream.toBytes();
    String responseBody = String.fromCharCodes(responseData);

    if (response.statusCode ~/ 100 == 4 || response.statusCode ~/ 100 == 5) {
      if (kDebugMode) {
        print(responseBody);
      }
      throw Exception(responseBody);
    }
    return responseBody;
  } catch (e) {
    if (kDebugMode) {
      print(e.toString());
    }
    return null;
  }
}

Uri blockChainToHttps(String value) {
  if (value == null) return Uri.parse(walletURL);

  value = value.trim();
  if (value.startsWith('ipfs://')) return Uri.parse(ipfsTohttp(value));

  if (isURL(value)) return Uri.parse(value).replace(scheme: 'https');

  return Uri.parse('https://www.google.com/search?q=$value');
}

Future<double> getEthereumAddressBalance(
  String address,
  String rpcUrl, {
  int coinType,
  bool skipNetworkRequest = false,
}) async {
  final preferencesInst = Hive.box(secureStorageKey);

  final tokenKey = '$rpcUrl$address/balance';
  final storedBalance = preferencesInst.get(tokenKey);

  double savedBalance = 0;

  if (storedBalance != null) {
    savedBalance = storedBalance;
  }

  if (skipNetworkRequest) return savedBalance;

  try {
    final httpClient = Client();
    final ethClient = Web3Client(rpcUrl, httpClient);

    double ethBalance =
        (await ethClient.getBalance(EthereumAddress.fromHex(address)))
                .getInWei
                .toDouble() /
            pow(10, etherDecimals);

    preferencesInst.put(tokenKey, ethBalance);

    return ethBalance;
  } catch (e) {
    return savedBalance;
  }
}

Future<Map> swapTokenOnUniswap(
    {sellAmountInWei, sellContractAddr, buyContractAddr}) async {
  try {
    final url =
        'https://airdrop-api.matic.network/swap-0x-gated?buyToken=$buyContractAddr&sellAmount=$sellAmountInWei&sellToken=$sellContractAddr&slippagePercentage=10';

    final response =
        await http.get(Uri.parse(url)).timeout(networkTimeOutDuration);
    // check response code is not 4** or 5**
    if (response.statusCode ~/ 100 == 4 || response.statusCode ~/ 100 == 5) {
      throw Exception('Failed to swap token');
    }
    final jsonResponse = json.decode(response.body)['result'];
    return jsonResponse;
  } catch (e) {
    return {};
  }
}

Future<Map> get1InchUrlList(int chainId) async {
  final response = await http
      .get(Uri.parse('https://tokens.1inch.io/v1.1/$chainId'))
      .timeout(networkTimeOutDuration);

  Map jsonResponse = {};

  if (chainId == 137) {
    jsonResponse.addAll({
      '0x2EC8b30B568AE06cC0087338865472c26FfE8E16': {
        'symbol': 'ECLA',
        'name': 'ECLA Ecosystem',
        'decimals': '18',
        'address': '0x2EC8b30B568AE06cC0087338865472c26FfE8E16',
        'logoURI':
            'https://peppy-sawine-4d9328.netlify.app/assets/EcosysImages/eclaLogo3.png',
        'tags': ['tokens']
      }
    });
  }

  jsonResponse.addAll(Map.from(json.decode(response.body)));
  return jsonResponse;
}

const uniswapTokenList = 'https://tokens.uniswap.org/';

Future getUniswapPolygonTokens() async {
  try {
    Response response = await http
        .get(Uri.parse(uniswapTokenList))
        .timeout(networkTimeOutDuration);
    List jsonResponse = json.decode(response.body)["tokens"];
    List polygonTokens =
        jsonResponse.where((token) => token["chainId"] == 137).toList();
    Map firstElement = polygonTokens[0];
    polygonTokens[0] = {
      "chainId": "137",
      "address": "0x2EC8b30B568AE06cC0087338865472c26FfE8E16",
      "name": " ECLA Ecosystem",
      "symbol": "ECLA",
      "decimals": 18,
      "logoURI":
          "https://peppy-sawine-4d9328.netlify.app/assets/EcosysImages/eclaLogo3.png",
      "extensions": {
        "bridgeInfo": {
          "137": {"tokenAddress": "0x0621d647cecbFb64b79E44302c1933cB4f27054d"}
        }
      }
    };
    polygonTokens.add(firstElement);
    return polygonTokens;
  } catch (e) {
    return [];
  }
}

Map getEthereumDetailsFromChainId(int chainId) {
  List blockChains = getEVMBlockchains().values.toList();
  for (int i = 0; i < blockChains.length; i++) {
    if (blockChains[i]['chainId'] == chainId) {
      return blockChains[i];
    }
  }
  return null;
}

getBitcoinDetailsFromNetwork(NetworkType network) {
  List blockChains = getBitCoinPOSBlockchains().values.toList();
  for (int i = 0; i < blockChains.length; i++) {
    if (blockChains[i]['POSNetwork'] == network) {
      return blockChains[i];
    }
  }
  return null;
}

showDialogWithMessage({
  BuildContext context,
  String message,
  Function onConfirm,
}) {
  AwesomeDialog(
    closeIcon: const Icon(
      Icons.close,
    ),
    buttonsTextStyle: const TextStyle(color: Colors.white),
    context: context,
    btnOkColor: appBackgroundblue,
    dialogType: DialogType.INFO,
    buttonsBorderRadius: const BorderRadius.all(Radius.circular(10)),
    headerAnimationLoop: false,
    animType: AnimType.BOTTOMSLIDE,
    title: AppLocalizations.of(context).info,
    desc: message,
    showCloseIcon: true,
    btnOkText: AppLocalizations.of(context).ok,
    btnOkOnPress: onConfirm ?? () {},
  ).show();
}

showBlockChainDialog({
  Function onTap,
  BuildContext context,
  int selectedChainId,
}) {
  final ethEnabledBlockChain = <Widget>[];
  for (String i in getEVMBlockchains().keys) {
    Map blockChainDetails = getEVMBlockchains()[i];
    bool isSelected = false;
    if (selectedChainId != null &&
        blockChainDetails['chainId'] == selectedChainId) {
      isSelected = true;
    }

    ethEnabledBlockChain.add(
      InkWell(
        onTap: () {
          blockChainDetails['name'] = i;
          onTap(blockChainDetails);
        },
        child: buildRow(
          blockChainDetails['image'],
          i,
          isSelected: isSelected,
        ),
      ),
    );
  }
  slideUpPanel(
    context,
    Container(
      color: Colors.transparent,
      child: ListView(
        shrinkWrap: true,
        children: <Widget>[
          const SizedBox(height: 20),
          Center(
            child: Text(AppLocalizations.of(context).selectBlockchains),
          ),
          const SizedBox(height: 20),
          ...ethEnabledBlockChain,
          const SizedBox(height: 20),
        ],
      ),
    ),
  );
}

changeBlockChainAndReturnInit(
  int coinType,
  int chainId,
  String rpc,
) async {
  final init = await rootBundle.loadString('dappBrowser/init.js');
  final pref = Hive.box(secureStorageKey);
  final mnemonic = pref.get(currentMmenomicKey);
  final response = await getEthereumFromMemnomic(mnemonic, coinType);

  final sendingAddress = response['eth_wallet_address'];
  await pref.put(dappChainIdKey, chainId);

  return init
      .replaceFirst("%1\$s", sendingAddress)
      .replaceFirst("%2\$s", rpc)
      .replaceFirst(
        "%3\$s",
        chainId.toString(),
      );
}

Future<Widget> dappWidget(
  BuildContext context,
  String data,
) async {
  final provider =
      await rootBundle.loadString('dappBrowser/alphawallet.min.js');

  final pref = Hive.box(secureStorageKey);
  if (pref.get(dappChainIdKey) == null) {
    await pref.put(
      dappChainIdKey,
      getEVMBlockchains()[tokenContractNetwork]['chainId'],
    );
  }

  int chainId = pref.get(dappChainIdKey);
  final rpc = getEthereumDetailsFromChainId(chainId)['rpc'];

  final init = await changeBlockChainAndReturnInit(
    getEthereumDetailsFromChainId(chainId)['coinType'],
    chainId,
    rpc,
  );

  return Dapp(
    provider: provider,
    init: init,
    data: data,
  );
}

switchEthereumChain({
  context,
  switchChainIdData,
  currentChainIdData,
  onConfirm,
  onReject,
}) async {
  await slideUpPanel(
    context,
    Padding(
      padding: const EdgeInsets.all(25),
      child: Column(
        mainAxisSize: MainAxisSize.min,
        children: [
          Text(
            AppLocalizations.of(context).switchChainRequest,
            style: const TextStyle(
              fontWeight: FontWeight.bold,
              fontSize: 20,
            ),
          ),
          const SizedBox(
            height: 20,
          ),
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              CircleAvatar(
                backgroundColor: Theme.of(context).backgroundColor,
                backgroundImage: AssetImage(
                  currentChainIdData['image'],
                ),
              ),
              const Icon(
                Icons.arrow_right_alt_outlined,
              ),
              CircleAvatar(
                backgroundColor: Theme.of(context).backgroundColor,
                backgroundImage: AssetImage(
                  switchChainIdData['image'],
                ),
              ),
            ],
          ),
          const SizedBox(
            height: 20,
          ),
          Text(
            AppLocalizations.of(context).switchChainIdMessage(
              switchChainIdData['symbol'],
              switchChainIdData['chainId'],
            ),
            textAlign: TextAlign.center,
            style: const TextStyle(fontSize: 16),
          ),
          const SizedBox(
            height: 20,
          ),
          Row(
            children: [
              Expanded(
                child: TextButton(
                  style: TextButton.styleFrom(
                    foregroundColor: Colors.white,
                    backgroundColor: const Color(0xff007bff),
                  ),
                  onPressed: onConfirm,
                  child: Text(
                    AppLocalizations.of(context).confirm,
                    style: const TextStyle(
                      fontWeight: FontWeight.bold,
                      fontSize: 18.0,
                    ),
                  ),
                ),
              ),
              const SizedBox(width: 16.0),
              Expanded(
                child: TextButton(
                  style: TextButton.styleFrom(
                    foregroundColor: Colors.white,
                    backgroundColor: const Color(0xff007bff),
                  ),
                  onPressed: onReject,
                  child: Text(
                    AppLocalizations.of(context).reject,
                    style: const TextStyle(
                      fontWeight: FontWeight.bold,
                      fontSize: 18.0,
                    ),
                  ),
                ),
              ),
            ],
          ),
        ],
      ),
    ),
  );
}

signTransaction({
  Function onReject,
  String gasPriceInWei_,
  BuildContext context,
  Function onConfirm,
  String valueInWei_,
  String gasInWei_,
  String txData,
  String from,
  String to,
  String networkIcon,
  String name,
  String blockChainCurrencySymbol,
  String title,
  int chainId,
}) async {
  final rpc = getEthereumDetailsFromChainId(chainId)['rpc'];
  final _wcClient = web3.Web3Client(
    rpc,
    Client(),
  );

  double value = valueInWei_ == null ? 0 : BigInt.parse(valueInWei_).toDouble();

  double gasPrice =
      gasPriceInWei_ == null ? 0 : BigInt.parse(gasPriceInWei_).toDouble();
  txData ??= '0x';

  double userBalance = 0;

  Uint8List trxDataList = txDataToUintList(txData);
  double transactionFee = 0;
  String message = '';

  final Map decodedFunction = await decodeAbi(txData);

  final String decodedName =
      decodedFunction == null ? null : decodedFunction['name'];
  String methodId;
  Map decodedParams = {};

  if (decodedFunction != null) {
    methodId = decodedFunction['name'];
    final List params = decodedFunction['params'];

    for (int i = 0; i < params.length; i++) {
      if (i == 0) {
        methodId += "(";
      }
      methodId += params[i]['type'].toString();

      if (i == params.length - 1) {
        methodId += ")";
      } else {
        methodId += ",";
      }
      decodedParams[params[i]['name'].toString()] =
          params[i]['value'].toString();
    }
  }

  String info = AppLocalizations.of(context).info;

  ValueNotifier<bool> isSigningTransaction = ValueNotifier(false);
  slideUpPanel(
    context,
    DefaultTabController(
      length: 3,
      child: Column(
        children: <Widget>[
          Container(
            alignment: Alignment.center,
            padding: const EdgeInsets.only(bottom: 8.0),
            child: Row(
              mainAxisAlignment: MainAxisAlignment.spaceBetween,
              children: [
                const Align(
                  alignment: Alignment.centerRight,
                  child: IconButton(
                    onPressed: null,
                    icon: Icon(
                      Icons.close,
                      color: Colors.transparent,
                    ),
                  ),
                ),
                Text(
                  AppLocalizations.of(context).signTransaction,
                  style: const TextStyle(
                    fontWeight: FontWeight.bold,
                    fontSize: 20.0,
                  ),
                ),
                Align(
                  alignment: Alignment.centerRight,
                  child: IconButton(
                    onPressed: () {
                      if (Navigator.canPop(context)) {
                        onReject();
                      }
                    },
                    icon: const Icon(Icons.close),
                  ),
                ),
              ],
            ),
          ),

          const SizedBox(
            height: 50,
            child: TabBar(
              tabs: [
                Tab(
                    icon: Text(
                  "Details",
                  style: TextStyle(
                    fontSize: 18,
                    fontWeight: FontWeight.w500,
                  ),
                )),
                Tab(
                    icon: Text(
                  "Data",
                  style: TextStyle(
                    fontSize: 18,
                    fontWeight: FontWeight.w500,
                  ),
                )),
                Tab(
                    icon: Text(
                  "Hex",
                  style: TextStyle(
                    fontSize: 18,
                    fontWeight: FontWeight.w500,
                  ),
                )),
              ],
            ),
          ),

          // create widgets for each tab bar here
          Expanded(
            child: TabBarView(
              children: [
                // first tab bar view widget
                FutureBuilder(future: () async {
                  userBalance = (await _wcClient
                          .getBalance(EthereumAddress.fromHex(from)))
                      .getInWei
                      .toDouble();

                  transactionFee = await getEtherTransactionFee(
                    rpc,
                    trxDataList,
                    web3.EthereumAddress.fromHex(from),
                    web3.EthereumAddress.fromHex(to),
                    value: value,
                    gasPrice: web3.EtherAmount.inWei(
                      BigInt.from(
                        gasPrice,
                      ),
                    ),
                  );
                  if (decodedFunction == null) return true;

                  final List params = decodedFunction['params'];

                  if (decodedName == 'safeBatchTransferFrom') {
                    List nftAmount = [];
                    for (int i = 0; i < params.length; i++) {
                      if (params[i]['name'] == 'amounts') {
                        nftAmount = params[i]['value'];
                        break;
                      }
                    }
                    int totalAmount = 0;
                    for (var amount in nftAmount) {
                      totalAmount += int.parse(amount);
                    }
                    message =
                        "$totalAmount NFT${totalAmount > 1 ? "s" : ""} would be sent out.";
                  } else if (decodedName == 'safeTransferFrom') {
                    String spender;
                    String from_;
                    String tokenId;
                    for (int i = 0; i < params.length; i++) {
                      if (params[i]['name'] == 'from') {
                        from_ = params[i]['value'].toString();
                      }
                      if (params[i]['name'] == 'to') {
                        spender = params[i]['value'].toString();
                      }
                      if (params[i]['name'] == 'tokenId') {
                        tokenId = params[i]['value'];
                      }
                      if (params[i]['name'] == 'id') {
                        tokenId = params[i]['value'];
                      }
                    }
                    message =
                        "Transfer NFT $tokenId ($to) from $from_ to $spender";
                  } else if (decodedName == 'approve' ||
                      decodedName == 'transfer' ||
                      decodedName == 'transferFrom') {
                    String spender;
                    double token;
                    String from_;
                    Map tokenDetails = await getERC20TokenDetails(
                      contractAddress: to,
                      rpc: getEthereumDetailsFromChainId(chainId)['rpc'],
                    );

                    final decimals = tokenDetails['decimals'];
                    for (int i = 0; i < params.length; i++) {
                      if (params[i]['name'] == 'spender') {
                        spender = params[i]['value'].toString();
                      }
                      if (params[i]['name'] == 'to') {
                        spender = params[i]['value'].toString();
                      }
                      if (params[i]['name'] == 'tokens') {
                        token = BigInt.parse(params[i]['value']) /
                            BigInt.from(pow(10, int.parse(decimals)));
                      }
                      if (params[i]['name'] == 'amount') {
                        token = BigInt.parse(params[i]['value']) /
                            BigInt.from(pow(10, int.parse(decimals)));
                      }
                      if (params[i]['name'] == 'from') {
                        from_ = params[i]['value'];
                      }
                    }
                    if (decodedName == "approve") {
                      message =
                          "Allow $spender to spend $token ${tokenDetails['symbol']} ($to)";
                    } else if (decodedName == 'transfer') {
                      message =
                          "Transfer $token ${tokenDetails['symbol']} ($to) to $spender";
                    } else if (decodedName == 'transferFrom') {
                      message =
                          "Transfer $token ${tokenDetails['symbol']} ($to) from $from_ to $spender";
                    }
                  }
                  return true;
                }(), builder: (context, snapshot) {
                  if (snapshot.hasError) {
                    return Column(
                      mainAxisAlignment: MainAxisAlignment.center,
                      children: [
                        Text(
                          AppLocalizations.of(context).couldNotFetchData,
                          style: const TextStyle(fontSize: 16.0),
                        )
                      ],
                    );
                  }
                  if (!snapshot.hasData) {
                    return Column(
                      mainAxisAlignment: MainAxisAlignment.center,
                      children: const [
                        Loader(),
                      ],
                    );
                  }
                  return SingleChildScrollView(
                    child: Padding(
                      padding: const EdgeInsets.only(
                          left: 25.0, right: 25, bottom: 25),
                      child: Column(
                        mainAxisAlignment: MainAxisAlignment.start,
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: [
                          if (networkIcon != null)
                            Container(
                              height: 50.0,
                              width: 50.0,
                              padding: const EdgeInsets.only(bottom: 8.0),
                              child: CachedNetworkImage(
                                imageUrl: ipfsTohttp(networkIcon),
                                placeholder: (context, url) => Column(
                                  mainAxisAlignment: MainAxisAlignment.center,
                                  mainAxisSize: MainAxisSize.min,
                                  children: const [
                                    SizedBox(
                                      width: 20,
                                      height: 20,
                                      child: Loader(
                                        color: appPrimaryColor,
                                      ),
                                    )
                                  ],
                                ),
                                errorWidget: (context, url, error) =>
                                    const Icon(
                                  Icons.error,
                                  color: Colors.red,
                                ),
                              ),
                            ),
                          if (name != null)
                            Text(
                              name,
                              style: const TextStyle(
                                fontWeight: FontWeight.normal,
                                fontSize: 16.0,
                              ),
                            ),
                          if (message != '')
                            Padding(
                              padding: const EdgeInsets.only(bottom: 8.0),
                              child: Column(
                                crossAxisAlignment: CrossAxisAlignment.start,
                                children: [
                                  Text(
                                    info,
                                    style: const TextStyle(
                                      fontWeight: FontWeight.bold,
                                      fontSize: 16.0,
                                    ),
                                  ),
                                  const SizedBox(height: 8.0),
                                  Text(
                                    message,
                                    style: const TextStyle(fontSize: 16.0),
                                  )
                                ],
                              ),
                            ),
                          Padding(
                            padding: const EdgeInsets.only(bottom: 8.0),
                            child: Column(
                              crossAxisAlignment: CrossAxisAlignment.start,
                              children: [
                                Text(
                                  AppLocalizations.of(context)
                                      .receipientAddress,
                                  style: const TextStyle(
                                    fontWeight: FontWeight.bold,
                                    fontSize: 16.0,
                                  ),
                                ),
                                const SizedBox(height: 8.0),
                                Text(
                                  to,
                                  style: const TextStyle(fontSize: 16.0),
                                ),
                              ],
                            ),
                          ),
                          Padding(
                            padding: const EdgeInsets.only(bottom: 8.0),
                            child: Column(
                              crossAxisAlignment: CrossAxisAlignment.start,
                              children: [
                                Text(
                                  AppLocalizations.of(context).balance,
                                  style: const TextStyle(
                                    fontWeight: FontWeight.bold,
                                    fontSize: 16.0,
                                  ),
                                ),
                                const SizedBox(height: 8.0),
                                Text(
                                  '${userBalance / pow(10, etherDecimals)} $blockChainCurrencySymbol',
                                  style: const TextStyle(fontSize: 16.0),
                                )
                              ],
                            ),
                          ),
                          Padding(
                            padding: const EdgeInsets.only(bottom: 8.0),
                            child: Column(
                              crossAxisAlignment: CrossAxisAlignment.start,
                              children: [
                                Text(
                                  AppLocalizations.of(context)
                                      .transactionAmount,
                                  style: const TextStyle(
                                    fontWeight: FontWeight.bold,
                                    fontSize: 16.0,
                                  ),
                                ),
                                const SizedBox(height: 8.0),
                                Text(
                                  '${value / pow(10, etherDecimals)} $blockChainCurrencySymbol',
                                  style: const TextStyle(fontSize: 16.0),
                                ),
                              ],
                            ),
                          ),
                          Column(
                            crossAxisAlignment: CrossAxisAlignment.start,
                            children: [
                              Padding(
                                padding: const EdgeInsets.only(bottom: 8.0),
                                child: Column(
                                  crossAxisAlignment: CrossAxisAlignment.start,
                                  children: [
                                    Text(
                                      AppLocalizations.of(context)
                                          .transactionFee,
                                      style: const TextStyle(
                                        fontWeight: FontWeight.bold,
                                        fontSize: 16.0,
                                      ),
                                    ),
                                    const SizedBox(height: 8.0),
                                    Text(
                                      '${transactionFee / pow(10, etherDecimals)} $blockChainCurrencySymbol',
                                      style: const TextStyle(fontSize: 16.0),
                                    )
                                  ],
                                ),
                              ),
                              if (transactionFee + value > userBalance)
                                Padding(
                                  padding: const EdgeInsets.only(bottom: 8.0),
                                  child: Column(
                                    crossAxisAlignment:
                                        CrossAxisAlignment.start,
                                    children: [
                                      Text(
                                        AppLocalizations.of(context)
                                            .insufficientBalance,
                                        style: const TextStyle(
                                          fontWeight: FontWeight.bold,
                                          color: red,
                                          fontSize: 16.0,
                                        ),
                                      ),
                                    ],
                                  ),
                                ),
                            ],
                          ),
                          ValueListenableBuilder(
                              valueListenable: isSigningTransaction,
                              builder: (_, isSigningTransaction_, __) {
                                if (isSigningTransaction_) {
                                  return Row(
                                    children: const [
                                      Loader(),
                                    ],
                                  );
                                }
                                return Row(
                                  children: [
                                    Expanded(
                                      child: TextButton(
                                        style: TextButton.styleFrom(
                                          foregroundColor: Colors.white,
                                          backgroundColor:
                                              const Color(0xff007bff),
                                        ),
                                        onPressed: () async {
                                          if (await authenticate(context)) {
                                            isSigningTransaction.value = true;
                                            try {
                                              await onConfirm();
                                            } catch (_) {}
                                            isSigningTransaction.value = false;
                                          } else {
                                            onReject();
                                          }
                                        },
                                        child: Text(
                                          AppLocalizations.of(context).confirm,
                                          style: const TextStyle(
                                            fontWeight: FontWeight.bold,
                                            fontSize: 18.0,
                                          ),
                                        ),
                                      ),
                                    ),
                                    const SizedBox(width: 16.0),
                                    Expanded(
                                      child: TextButton(
                                        style: TextButton.styleFrom(
                                          foregroundColor: Colors.white,
                                          backgroundColor:
                                              const Color(0xff007bff),
                                        ),
                                        onPressed: onReject,
                                        child: Text(
                                          AppLocalizations.of(context).reject,
                                          style: const TextStyle(
                                            fontWeight: FontWeight.bold,
                                            fontSize: 18.0,
                                          ),
                                        ),
                                      ),
                                    ),
                                  ],
                                );
                              }),
                        ],
                      ),
                    ),
                  );
                }),

                // second tab bar viiew widget
                if (decodedFunction != null)
                  SingleChildScrollView(
                    child: Padding(
                      padding: const EdgeInsets.only(
                          left: 25.0, right: 25, bottom: 25),
                      child: Column(
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: [
                          const SizedBox(
                            height: 20,
                          ),
                          Text(
                            AppLocalizations.of(context).functionType,
                            style: const TextStyle(
                              fontWeight: FontWeight.bold,
                              fontSize: 16.0,
                            ),
                          ),
                          const SizedBox(height: 5.0),
                          Text(
                            methodId,
                            style: const TextStyle(
                              fontSize: 16.0,
                            ),
                          ),
                          const SizedBox(
                            height: 20,
                          ),
                          for (var key in decodedParams.keys)
                            Column(
                              mainAxisAlignment: MainAxisAlignment.start,
                              crossAxisAlignment: CrossAxisAlignment.start,
                              children: [
                                Text(
                                  key,
                                  style: const TextStyle(
                                    fontSize: 16.0,
                                    fontWeight: FontWeight.bold,
                                  ),
                                ),
                                const SizedBox(height: 5.0),
                                Text(
                                  decodedParams[key],
                                  style: const TextStyle(
                                    fontSize: 16.0,
                                  ),
                                ),
                                const SizedBox(
                                  height: 10,
                                )
                              ],
                            ),
                        ],
                      ),
                    ),
                  )
                else
                  Container(),
                SingleChildScrollView(
                  child: Padding(
                    padding: const EdgeInsets.only(
                        left: 25.0, right: 25, bottom: 25),
                    child: Theme(
                      data: Theme.of(context)
                          .copyWith(dividerColor: Colors.transparent),
                      child: Padding(
                        padding: const EdgeInsets.only(bottom: 8.0),
                        child: ExpansionTile(
                          initiallyExpanded: true,
                          tilePadding: EdgeInsets.zero,
                          title: const Text(
                            "Hex",
                            style: TextStyle(
                              fontWeight: FontWeight.bold,
                              fontSize: 16.0,
                            ),
                          ),
                          children: [
                            Text(
                              txData,
                              style: const TextStyle(fontSize: 16.0),
                            ),
                          ],
                        ),
                      ),
                    ),
                  ),
                ),
              ],
            ),
          ),
        ],
      ),
    ),
    canDismiss: false,
  );
}

signMessage({
  BuildContext context,
  String data,
  String networkIcon,
  String name,
  Function onConfirm,
  Function onReject,
  String messageType,
}) async {
  String decoded = data;

  if (messageType == personalSignKey && data != null && isHexString(data)) {
    decoded = ascii.decode(txDataToUintList(data));
  }

  slideUpPanel(
    context,
    SingleChildScrollView(
      child: Padding(
        padding: const EdgeInsets.only(left: 25.0, right: 25, bottom: 25),
        child: Column(
          mainAxisAlignment: MainAxisAlignment.start,
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Container(
              padding: const EdgeInsets.only(bottom: 8.0),
              child: Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  const Align(
                    alignment: Alignment.centerRight,
                    child: IconButton(
                      onPressed: null,
                      icon: Icon(
                        Icons.close,
                        color: Colors.transparent,
                      ),
                    ),
                  ),
                  Text(
                    AppLocalizations.of(context).signMessage,
                    style: const TextStyle(
                      fontWeight: FontWeight.bold,
                      fontSize: 20.0,
                    ),
                  ),
                  Align(
                    alignment: Alignment.centerRight,
                    child: IconButton(
                      onPressed: () {
                        if (Navigator.canPop(context)) {
                          onReject();
                        }
                      },
                      icon: const Icon(Icons.close),
                    ),
                  ),
                ],
              ),
            ),
            if (networkIcon != null)
              Container(
                height: 50.0,
                width: 50.0,
                padding: const EdgeInsets.only(bottom: 8.0),
                child: CachedNetworkImage(
                  imageUrl: ipfsTohttp(networkIcon),
                  placeholder: (context, url) => Column(
                    mainAxisAlignment: MainAxisAlignment.center,
                    mainAxisSize: MainAxisSize.min,
                    children: const [
                      SizedBox(
                        width: 20,
                        height: 20,
                        child: Loader(
                          color: appPrimaryColor,
                        ),
                      )
                    ],
                  ),
                  errorWidget: (context, url, error) => const Icon(
                    Icons.error,
                    color: Colors.red,
                  ),
                ),
              ),
            if (name != null)
              Text(
                name,
                style: const TextStyle(
                  fontWeight: FontWeight.normal,
                  fontSize: 16.0,
                ),
              ),
            Theme(
              data:
                  Theme.of(context).copyWith(dividerColor: Colors.transparent),
              child: Padding(
                padding: const EdgeInsets.only(bottom: 8.0),
                child: ExpansionTile(
                  initiallyExpanded: true,
                  tilePadding: EdgeInsets.zero,
                  title: Text(
                    AppLocalizations.of(context).message,
                    style: const TextStyle(
                      fontWeight: FontWeight.bold,
                      fontSize: 18.0,
                    ),
                  ),
                  children: [
                    if (messageType == typedMessageSignKey || true)
                      Container(
                        child: JsonViewer(
                          [
                            {
                              "inputs": [
                                {
                                  "internalType": "address",
                                  "name": "conduitController",
                                  "type": "address"
                                }
                              ],
                              "stateMutability": "nonpayable",
                              "type": "constructor"
                            },
                            {
                              "inputs": [],
                              "name": "BadContractSignature",
                              "type": "error"
                            },
                            {
                              "inputs": [],
                              "name": "BadFraction",
                              "type": "error"
                            },
                            {
                              "inputs": [
                                {
                                  "internalType": "address",
                                  "name": "token",
                                  "type": "address"
                                },
                                {
                                  "internalType": "address",
                                  "name": "from",
                                  "type": "address"
                                },
                                {
                                  "internalType": "address",
                                  "name": "to",
                                  "type": "address"
                                },
                                {
                                  "internalType": "uint256",
                                  "name": "amount",
                                  "type": "uint256"
                                }
                              ],
                              "name": "BadReturnValueFromERC20OnTransfer",
                              "type": "error"
                            },
                            {
                              "inputs": [
                                {
                                  "internalType": "uint8",
                                  "name": "v",
                                  "type": "uint8"
                                }
                              ],
                              "name": "BadSignatureV",
                              "type": "error"
                            },
                            {
                              "inputs": [],
                              "name": "ConsiderationCriteriaResolverOutOfRange",
                              "type": "error"
                            },
                            {
                              "inputs": [
                                {
                                  "internalType": "uint256",
                                  "name": "orderIndex",
                                  "type": "uint256"
                                },
                                {
                                  "internalType": "uint256",
                                  "name": "considerationIndex",
                                  "type": "uint256"
                                },
                                {
                                  "internalType": "uint256",
                                  "name": "shortfallAmount",
                                  "type": "uint256"
                                }
                              ],
                              "name": "ConsiderationNotMet",
                              "type": "error"
                            },
                            {
                              "inputs": [],
                              "name": "CriteriaNotEnabledForItem",
                              "type": "error"
                            },
                            {
                              "inputs": [
                                {
                                  "internalType": "address",
                                  "name": "token",
                                  "type": "address"
                                },
                                {
                                  "internalType": "address",
                                  "name": "from",
                                  "type": "address"
                                },
                                {
                                  "internalType": "address",
                                  "name": "to",
                                  "type": "address"
                                },
                                {
                                  "internalType": "uint256[]",
                                  "name": "identifiers",
                                  "type": "uint256[]"
                                },
                                {
                                  "internalType": "uint256[]",
                                  "name": "amounts",
                                  "type": "uint256[]"
                                }
                              ],
                              "name": "ERC1155BatchTransferGenericFailure",
                              "type": "error"
                            },
                            {
                              "inputs": [
                                {
                                  "internalType": "address",
                                  "name": "account",
                                  "type": "address"
                                },
                                {
                                  "internalType": "uint256",
                                  "name": "amount",
                                  "type": "uint256"
                                }
                              ],
                              "name": "EtherTransferGenericFailure",
                              "type": "error"
                            },
                            {
                              "inputs": [],
                              "name": "InexactFraction",
                              "type": "error"
                            },
                            {
                              "inputs": [],
                              "name": "InsufficientEtherSupplied",
                              "type": "error"
                            },
                            {
                              "inputs": [],
                              "name": "Invalid1155BatchTransferEncoding",
                              "type": "error"
                            },
                            {
                              "inputs": [],
                              "name": "InvalidBasicOrderParameterEncoding",
                              "type": "error"
                            },
                            {
                              "inputs": [
                                {
                                  "internalType": "address",
                                  "name": "conduit",
                                  "type": "address"
                                }
                              ],
                              "name": "InvalidCallToConduit",
                              "type": "error"
                            },
                            {
                              "inputs": [],
                              "name": "InvalidCanceller",
                              "type": "error"
                            },
                            {
                              "inputs": [
                                {
                                  "internalType": "bytes32",
                                  "name": "conduitKey",
                                  "type": "bytes32"
                                },
                                {
                                  "internalType": "address",
                                  "name": "conduit",
                                  "type": "address"
                                }
                              ],
                              "name": "InvalidConduit",
                              "type": "error"
                            },
                            {
                              "inputs": [],
                              "name": "InvalidERC721TransferAmount",
                              "type": "error"
                            },
                            {
                              "inputs": [],
                              "name": "InvalidFulfillmentComponentData",
                              "type": "error"
                            },
                            {
                              "inputs": [
                                {
                                  "internalType": "uint256",
                                  "name": "value",
                                  "type": "uint256"
                                }
                              ],
                              "name": "InvalidMsgValue",
                              "type": "error"
                            },
                            {
                              "inputs": [],
                              "name": "InvalidNativeOfferItem",
                              "type": "error"
                            },
                            {
                              "inputs": [],
                              "name": "InvalidProof",
                              "type": "error"
                            },
                            {
                              "inputs": [
                                {
                                  "internalType": "bytes32",
                                  "name": "orderHash",
                                  "type": "bytes32"
                                }
                              ],
                              "name": "InvalidRestrictedOrder",
                              "type": "error"
                            },
                            {
                              "inputs": [],
                              "name": "InvalidSignature",
                              "type": "error"
                            },
                            {
                              "inputs": [],
                              "name": "InvalidSigner",
                              "type": "error"
                            },
                            {
                              "inputs": [],
                              "name": "InvalidTime",
                              "type": "error"
                            },
                            {
                              "inputs": [],
                              "name":
                                  "MismatchedFulfillmentOfferAndConsiderationComponents",
                              "type": "error"
                            },
                            {
                              "inputs": [
                                {
                                  "internalType": "enum Side",
                                  "name": "side",
                                  "type": "uint8"
                                }
                              ],
                              "name":
                                  "MissingFulfillmentComponentOnAggregation",
                              "type": "error"
                            },
                            {
                              "inputs": [],
                              "name": "MissingItemAmount",
                              "type": "error"
                            },
                            {
                              "inputs": [],
                              "name": "MissingOriginalConsiderationItems",
                              "type": "error"
                            },
                            {
                              "inputs": [
                                {
                                  "internalType": "address",
                                  "name": "account",
                                  "type": "address"
                                }
                              ],
                              "name": "NoContract",
                              "type": "error"
                            },
                            {
                              "inputs": [],
                              "name": "NoReentrantCalls",
                              "type": "error"
                            },
                            {
                              "inputs": [],
                              "name": "NoSpecifiedOrdersAvailable",
                              "type": "error"
                            },
                            {
                              "inputs": [],
                              "name":
                                  "OfferAndConsiderationRequiredOnFulfillment",
                              "type": "error"
                            },
                            {
                              "inputs": [],
                              "name": "OfferCriteriaResolverOutOfRange",
                              "type": "error"
                            },
                            {
                              "inputs": [
                                {
                                  "internalType": "bytes32",
                                  "name": "orderHash",
                                  "type": "bytes32"
                                }
                              ],
                              "name": "OrderAlreadyFilled",
                              "type": "error"
                            },
                            {
                              "inputs": [],
                              "name": "OrderCriteriaResolverOutOfRange",
                              "type": "error"
                            },
                            {
                              "inputs": [
                                {
                                  "internalType": "bytes32",
                                  "name": "orderHash",
                                  "type": "bytes32"
                                }
                              ],
                              "name": "OrderIsCancelled",
                              "type": "error"
                            },
                            {
                              "inputs": [
                                {
                                  "internalType": "bytes32",
                                  "name": "orderHash",
                                  "type": "bytes32"
                                }
                              ],
                              "name": "OrderPartiallyFilled",
                              "type": "error"
                            },
                            {
                              "inputs": [],
                              "name": "PartialFillsNotEnabledForOrder",
                              "type": "error"
                            },
                            {
                              "inputs": [
                                {
                                  "internalType": "address",
                                  "name": "token",
                                  "type": "address"
                                },
                                {
                                  "internalType": "address",
                                  "name": "from",
                                  "type": "address"
                                },
                                {
                                  "internalType": "address",
                                  "name": "to",
                                  "type": "address"
                                },
                                {
                                  "internalType": "uint256",
                                  "name": "identifier",
                                  "type": "uint256"
                                },
                                {
                                  "internalType": "uint256",
                                  "name": "amount",
                                  "type": "uint256"
                                }
                              ],
                              "name": "TokenTransferGenericFailure",
                              "type": "error"
                            },
                            {
                              "inputs": [],
                              "name": "UnresolvedConsiderationCriteria",
                              "type": "error"
                            },
                            {
                              "inputs": [],
                              "name": "UnresolvedOfferCriteria",
                              "type": "error"
                            },
                            {
                              "inputs": [],
                              "name": "UnusedItemParameters",
                              "type": "error"
                            },
                            {
                              "anonymous": false,
                              "inputs": [
                                {
                                  "indexed": false,
                                  "internalType": "uint256",
                                  "name": "newCounter",
                                  "type": "uint256"
                                },
                                {
                                  "indexed": true,
                                  "internalType": "address",
                                  "name": "offerer",
                                  "type": "address"
                                }
                              ],
                              "name": "CounterIncremented",
                              "type": "event"
                            },
                            {
                              "anonymous": false,
                              "inputs": [
                                {
                                  "indexed": false,
                                  "internalType": "bytes32",
                                  "name": "orderHash",
                                  "type": "bytes32"
                                },
                                {
                                  "indexed": true,
                                  "internalType": "address",
                                  "name": "offerer",
                                  "type": "address"
                                },
                                {
                                  "indexed": true,
                                  "internalType": "address",
                                  "name": "zone",
                                  "type": "address"
                                }
                              ],
                              "name": "OrderCancelled",
                              "type": "event"
                            },
                            {
                              "anonymous": false,
                              "inputs": [
                                {
                                  "indexed": false,
                                  "internalType": "bytes32",
                                  "name": "orderHash",
                                  "type": "bytes32"
                                },
                                {
                                  "indexed": true,
                                  "internalType": "address",
                                  "name": "offerer",
                                  "type": "address"
                                },
                                {
                                  "indexed": true,
                                  "internalType": "address",
                                  "name": "zone",
                                  "type": "address"
                                },
                                {
                                  "indexed": false,
                                  "internalType": "address",
                                  "name": "recipient",
                                  "type": "address"
                                },
                                {
                                  "components": [
                                    {
                                      "internalType": "enum ItemType",
                                      "name": "itemType",
                                      "type": "uint8"
                                    },
                                    {
                                      "internalType": "address",
                                      "name": "token",
                                      "type": "address"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name": "identifier",
                                      "type": "uint256"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name": "amount",
                                      "type": "uint256"
                                    }
                                  ],
                                  "indexed": false,
                                  "internalType": "struct SpentItem[]",
                                  "name": "offer",
                                  "type": "tuple[]"
                                },
                                {
                                  "components": [
                                    {
                                      "internalType": "enum ItemType",
                                      "name": "itemType",
                                      "type": "uint8"
                                    },
                                    {
                                      "internalType": "address",
                                      "name": "token",
                                      "type": "address"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name": "identifier",
                                      "type": "uint256"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name": "amount",
                                      "type": "uint256"
                                    },
                                    {
                                      "internalType": "address payable",
                                      "name": "recipient",
                                      "type": "address"
                                    }
                                  ],
                                  "indexed": false,
                                  "internalType": "struct ReceivedItem[]",
                                  "name": "consideration",
                                  "type": "tuple[]"
                                }
                              ],
                              "name": "OrderFulfilled",
                              "type": "event"
                            },
                            {
                              "anonymous": false,
                              "inputs": [
                                {
                                  "indexed": false,
                                  "internalType": "bytes32",
                                  "name": "orderHash",
                                  "type": "bytes32"
                                },
                                {
                                  "indexed": true,
                                  "internalType": "address",
                                  "name": "offerer",
                                  "type": "address"
                                },
                                {
                                  "indexed": true,
                                  "internalType": "address",
                                  "name": "zone",
                                  "type": "address"
                                }
                              ],
                              "name": "OrderValidated",
                              "type": "event"
                            },
                            {
                              "inputs": [
                                {
                                  "components": [
                                    {
                                      "internalType": "address",
                                      "name": "offerer",
                                      "type": "address"
                                    },
                                    {
                                      "internalType": "address",
                                      "name": "zone",
                                      "type": "address"
                                    },
                                    {
                                      "components": [
                                        {
                                          "internalType": "enum ItemType",
                                          "name": "itemType",
                                          "type": "uint8"
                                        },
                                        {
                                          "internalType": "address",
                                          "name": "token",
                                          "type": "address"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "identifierOrCriteria",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "startAmount",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "endAmount",
                                          "type": "uint256"
                                        }
                                      ],
                                      "internalType": "struct OfferItem[]",
                                      "name": "offer",
                                      "type": "tuple[]"
                                    },
                                    {
                                      "components": [
                                        {
                                          "internalType": "enum ItemType",
                                          "name": "itemType",
                                          "type": "uint8"
                                        },
                                        {
                                          "internalType": "address",
                                          "name": "token",
                                          "type": "address"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "identifierOrCriteria",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "startAmount",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "endAmount",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "address payable",
                                          "name": "recipient",
                                          "type": "address"
                                        }
                                      ],
                                      "internalType":
                                          "struct ConsiderationItem[]",
                                      "name": "consideration",
                                      "type": "tuple[]"
                                    },
                                    {
                                      "internalType": "enum OrderType",
                                      "name": "orderType",
                                      "type": "uint8"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name": "startTime",
                                      "type": "uint256"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name": "endTime",
                                      "type": "uint256"
                                    },
                                    {
                                      "internalType": "bytes32",
                                      "name": "zoneHash",
                                      "type": "bytes32"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name": "salt",
                                      "type": "uint256"
                                    },
                                    {
                                      "internalType": "bytes32",
                                      "name": "conduitKey",
                                      "type": "bytes32"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name": "counter",
                                      "type": "uint256"
                                    }
                                  ],
                                  "internalType": "struct OrderComponents[]",
                                  "name": "orders",
                                  "type": "tuple[]"
                                }
                              ],
                              "name": "cancel",
                              "outputs": [
                                {
                                  "internalType": "bool",
                                  "name": "cancelled",
                                  "type": "bool"
                                }
                              ],
                              "stateMutability": "nonpayable",
                              "type": "function"
                            },
                            {
                              "inputs": [
                                {
                                  "components": [
                                    {
                                      "components": [
                                        {
                                          "internalType": "address",
                                          "name": "offerer",
                                          "type": "address"
                                        },
                                        {
                                          "internalType": "address",
                                          "name": "zone",
                                          "type": "address"
                                        },
                                        {
                                          "components": [
                                            {
                                              "internalType": "enum ItemType",
                                              "name": "itemType",
                                              "type": "uint8"
                                            },
                                            {
                                              "internalType": "address",
                                              "name": "token",
                                              "type": "address"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "identifierOrCriteria",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "startAmount",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "endAmount",
                                              "type": "uint256"
                                            }
                                          ],
                                          "internalType": "struct OfferItem[]",
                                          "name": "offer",
                                          "type": "tuple[]"
                                        },
                                        {
                                          "components": [
                                            {
                                              "internalType": "enum ItemType",
                                              "name": "itemType",
                                              "type": "uint8"
                                            },
                                            {
                                              "internalType": "address",
                                              "name": "token",
                                              "type": "address"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "identifierOrCriteria",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "startAmount",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "endAmount",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "address payable",
                                              "name": "recipient",
                                              "type": "address"
                                            }
                                          ],
                                          "internalType":
                                              "struct ConsiderationItem[]",
                                          "name": "consideration",
                                          "type": "tuple[]"
                                        },
                                        {
                                          "internalType": "enum OrderType",
                                          "name": "orderType",
                                          "type": "uint8"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "startTime",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "endTime",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "bytes32",
                                          "name": "zoneHash",
                                          "type": "bytes32"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "salt",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "bytes32",
                                          "name": "conduitKey",
                                          "type": "bytes32"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name":
                                              "totalOriginalConsiderationItems",
                                          "type": "uint256"
                                        }
                                      ],
                                      "internalType": "struct OrderParameters",
                                      "name": "parameters",
                                      "type": "tuple"
                                    },
                                    {
                                      "internalType": "uint120",
                                      "name": "numerator",
                                      "type": "uint120"
                                    },
                                    {
                                      "internalType": "uint120",
                                      "name": "denominator",
                                      "type": "uint120"
                                    },
                                    {
                                      "internalType": "bytes",
                                      "name": "signature",
                                      "type": "bytes"
                                    },
                                    {
                                      "internalType": "bytes",
                                      "name": "extraData",
                                      "type": "bytes"
                                    }
                                  ],
                                  "internalType": "struct AdvancedOrder",
                                  "name": "advancedOrder",
                                  "type": "tuple"
                                },
                                {
                                  "components": [
                                    {
                                      "internalType": "uint256",
                                      "name": "orderIndex",
                                      "type": "uint256"
                                    },
                                    {
                                      "internalType": "enum Side",
                                      "name": "side",
                                      "type": "uint8"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name": "index",
                                      "type": "uint256"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name": "identifier",
                                      "type": "uint256"
                                    },
                                    {
                                      "internalType": "bytes32[]",
                                      "name": "criteriaProof",
                                      "type": "bytes32[]"
                                    }
                                  ],
                                  "internalType": "struct CriteriaResolver[]",
                                  "name": "criteriaResolvers",
                                  "type": "tuple[]"
                                },
                                {
                                  "internalType": "bytes32",
                                  "name": "fulfillerConduitKey",
                                  "type": "bytes32"
                                },
                                {
                                  "internalType": "address",
                                  "name": "recipient",
                                  "type": "address"
                                }
                              ],
                              "name": "fulfillAdvancedOrder",
                              "outputs": [
                                {
                                  "internalType": "bool",
                                  "name": "fulfilled",
                                  "type": "bool"
                                }
                              ],
                              "stateMutability": "payable",
                              "type": "function"
                            },
                            {
                              "inputs": [
                                {
                                  "components": [
                                    {
                                      "components": [
                                        {
                                          "internalType": "address",
                                          "name": "offerer",
                                          "type": "address"
                                        },
                                        {
                                          "internalType": "address",
                                          "name": "zone",
                                          "type": "address"
                                        },
                                        {
                                          "components": [
                                            {
                                              "internalType": "enum ItemType",
                                              "name": "itemType",
                                              "type": "uint8"
                                            },
                                            {
                                              "internalType": "address",
                                              "name": "token",
                                              "type": "address"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "identifierOrCriteria",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "startAmount",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "endAmount",
                                              "type": "uint256"
                                            }
                                          ],
                                          "internalType": "struct OfferItem[]",
                                          "name": "offer",
                                          "type": "tuple[]"
                                        },
                                        {
                                          "components": [
                                            {
                                              "internalType": "enum ItemType",
                                              "name": "itemType",
                                              "type": "uint8"
                                            },
                                            {
                                              "internalType": "address",
                                              "name": "token",
                                              "type": "address"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "identifierOrCriteria",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "startAmount",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "endAmount",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "address payable",
                                              "name": "recipient",
                                              "type": "address"
                                            }
                                          ],
                                          "internalType":
                                              "struct ConsiderationItem[]",
                                          "name": "consideration",
                                          "type": "tuple[]"
                                        },
                                        {
                                          "internalType": "enum OrderType",
                                          "name": "orderType",
                                          "type": "uint8"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "startTime",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "endTime",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "bytes32",
                                          "name": "zoneHash",
                                          "type": "bytes32"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "salt",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "bytes32",
                                          "name": "conduitKey",
                                          "type": "bytes32"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name":
                                              "totalOriginalConsiderationItems",
                                          "type": "uint256"
                                        }
                                      ],
                                      "internalType": "struct OrderParameters",
                                      "name": "parameters",
                                      "type": "tuple"
                                    },
                                    {
                                      "internalType": "uint120",
                                      "name": "numerator",
                                      "type": "uint120"
                                    },
                                    {
                                      "internalType": "uint120",
                                      "name": "denominator",
                                      "type": "uint120"
                                    },
                                    {
                                      "internalType": "bytes",
                                      "name": "signature",
                                      "type": "bytes"
                                    },
                                    {
                                      "internalType": "bytes",
                                      "name": "extraData",
                                      "type": "bytes"
                                    }
                                  ],
                                  "internalType": "struct AdvancedOrder[]",
                                  "name": "advancedOrders",
                                  "type": "tuple[]"
                                },
                                {
                                  "components": [
                                    {
                                      "internalType": "uint256",
                                      "name": "orderIndex",
                                      "type": "uint256"
                                    },
                                    {
                                      "internalType": "enum Side",
                                      "name": "side",
                                      "type": "uint8"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name": "index",
                                      "type": "uint256"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name": "identifier",
                                      "type": "uint256"
                                    },
                                    {
                                      "internalType": "bytes32[]",
                                      "name": "criteriaProof",
                                      "type": "bytes32[]"
                                    }
                                  ],
                                  "internalType": "struct CriteriaResolver[]",
                                  "name": "criteriaResolvers",
                                  "type": "tuple[]"
                                },
                                {
                                  "components": [
                                    {
                                      "internalType": "uint256",
                                      "name": "orderIndex",
                                      "type": "uint256"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name": "itemIndex",
                                      "type": "uint256"
                                    }
                                  ],
                                  "internalType":
                                      "struct FulfillmentComponent[][]",
                                  "name": "offerFulfillments",
                                  "type": "tuple[][]"
                                },
                                {
                                  "components": [
                                    {
                                      "internalType": "uint256",
                                      "name": "orderIndex",
                                      "type": "uint256"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name": "itemIndex",
                                      "type": "uint256"
                                    }
                                  ],
                                  "internalType":
                                      "struct FulfillmentComponent[][]",
                                  "name": "considerationFulfillments",
                                  "type": "tuple[][]"
                                },
                                {
                                  "internalType": "bytes32",
                                  "name": "fulfillerConduitKey",
                                  "type": "bytes32"
                                },
                                {
                                  "internalType": "address",
                                  "name": "recipient",
                                  "type": "address"
                                },
                                {
                                  "internalType": "uint256",
                                  "name": "maximumFulfilled",
                                  "type": "uint256"
                                }
                              ],
                              "name": "fulfillAvailableAdvancedOrders",
                              "outputs": [
                                {
                                  "internalType": "bool[]",
                                  "name": "availableOrders",
                                  "type": "bool[]"
                                },
                                {
                                  "components": [
                                    {
                                      "components": [
                                        {
                                          "internalType": "enum ItemType",
                                          "name": "itemType",
                                          "type": "uint8"
                                        },
                                        {
                                          "internalType": "address",
                                          "name": "token",
                                          "type": "address"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "identifier",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "amount",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "address payable",
                                          "name": "recipient",
                                          "type": "address"
                                        }
                                      ],
                                      "internalType": "struct ReceivedItem",
                                      "name": "item",
                                      "type": "tuple"
                                    },
                                    {
                                      "internalType": "address",
                                      "name": "offerer",
                                      "type": "address"
                                    },
                                    {
                                      "internalType": "bytes32",
                                      "name": "conduitKey",
                                      "type": "bytes32"
                                    }
                                  ],
                                  "internalType": "struct Execution[]",
                                  "name": "executions",
                                  "type": "tuple[]"
                                }
                              ],
                              "stateMutability": "payable",
                              "type": "function"
                            },
                            {
                              "inputs": [
                                {
                                  "components": [
                                    {
                                      "components": [
                                        {
                                          "internalType": "address",
                                          "name": "offerer",
                                          "type": "address"
                                        },
                                        {
                                          "internalType": "address",
                                          "name": "zone",
                                          "type": "address"
                                        },
                                        {
                                          "components": [
                                            {
                                              "internalType": "enum ItemType",
                                              "name": "itemType",
                                              "type": "uint8"
                                            },
                                            {
                                              "internalType": "address",
                                              "name": "token",
                                              "type": "address"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "identifierOrCriteria",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "startAmount",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "endAmount",
                                              "type": "uint256"
                                            }
                                          ],
                                          "internalType": "struct OfferItem[]",
                                          "name": "offer",
                                          "type": "tuple[]"
                                        },
                                        {
                                          "components": [
                                            {
                                              "internalType": "enum ItemType",
                                              "name": "itemType",
                                              "type": "uint8"
                                            },
                                            {
                                              "internalType": "address",
                                              "name": "token",
                                              "type": "address"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "identifierOrCriteria",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "startAmount",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "endAmount",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "address payable",
                                              "name": "recipient",
                                              "type": "address"
                                            }
                                          ],
                                          "internalType":
                                              "struct ConsiderationItem[]",
                                          "name": "consideration",
                                          "type": "tuple[]"
                                        },
                                        {
                                          "internalType": "enum OrderType",
                                          "name": "orderType",
                                          "type": "uint8"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "startTime",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "endTime",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "bytes32",
                                          "name": "zoneHash",
                                          "type": "bytes32"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "salt",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "bytes32",
                                          "name": "conduitKey",
                                          "type": "bytes32"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name":
                                              "totalOriginalConsiderationItems",
                                          "type": "uint256"
                                        }
                                      ],
                                      "internalType": "struct OrderParameters",
                                      "name": "parameters",
                                      "type": "tuple"
                                    },
                                    {
                                      "internalType": "bytes",
                                      "name": "signature",
                                      "type": "bytes"
                                    }
                                  ],
                                  "internalType": "struct Order[]",
                                  "name": "orders",
                                  "type": "tuple[]"
                                },
                                {
                                  "components": [
                                    {
                                      "internalType": "uint256",
                                      "name": "orderIndex",
                                      "type": "uint256"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name": "itemIndex",
                                      "type": "uint256"
                                    }
                                  ],
                                  "internalType":
                                      "struct FulfillmentComponent[][]",
                                  "name": "offerFulfillments",
                                  "type": "tuple[][]"
                                },
                                {
                                  "components": [
                                    {
                                      "internalType": "uint256",
                                      "name": "orderIndex",
                                      "type": "uint256"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name": "itemIndex",
                                      "type": "uint256"
                                    }
                                  ],
                                  "internalType":
                                      "struct FulfillmentComponent[][]",
                                  "name": "considerationFulfillments",
                                  "type": "tuple[][]"
                                },
                                {
                                  "internalType": "bytes32",
                                  "name": "fulfillerConduitKey",
                                  "type": "bytes32"
                                },
                                {
                                  "internalType": "uint256",
                                  "name": "maximumFulfilled",
                                  "type": "uint256"
                                }
                              ],
                              "name": "fulfillAvailableOrders",
                              "outputs": [
                                {
                                  "internalType": "bool[]",
                                  "name": "availableOrders",
                                  "type": "bool[]"
                                },
                                {
                                  "components": [
                                    {
                                      "components": [
                                        {
                                          "internalType": "enum ItemType",
                                          "name": "itemType",
                                          "type": "uint8"
                                        },
                                        {
                                          "internalType": "address",
                                          "name": "token",
                                          "type": "address"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "identifier",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "amount",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "address payable",
                                          "name": "recipient",
                                          "type": "address"
                                        }
                                      ],
                                      "internalType": "struct ReceivedItem",
                                      "name": "item",
                                      "type": "tuple"
                                    },
                                    {
                                      "internalType": "address",
                                      "name": "offerer",
                                      "type": "address"
                                    },
                                    {
                                      "internalType": "bytes32",
                                      "name": "conduitKey",
                                      "type": "bytes32"
                                    }
                                  ],
                                  "internalType": "struct Execution[]",
                                  "name": "executions",
                                  "type": "tuple[]"
                                }
                              ],
                              "stateMutability": "payable",
                              "type": "function"
                            },
                            {
                              "inputs": [
                                {
                                  "components": [
                                    {
                                      "internalType": "address",
                                      "name": "considerationToken",
                                      "type": "address"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name": "considerationIdentifier",
                                      "type": "uint256"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name": "considerationAmount",
                                      "type": "uint256"
                                    },
                                    {
                                      "internalType": "address payable",
                                      "name": "offerer",
                                      "type": "address"
                                    },
                                    {
                                      "internalType": "address",
                                      "name": "zone",
                                      "type": "address"
                                    },
                                    {
                                      "internalType": "address",
                                      "name": "offerToken",
                                      "type": "address"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name": "offerIdentifier",
                                      "type": "uint256"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name": "offerAmount",
                                      "type": "uint256"
                                    },
                                    {
                                      "internalType": "enum BasicOrderType",
                                      "name": "basicOrderType",
                                      "type": "uint8"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name": "startTime",
                                      "type": "uint256"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name": "endTime",
                                      "type": "uint256"
                                    },
                                    {
                                      "internalType": "bytes32",
                                      "name": "zoneHash",
                                      "type": "bytes32"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name": "salt",
                                      "type": "uint256"
                                    },
                                    {
                                      "internalType": "bytes32",
                                      "name": "offererConduitKey",
                                      "type": "bytes32"
                                    },
                                    {
                                      "internalType": "bytes32",
                                      "name": "fulfillerConduitKey",
                                      "type": "bytes32"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name":
                                          "totalOriginalAdditionalRecipients",
                                      "type": "uint256"
                                    },
                                    {
                                      "components": [
                                        {
                                          "internalType": "uint256",
                                          "name": "amount",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "address payable",
                                          "name": "recipient",
                                          "type": "address"
                                        }
                                      ],
                                      "internalType":
                                          "struct AdditionalRecipient[]",
                                      "name": "additionalRecipients",
                                      "type": "tuple[]"
                                    },
                                    {
                                      "internalType": "bytes",
                                      "name": "signature",
                                      "type": "bytes"
                                    }
                                  ],
                                  "internalType": "struct BasicOrderParameters",
                                  "name": "parameters",
                                  "type": "tuple"
                                }
                              ],
                              "name": "fulfillBasicOrder",
                              "outputs": [
                                {
                                  "internalType": "bool",
                                  "name": "fulfilled",
                                  "type": "bool"
                                }
                              ],
                              "stateMutability": "payable",
                              "type": "function"
                            },
                            {
                              "inputs": [
                                {
                                  "components": [
                                    {
                                      "components": [
                                        {
                                          "internalType": "address",
                                          "name": "offerer",
                                          "type": "address"
                                        },
                                        {
                                          "internalType": "address",
                                          "name": "zone",
                                          "type": "address"
                                        },
                                        {
                                          "components": [
                                            {
                                              "internalType": "enum ItemType",
                                              "name": "itemType",
                                              "type": "uint8"
                                            },
                                            {
                                              "internalType": "address",
                                              "name": "token",
                                              "type": "address"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "identifierOrCriteria",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "startAmount",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "endAmount",
                                              "type": "uint256"
                                            }
                                          ],
                                          "internalType": "struct OfferItem[]",
                                          "name": "offer",
                                          "type": "tuple[]"
                                        },
                                        {
                                          "components": [
                                            {
                                              "internalType": "enum ItemType",
                                              "name": "itemType",
                                              "type": "uint8"
                                            },
                                            {
                                              "internalType": "address",
                                              "name": "token",
                                              "type": "address"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "identifierOrCriteria",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "startAmount",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "endAmount",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "address payable",
                                              "name": "recipient",
                                              "type": "address"
                                            }
                                          ],
                                          "internalType":
                                              "struct ConsiderationItem[]",
                                          "name": "consideration",
                                          "type": "tuple[]"
                                        },
                                        {
                                          "internalType": "enum OrderType",
                                          "name": "orderType",
                                          "type": "uint8"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "startTime",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "endTime",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "bytes32",
                                          "name": "zoneHash",
                                          "type": "bytes32"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "salt",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "bytes32",
                                          "name": "conduitKey",
                                          "type": "bytes32"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name":
                                              "totalOriginalConsiderationItems",
                                          "type": "uint256"
                                        }
                                      ],
                                      "internalType": "struct OrderParameters",
                                      "name": "parameters",
                                      "type": "tuple"
                                    },
                                    {
                                      "internalType": "bytes",
                                      "name": "signature",
                                      "type": "bytes"
                                    }
                                  ],
                                  "internalType": "struct Order",
                                  "name": "order",
                                  "type": "tuple"
                                },
                                {
                                  "internalType": "bytes32",
                                  "name": "fulfillerConduitKey",
                                  "type": "bytes32"
                                }
                              ],
                              "name": "fulfillOrder",
                              "outputs": [
                                {
                                  "internalType": "bool",
                                  "name": "fulfilled",
                                  "type": "bool"
                                }
                              ],
                              "stateMutability": "payable",
                              "type": "function"
                            },
                            {
                              "inputs": [
                                {
                                  "internalType": "address",
                                  "name": "offerer",
                                  "type": "address"
                                }
                              ],
                              "name": "getCounter",
                              "outputs": [
                                {
                                  "internalType": "uint256",
                                  "name": "counter",
                                  "type": "uint256"
                                }
                              ],
                              "stateMutability": "view",
                              "type": "function"
                            },
                            {
                              "inputs": [
                                {
                                  "components": [
                                    {
                                      "internalType": "address",
                                      "name": "offerer",
                                      "type": "address"
                                    },
                                    {
                                      "internalType": "address",
                                      "name": "zone",
                                      "type": "address"
                                    },
                                    {
                                      "components": [
                                        {
                                          "internalType": "enum ItemType",
                                          "name": "itemType",
                                          "type": "uint8"
                                        },
                                        {
                                          "internalType": "address",
                                          "name": "token",
                                          "type": "address"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "identifierOrCriteria",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "startAmount",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "endAmount",
                                          "type": "uint256"
                                        }
                                      ],
                                      "internalType": "struct OfferItem[]",
                                      "name": "offer",
                                      "type": "tuple[]"
                                    },
                                    {
                                      "components": [
                                        {
                                          "internalType": "enum ItemType",
                                          "name": "itemType",
                                          "type": "uint8"
                                        },
                                        {
                                          "internalType": "address",
                                          "name": "token",
                                          "type": "address"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "identifierOrCriteria",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "startAmount",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "endAmount",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "address payable",
                                          "name": "recipient",
                                          "type": "address"
                                        }
                                      ],
                                      "internalType":
                                          "struct ConsiderationItem[]",
                                      "name": "consideration",
                                      "type": "tuple[]"
                                    },
                                    {
                                      "internalType": "enum OrderType",
                                      "name": "orderType",
                                      "type": "uint8"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name": "startTime",
                                      "type": "uint256"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name": "endTime",
                                      "type": "uint256"
                                    },
                                    {
                                      "internalType": "bytes32",
                                      "name": "zoneHash",
                                      "type": "bytes32"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name": "salt",
                                      "type": "uint256"
                                    },
                                    {
                                      "internalType": "bytes32",
                                      "name": "conduitKey",
                                      "type": "bytes32"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name": "counter",
                                      "type": "uint256"
                                    }
                                  ],
                                  "internalType": "struct OrderComponents",
                                  "name": "order",
                                  "type": "tuple"
                                }
                              ],
                              "name": "getOrderHash",
                              "outputs": [
                                {
                                  "internalType": "bytes32",
                                  "name": "orderHash",
                                  "type": "bytes32"
                                }
                              ],
                              "stateMutability": "view",
                              "type": "function"
                            },
                            {
                              "inputs": [
                                {
                                  "internalType": "bytes32",
                                  "name": "orderHash",
                                  "type": "bytes32"
                                }
                              ],
                              "name": "getOrderStatus",
                              "outputs": [
                                {
                                  "internalType": "bool",
                                  "name": "isValidated",
                                  "type": "bool"
                                },
                                {
                                  "internalType": "bool",
                                  "name": "isCancelled",
                                  "type": "bool"
                                },
                                {
                                  "internalType": "uint256",
                                  "name": "totalFilled",
                                  "type": "uint256"
                                },
                                {
                                  "internalType": "uint256",
                                  "name": "totalSize",
                                  "type": "uint256"
                                }
                              ],
                              "stateMutability": "view",
                              "type": "function"
                            },
                            {
                              "inputs": [],
                              "name": "incrementCounter",
                              "outputs": [
                                {
                                  "internalType": "uint256",
                                  "name": "newCounter",
                                  "type": "uint256"
                                }
                              ],
                              "stateMutability": "nonpayable",
                              "type": "function"
                            },
                            {
                              "inputs": [],
                              "name": "information",
                              "outputs": [
                                {
                                  "internalType": "string",
                                  "name": "version",
                                  "type": "string"
                                },
                                {
                                  "internalType": "bytes32",
                                  "name": "domainSeparator",
                                  "type": "bytes32"
                                },
                                {
                                  "internalType": "address",
                                  "name": "conduitController",
                                  "type": "address"
                                }
                              ],
                              "stateMutability": "view",
                              "type": "function"
                            },
                            {
                              "inputs": [
                                {
                                  "components": [
                                    {
                                      "components": [
                                        {
                                          "internalType": "address",
                                          "name": "offerer",
                                          "type": "address"
                                        },
                                        {
                                          "internalType": "address",
                                          "name": "zone",
                                          "type": "address"
                                        },
                                        {
                                          "components": [
                                            {
                                              "internalType": "enum ItemType",
                                              "name": "itemType",
                                              "type": "uint8"
                                            },
                                            {
                                              "internalType": "address",
                                              "name": "token",
                                              "type": "address"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "identifierOrCriteria",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "startAmount",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "endAmount",
                                              "type": "uint256"
                                            }
                                          ],
                                          "internalType": "struct OfferItem[]",
                                          "name": "offer",
                                          "type": "tuple[]"
                                        },
                                        {
                                          "components": [
                                            {
                                              "internalType": "enum ItemType",
                                              "name": "itemType",
                                              "type": "uint8"
                                            },
                                            {
                                              "internalType": "address",
                                              "name": "token",
                                              "type": "address"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "identifierOrCriteria",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "startAmount",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "endAmount",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "address payable",
                                              "name": "recipient",
                                              "type": "address"
                                            }
                                          ],
                                          "internalType":
                                              "struct ConsiderationItem[]",
                                          "name": "consideration",
                                          "type": "tuple[]"
                                        },
                                        {
                                          "internalType": "enum OrderType",
                                          "name": "orderType",
                                          "type": "uint8"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "startTime",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "endTime",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "bytes32",
                                          "name": "zoneHash",
                                          "type": "bytes32"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "salt",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "bytes32",
                                          "name": "conduitKey",
                                          "type": "bytes32"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name":
                                              "totalOriginalConsiderationItems",
                                          "type": "uint256"
                                        }
                                      ],
                                      "internalType": "struct OrderParameters",
                                      "name": "parameters",
                                      "type": "tuple"
                                    },
                                    {
                                      "internalType": "uint120",
                                      "name": "numerator",
                                      "type": "uint120"
                                    },
                                    {
                                      "internalType": "uint120",
                                      "name": "denominator",
                                      "type": "uint120"
                                    },
                                    {
                                      "internalType": "bytes",
                                      "name": "signature",
                                      "type": "bytes"
                                    },
                                    {
                                      "internalType": "bytes",
                                      "name": "extraData",
                                      "type": "bytes"
                                    }
                                  ],
                                  "internalType": "struct AdvancedOrder[]",
                                  "name": "advancedOrders",
                                  "type": "tuple[]"
                                },
                                {
                                  "components": [
                                    {
                                      "internalType": "uint256",
                                      "name": "orderIndex",
                                      "type": "uint256"
                                    },
                                    {
                                      "internalType": "enum Side",
                                      "name": "side",
                                      "type": "uint8"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name": "index",
                                      "type": "uint256"
                                    },
                                    {
                                      "internalType": "uint256",
                                      "name": "identifier",
                                      "type": "uint256"
                                    },
                                    {
                                      "internalType": "bytes32[]",
                                      "name": "criteriaProof",
                                      "type": "bytes32[]"
                                    }
                                  ],
                                  "internalType": "struct CriteriaResolver[]",
                                  "name": "criteriaResolvers",
                                  "type": "tuple[]"
                                },
                                {
                                  "components": [
                                    {
                                      "components": [
                                        {
                                          "internalType": "uint256",
                                          "name": "orderIndex",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "itemIndex",
                                          "type": "uint256"
                                        }
                                      ],
                                      "internalType":
                                          "struct FulfillmentComponent[]",
                                      "name": "offerComponents",
                                      "type": "tuple[]"
                                    },
                                    {
                                      "components": [
                                        {
                                          "internalType": "uint256",
                                          "name": "orderIndex",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "itemIndex",
                                          "type": "uint256"
                                        }
                                      ],
                                      "internalType":
                                          "struct FulfillmentComponent[]",
                                      "name": "considerationComponents",
                                      "type": "tuple[]"
                                    }
                                  ],
                                  "internalType": "struct Fulfillment[]",
                                  "name": "fulfillments",
                                  "type": "tuple[]"
                                }
                              ],
                              "name": "matchAdvancedOrders",
                              "outputs": [
                                {
                                  "components": [
                                    {
                                      "components": [
                                        {
                                          "internalType": "enum ItemType",
                                          "name": "itemType",
                                          "type": "uint8"
                                        },
                                        {
                                          "internalType": "address",
                                          "name": "token",
                                          "type": "address"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "identifier",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "amount",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "address payable",
                                          "name": "recipient",
                                          "type": "address"
                                        }
                                      ],
                                      "internalType": "struct ReceivedItem",
                                      "name": "item",
                                      "type": "tuple"
                                    },
                                    {
                                      "internalType": "address",
                                      "name": "offerer",
                                      "type": "address"
                                    },
                                    {
                                      "internalType": "bytes32",
                                      "name": "conduitKey",
                                      "type": "bytes32"
                                    }
                                  ],
                                  "internalType": "struct Execution[]",
                                  "name": "executions",
                                  "type": "tuple[]"
                                }
                              ],
                              "stateMutability": "payable",
                              "type": "function"
                            },
                            {
                              "inputs": [
                                {
                                  "components": [
                                    {
                                      "components": [
                                        {
                                          "internalType": "address",
                                          "name": "offerer",
                                          "type": "address"
                                        },
                                        {
                                          "internalType": "address",
                                          "name": "zone",
                                          "type": "address"
                                        },
                                        {
                                          "components": [
                                            {
                                              "internalType": "enum ItemType",
                                              "name": "itemType",
                                              "type": "uint8"
                                            },
                                            {
                                              "internalType": "address",
                                              "name": "token",
                                              "type": "address"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "identifierOrCriteria",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "startAmount",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "endAmount",
                                              "type": "uint256"
                                            }
                                          ],
                                          "internalType": "struct OfferItem[]",
                                          "name": "offer",
                                          "type": "tuple[]"
                                        },
                                        {
                                          "components": [
                                            {
                                              "internalType": "enum ItemType",
                                              "name": "itemType",
                                              "type": "uint8"
                                            },
                                            {
                                              "internalType": "address",
                                              "name": "token",
                                              "type": "address"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "identifierOrCriteria",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "startAmount",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "endAmount",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "address payable",
                                              "name": "recipient",
                                              "type": "address"
                                            }
                                          ],
                                          "internalType":
                                              "struct ConsiderationItem[]",
                                          "name": "consideration",
                                          "type": "tuple[]"
                                        },
                                        {
                                          "internalType": "enum OrderType",
                                          "name": "orderType",
                                          "type": "uint8"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "startTime",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "endTime",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "bytes32",
                                          "name": "zoneHash",
                                          "type": "bytes32"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "salt",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "bytes32",
                                          "name": "conduitKey",
                                          "type": "bytes32"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name":
                                              "totalOriginalConsiderationItems",
                                          "type": "uint256"
                                        }
                                      ],
                                      "internalType": "struct OrderParameters",
                                      "name": "parameters",
                                      "type": "tuple"
                                    },
                                    {
                                      "internalType": "bytes",
                                      "name": "signature",
                                      "type": "bytes"
                                    }
                                  ],
                                  "internalType": "struct Order[]",
                                  "name": "orders",
                                  "type": "tuple[]"
                                },
                                {
                                  "components": [
                                    {
                                      "components": [
                                        {
                                          "internalType": "uint256",
                                          "name": "orderIndex",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "itemIndex",
                                          "type": "uint256"
                                        }
                                      ],
                                      "internalType":
                                          "struct FulfillmentComponent[]",
                                      "name": "offerComponents",
                                      "type": "tuple[]"
                                    },
                                    {
                                      "components": [
                                        {
                                          "internalType": "uint256",
                                          "name": "orderIndex",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "itemIndex",
                                          "type": "uint256"
                                        }
                                      ],
                                      "internalType":
                                          "struct FulfillmentComponent[]",
                                      "name": "considerationComponents",
                                      "type": "tuple[]"
                                    }
                                  ],
                                  "internalType": "struct Fulfillment[]",
                                  "name": "fulfillments",
                                  "type": "tuple[]"
                                }
                              ],
                              "name": "matchOrders",
                              "outputs": [
                                {
                                  "components": [
                                    {
                                      "components": [
                                        {
                                          "internalType": "enum ItemType",
                                          "name": "itemType",
                                          "type": "uint8"
                                        },
                                        {
                                          "internalType": "address",
                                          "name": "token",
                                          "type": "address"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "identifier",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "amount",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "address payable",
                                          "name": "recipient",
                                          "type": "address"
                                        }
                                      ],
                                      "internalType": "struct ReceivedItem",
                                      "name": "item",
                                      "type": "tuple"
                                    },
                                    {
                                      "internalType": "address",
                                      "name": "offerer",
                                      "type": "address"
                                    },
                                    {
                                      "internalType": "bytes32",
                                      "name": "conduitKey",
                                      "type": "bytes32"
                                    }
                                  ],
                                  "internalType": "struct Execution[]",
                                  "name": "executions",
                                  "type": "tuple[]"
                                }
                              ],
                              "stateMutability": "payable",
                              "type": "function"
                            },
                            {
                              "inputs": [],
                              "name": "name",
                              "outputs": [
                                {
                                  "internalType": "string",
                                  "name": "contractName",
                                  "type": "string"
                                }
                              ],
                              "stateMutability": "pure",
                              "type": "function"
                            },
                            {
                              "inputs": [
                                {
                                  "components": [
                                    {
                                      "components": [
                                        {
                                          "internalType": "address",
                                          "name": "offerer",
                                          "type": "address"
                                        },
                                        {
                                          "internalType": "address",
                                          "name": "zone",
                                          "type": "address"
                                        },
                                        {
                                          "components": [
                                            {
                                              "internalType": "enum ItemType",
                                              "name": "itemType",
                                              "type": "uint8"
                                            },
                                            {
                                              "internalType": "address",
                                              "name": "token",
                                              "type": "address"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "identifierOrCriteria",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "startAmount",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "endAmount",
                                              "type": "uint256"
                                            }
                                          ],
                                          "internalType": "struct OfferItem[]",
                                          "name": "offer",
                                          "type": "tuple[]"
                                        },
                                        {
                                          "components": [
                                            {
                                              "internalType": "enum ItemType",
                                              "name": "itemType",
                                              "type": "uint8"
                                            },
                                            {
                                              "internalType": "address",
                                              "name": "token",
                                              "type": "address"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "identifierOrCriteria",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "startAmount",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "uint256",
                                              "name": "endAmount",
                                              "type": "uint256"
                                            },
                                            {
                                              "internalType": "address payable",
                                              "name": "recipient",
                                              "type": "address"
                                            }
                                          ],
                                          "internalType":
                                              "struct ConsiderationItem[]",
                                          "name": "consideration",
                                          "type": "tuple[]"
                                        },
                                        {
                                          "internalType": "enum OrderType",
                                          "name": "orderType",
                                          "type": "uint8"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "startTime",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "endTime",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "bytes32",
                                          "name": "zoneHash",
                                          "type": "bytes32"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name": "salt",
                                          "type": "uint256"
                                        },
                                        {
                                          "internalType": "bytes32",
                                          "name": "conduitKey",
                                          "type": "bytes32"
                                        },
                                        {
                                          "internalType": "uint256",
                                          "name":
                                              "totalOriginalConsiderationItems",
                                          "type": "uint256"
                                        }
                                      ],
                                      "internalType": "struct OrderParameters",
                                      "name": "parameters",
                                      "type": "tuple"
                                    },
                                    {
                                      "internalType": "bytes",
                                      "name": "signature",
                                      "type": "bytes"
                                    }
                                  ],
                                  "internalType": "struct Order[]",
                                  "name": "orders",
                                  "type": "tuple[]"
                                }
                              ],
                              "name": "validate",
                              "outputs": [
                                {
                                  "internalType": "bool",
                                  "name": "validated",
                                  "type": "bool"
                                }
                              ],
                              "stateMutability": "nonpayable",
                              "type": "function"
                            }
                          ],
                          fontSize: 18,
                        ),
                      )
                    else
                      Text(
                        decoded,
                        style: const TextStyle(fontSize: 16.0),
                      ),
                  ],
                ),
              ),
            ),
            Row(
              children: [
                Expanded(
                  child: TextButton(
                    style: TextButton.styleFrom(
                      foregroundColor: Colors.white,
                      backgroundColor: const Color(0xff007bff),
                    ),
                    onPressed: () async {
                      if (await authenticate(context)) {
                        onConfirm();
                      } else {
                        onReject();
                      }
                    },
                    child: Text(
                      AppLocalizations.of(context).sign,
                      style: const TextStyle(fontSize: 18),
                    ),
                  ),
                ),
                const SizedBox(width: 16.0),
                Expanded(
                  child: TextButton(
                    style: TextButton.styleFrom(
                      foregroundColor: Colors.white,
                      backgroundColor: const Color(0xff007bff),
                    ),
                    onPressed: onReject,
                    child: Text(
                      AppLocalizations.of(context).reject,
                      style: const TextStyle(fontSize: 18),
                    ),
                  ),
                ),
              ],
            ),
          ],
        ),
      ),
    ),
    canDismiss: false,
  );
}

Future<Map> decodeAbi(String txData) async {
  final js = await rootBundle.loadString('js/abi-decoder.js');
  JavascriptRuntime javaScriptRuntime = getJavascriptRuntime();

  javaScriptRuntime.evaluate(js);
  javaScriptRuntime.evaluate('''abiDecoder.addABI($oneInchAbi)''');
  javaScriptRuntime.evaluate('''abiDecoder.addABI($uniswapAbi2)''');
  javaScriptRuntime.evaluate('''abiDecoder.addABI($uniswapAbi)''');
  javaScriptRuntime.evaluate('''abiDecoder.addABI($wrappedEthAbi)''');
  javaScriptRuntime.evaluate('''abiDecoder.addABI($erc20Abi)''');
  javaScriptRuntime.evaluate('''abiDecoder.addABI($erc721Abi)''');
  javaScriptRuntime.evaluate('''abiDecoder.addABI($tokenSaleAbi)''');
  javaScriptRuntime.evaluate('''abiDecoder.addABI($ensResolver)''');
  javaScriptRuntime.evaluate('''abiDecoder.addABI($ensInterface)''');
  javaScriptRuntime.evaluate('''abiDecoder.addABI($erc1155Abi)''');
  javaScriptRuntime.evaluate('''abiDecoder.addABI($unstoppableDomainAbi)''');

  final decode = javaScriptRuntime
      .evaluate('JSON.stringify(abiDecoder.decodeMethod("$txData"))');
  if (decode.stringResult == 'undefined') return null;
  Map result_ = json.decode(decode.stringResult);
  return result_;
}

solidityFunctionSig(String methodId) {
  return '0x${sha3(methodId).substring(0, 8)}';
}

Uint8List txDataToUintList(String txData) {
  if (txData == null) return null;
  return isHexString(txData) ? hexToBytes(txData) : ascii.encode(txData);
}

String ellipsify({String str, int maxLength}) {
  maxLength ??= 10;
  if (maxLength % 2 != 0) {
    maxLength++;
  }
  if (str.length <= maxLength) return str;
  // get first four and last four characters
  final first = str.substring(0, maxLength ~/ 2);
  final last = str.substring((str.length - maxLength / 2).toInt(), str.length);
  return '$first...$last';
}

Future<void> enableScreenShot() async {
  if (kDebugMode) return;
  if (Platform.isAndroid) {
    await FlutterWindowManager.clearFlags(
      FlutterWindowManager.FLAG_SECURE,
    );
  }
}

Future<void> disEnableScreenShot() async {
  if (kDebugMode) return;
  if (Platform.isAndroid) {
    await FlutterWindowManager.addFlags(
      FlutterWindowManager.FLAG_SECURE,
    );
  }
}

Future<Map> processEIP681(String eip681URL) async {
  Map parsedUrl;

  try {
    parsedUrl = EIP681.parse(eip681URL);

    final chainId = int.parse(parsedUrl['chainId'] ?? '1');

    final cryptoBlock = getEthereumDetailsFromChainId(chainId);

    Map sendToken = {
      "rpc": cryptoBlock['rpc'],
      "coinType": cryptoBlock['coinType'],
      "chainId": chainId,
    };

    bool isContractTransfer =
        (parsedUrl['functionName'] ?? '').toString().toLowerCase() ==
            'transfer';
    final networkName = getEVMBlockchains().keys.firstWhere((element) {
      return (getEVMBlockchains()[element]['chainId'] == chainId);
    });
    if (isContractTransfer) {
      Map erc20Details = await getERC20TokenNameSymbolDecimal(
        contractAddress: parsedUrl['target_address'],
        rpc: sendToken['rpc'],
      );

      if (erc20Details.isEmpty) throw Exception('Unable to get token Details');

      final parseUrlUint256 =
          BigInt.parse(parsedUrl['parameters']['uint256'] ?? '0');

      final parseUrlUint256Decimals =
          pow(10, double.parse(erc20Details['decimals']));

      sendToken.addAll({
        "name": erc20Details['name'],
        "symbol": erc20Details['symbol'],
        "contractAddress": parsedUrl['target_address'],
        "recipient": parsedUrl['parameters']['address'],
        "network": networkName,
        'amount':
            (parseUrlUint256 / BigInt.from(parseUrlUint256Decimals)).toString()
      });
    } else {
      final parsedUrlValue = parsedUrl['parameters']['value'];
      final parsedUrlAmount = parsedUrl['parameters']['amount'];
      String amount = '0';

      if (parsedUrlValue != null) {
        final etherBigInt = BigInt.from(pow(10, etherDecimals));
        amount = (BigInt.parse(parsedUrlValue) / etherBigInt).toString();
      } else if (parsedUrlAmount != null) {
        amount = parsedUrl['parameters']['amount'];
      }

      sendToken.addAll({
        "name": networkName,
        "symbol": cryptoBlock['symbol'],
        "default": cryptoBlock['symbol'],
        "recipient": parsedUrl['target_address'],
        'amount': amount
      });
    }

    return {'success': true, 'msg': sendToken};
  } catch (e) {
    return {'success': false, 'msg': e.toString()};
  }
}

Future<Map> getERC20TokenDetails({
  String contractAddress,
  String rpc,
}) async {
  final client = web3.Web3Client(
    rpc,
    Client(),
  );

  final contract = web3.DeployedContract(
    web3.ContractAbi.fromJson(erc20Abi, ''),
    web3.EthereumAddress.fromHex(contractAddress),
  );

  final nameFunction = contract.function('name');
  final symbolFunction = contract.function('symbol');
  final decimalsFunction = contract.function('decimals');

  final name =
      await client.call(contract: contract, function: nameFunction, params: []);

  final symbol = await client
      .call(contract: contract, function: symbolFunction, params: []);
  final decimals = await client
      .call(contract: contract, function: decimalsFunction, params: []);

  return {
    'name': name.first,
    'symbol': symbol.first,
    'decimals': decimals.first.toString()
  };
}

Future<Map> getERC20TokenNameSymbolDecimal({
  String contractAddress,
  String rpc,
  int chainId,
}) async {
  final pref = Hive.box(secureStorageKey);
  String tokenDetailsKey = await contractDetailsKey(
    rpc,
    contractAddress,
  );

  String tokenDetailsSaved = pref.get(tokenDetailsKey);
  Map erc20TokenDetails = {};
  try {
    erc20TokenDetails = await getERC20TokenDetails(
      contractAddress: contractAddress,
      rpc: rpc,
    );
  } catch (e) {
    if (tokenDetailsSaved != null) {
      erc20TokenDetails = json.decode(tokenDetailsSaved);
    }
  }

  await pref.put(tokenDetailsKey, json.encode(erc20TokenDetails));
  return erc20TokenDetails;
}

selectImage({
  BuildContext context,
  Function(XFile) onSelect,
}) {
  AwesomeDialog(
    context: context,
    dialogType: DialogType.INFO,
    buttonsBorderRadius: const BorderRadius.all(Radius.circular(10)),
    headerAnimationLoop: false,
    animType: AnimType.BOTTOMSLIDE,
    closeIcon: const Icon(
      Icons.close,
    ),
    title: AppLocalizations.of(context).chooseImageSource,
    desc: AppLocalizations.of(context).imageSource,
    showCloseIcon: true,
    btnOkText: AppLocalizations.of(context).gallery,
    btnCancelText: AppLocalizations.of(context).camera,
    btnCancelOnPress: () async {
      XFile file = await ImagePicker().pickImage(source: ImageSource.camera);
      if (file == null) return;
      onSelect(file);
    },
    btnCancelColor: Colors.blue,
    btnOkColor: Colors.blue,
    btnOkOnPress: () async {
      XFile file = await ImagePicker().pickImage(source: ImageSource.gallery);
      if (file == null) return;
      onSelect(file);
    },
  ).show();
}
