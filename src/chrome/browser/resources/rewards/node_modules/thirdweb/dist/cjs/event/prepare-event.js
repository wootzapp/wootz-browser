"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.prepareEvent = prepareEvent;
const abitype_1 = require("abitype");
const viem_1 = require("viem");
const utils_js_1 = require("./utils.js");
/**
 * Prepares an event by parsing the signature, generating the event hash, and encoding the event topics.
 * @param options - The options for preparing the event.
 * @returns The prepared event object.
 * @example
 * ```ts
 * import { prepareEvent } from "thirdweb";
 * const myEvent = prepareEvent({
 *  signature: "event MyEvent(uint256 myArg)",
 * });
 * ```
 * @contract
 */
function prepareEvent(options) {
    const { signature } = options;
    let resolvedSignature;
    if ((0, utils_js_1.isAbiEvent)(signature)) {
        resolvedSignature = signature;
    }
    else {
        resolvedSignature = (0, abitype_1.parseAbiItem)(signature);
    }
    return {
        abiEvent: resolvedSignature,
        hash: (0, viem_1.toEventHash)(resolvedSignature),
        // @ts-expect-error - TODO: investiagte why this complains, it works fine however
        topics: (0, viem_1.encodeEventTopics)({
            abi: [resolvedSignature],
            args: options.filters,
        }),
    };
}
//# sourceMappingURL=prepare-event.js.map