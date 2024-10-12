import { type AbiEvent, type ParseAbiItem } from "abitype";
import { type Hex } from "viem";
import type { AbiEventParametersToPrimitiveTypes } from "./types.js";
type ParseEvent<event extends AbiEvent | string> = event extends AbiEvent ? event : event extends string ? event extends `event ${string}` ? ParseAbiItem<event> : AbiEvent : never;
type EventFilters<abiEvent extends AbiEvent> = AbiEventParametersToPrimitiveTypes<abiEvent["inputs"]>;
export type PrepareEventOptions<TSignature extends `event ${string}` | AbiEvent> = {
    signature: TSignature;
    filters?: Readonly<EventFilters<ParseEvent<TSignature>>>;
};
export type PreparedEvent<abiEvent extends AbiEvent> = {
    abiEvent: abiEvent;
    hash: Hex;
    topics: Hex[];
};
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
export declare function prepareEvent<TSignature extends `event ${string}` | AbiEvent>(options: PrepareEventOptions<TSignature>): PreparedEvent<ParseEvent<TSignature>>;
export {};
//# sourceMappingURL=prepare-event.d.ts.map