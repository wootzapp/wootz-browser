"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.getBytecode = getBytecode;
const eth_getCode_js_1 = require("../../rpc/actions/eth_getCode.js");
const rpc_js_1 = require("../../rpc/rpc.js");
const BYTECODE_CACHE = new WeakMap();
/**
 * Retrieves the bytecode of a contract.
 * @param contract - The ThirdwebContract instance.
 * @returns A Promise that resolves to the bytecode of the contract.
 * @example
 * ```ts
 * import { getBytecode } from "thirdweb/contract";
 * const bytecode = await getBytecode(contract);
 * ```
 * @contract
 */
function getBytecode(contract) {
    if (BYTECODE_CACHE.has(contract)) {
        return BYTECODE_CACHE.get(contract);
    }
    const prom = (async () => {
        const rpcRequest = (0, rpc_js_1.getRpcClient)(contract);
        const result = await (0, eth_getCode_js_1.eth_getCode)(rpcRequest, {
            address: contract.address,
            blockTag: "latest",
        });
        if (result === "0x") {
            BYTECODE_CACHE.delete(contract);
        }
        return result;
    })();
    BYTECODE_CACHE.set(contract, prom);
    return prom;
}
//# sourceMappingURL=get-bytecode.js.map