import { parseAbiItem } from "abitype";
import { encodeEventTopics, toEventHash } from "viem";
import { isAbiEvent } from "./utils.js";
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
export function prepareEvent(options) {
    const { signature } = options;
    let resolvedSignature;
    if (isAbiEvent(signature)) {
        resolvedSignature = signature;
    }
    else {
        resolvedSignature = parseAbiItem(signature);
    }
    return {
        abiEvent: resolvedSignature,
        hash: toEventHash(resolvedSignature),
        // @ts-expect-error - TODO: investiagte why this complains, it works fine however
        topics: encodeEventTopics({
            abi: [resolvedSignature],
            args: options.filters,
        }),
    };
}
//# sourceMappingURL=prepare-event.js.map