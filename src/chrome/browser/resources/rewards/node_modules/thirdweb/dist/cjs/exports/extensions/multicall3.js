"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.tryBlockAndAggregate = exports.tryAggregate = exports.blockAndAggregate = exports.aggregate3Value = exports.aggregate3 = exports.aggregate = exports.getLastBlockHash = exports.getEthBalance = exports.getCurrentBlockTimestamp = exports.getCurrentBlockGasLimit = exports.getCurrentBlockDifficulty = exports.getCurrentBlockCoinbase = exports.getChainId = exports.getBlockNumber = exports.getBlockHash = exports.getBasefee = void 0;
// READ
var getBasefee_js_1 = require("../../extensions/multicall3/__generated__/IMulticall3/read/getBasefee.js");
Object.defineProperty(exports, "getBasefee", { enumerable: true, get: function () { return getBasefee_js_1.getBasefee; } });
var getBlockHash_js_1 = require("../../extensions/multicall3/__generated__/IMulticall3/read/getBlockHash.js");
Object.defineProperty(exports, "getBlockHash", { enumerable: true, get: function () { return getBlockHash_js_1.getBlockHash; } });
var getBlockNumber_js_1 = require("../../extensions/multicall3/__generated__/IMulticall3/read/getBlockNumber.js");
Object.defineProperty(exports, "getBlockNumber", { enumerable: true, get: function () { return getBlockNumber_js_1.getBlockNumber; } });
var getChainId_js_1 = require("../../extensions/multicall3/__generated__/IMulticall3/read/getChainId.js");
Object.defineProperty(exports, "getChainId", { enumerable: true, get: function () { return getChainId_js_1.getChainId; } });
var getCurrentBlockCoinbase_js_1 = require("../../extensions/multicall3/__generated__/IMulticall3/read/getCurrentBlockCoinbase.js");
Object.defineProperty(exports, "getCurrentBlockCoinbase", { enumerable: true, get: function () { return getCurrentBlockCoinbase_js_1.getCurrentBlockCoinbase; } });
var getCurrentBlockDifficulty_js_1 = require("../../extensions/multicall3/__generated__/IMulticall3/read/getCurrentBlockDifficulty.js");
Object.defineProperty(exports, "getCurrentBlockDifficulty", { enumerable: true, get: function () { return getCurrentBlockDifficulty_js_1.getCurrentBlockDifficulty; } });
var getCurrentBlockGasLimit_js_1 = require("../../extensions/multicall3/__generated__/IMulticall3/read/getCurrentBlockGasLimit.js");
Object.defineProperty(exports, "getCurrentBlockGasLimit", { enumerable: true, get: function () { return getCurrentBlockGasLimit_js_1.getCurrentBlockGasLimit; } });
var getCurrentBlockTimestamp_js_1 = require("../../extensions/multicall3/__generated__/IMulticall3/read/getCurrentBlockTimestamp.js");
Object.defineProperty(exports, "getCurrentBlockTimestamp", { enumerable: true, get: function () { return getCurrentBlockTimestamp_js_1.getCurrentBlockTimestamp; } });
var getEthBalance_js_1 = require("../../extensions/multicall3/__generated__/IMulticall3/read/getEthBalance.js");
Object.defineProperty(exports, "getEthBalance", { enumerable: true, get: function () { return getEthBalance_js_1.getEthBalance; } });
var getLastBlockHash_js_1 = require("../../extensions/multicall3/__generated__/IMulticall3/read/getLastBlockHash.js");
Object.defineProperty(exports, "getLastBlockHash", { enumerable: true, get: function () { return getLastBlockHash_js_1.getLastBlockHash; } });
// WRITE
var aggregate_js_1 = require("../../extensions/multicall3/__generated__/IMulticall3/write/aggregate.js");
Object.defineProperty(exports, "aggregate", { enumerable: true, get: function () { return aggregate_js_1.aggregate; } });
var aggregate3_js_1 = require("../../extensions/multicall3/__generated__/IMulticall3/write/aggregate3.js");
Object.defineProperty(exports, "aggregate3", { enumerable: true, get: function () { return aggregate3_js_1.aggregate3; } });
var aggregate3Value_js_1 = require("../../extensions/multicall3/__generated__/IMulticall3/write/aggregate3Value.js");
Object.defineProperty(exports, "aggregate3Value", { enumerable: true, get: function () { return aggregate3Value_js_1.aggregate3Value; } });
var blockAndAggregate_js_1 = require("../../extensions/multicall3/__generated__/IMulticall3/write/blockAndAggregate.js");
Object.defineProperty(exports, "blockAndAggregate", { enumerable: true, get: function () { return blockAndAggregate_js_1.blockAndAggregate; } });
var tryAggregate_js_1 = require("../../extensions/multicall3/__generated__/IMulticall3/write/tryAggregate.js");
Object.defineProperty(exports, "tryAggregate", { enumerable: true, get: function () { return tryAggregate_js_1.tryAggregate; } });
var tryBlockAndAggregate_js_1 = require("../../extensions/multicall3/__generated__/IMulticall3/write/tryBlockAndAggregate.js");
Object.defineProperty(exports, "tryBlockAndAggregate", { enumerable: true, get: function () { return tryBlockAndAggregate_js_1.tryBlockAndAggregate; } });
//# sourceMappingURL=multicall3.js.map