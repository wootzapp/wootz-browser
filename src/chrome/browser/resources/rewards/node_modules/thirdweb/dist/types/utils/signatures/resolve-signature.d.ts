type FunctionString = `function ${string}`;
type EventString = `event ${string}`;
/**
 * Resolves a signature by converting a hexadecimal string into a function or event signature.
 * @param hexSig The hexadecimal signature to resolve.
 * @returns A promise that resolves to an object containing the function and event signatures.
 * @example
 * ```ts
 * import { resolveSignature } from "thirdweb/utils";
 * const res = await resolveSignature("0x1f931c1c");
 * console.log(res);
 * ```
 * @utils
 */
export declare function resolveSignature(hexSig: string): Promise<{
    function: FunctionString | null;
    event: EventString | null;
}>;
/**
 * Resolves the signatures of the given hexadecimal signatures.
 * @param hexSigs An array of hexadecimal signatures.
 * @returns A promise that resolves to an object containing the resolved functions and events.
 * @example
 * ```ts
 * import { resolveSignatures } from "thirdweb/utils";
 * const res = await resolveSignatures(["0x1f931c1c", "0x1f931c1c"]);
 * console.log(res);
 * ```
 * @utils
 */
export declare function resolveSignatures(hexSigs: string[]): Promise<{
    functions: FunctionString[];
    events: EventString[];
}>;
/**
 * @internal
 */
export declare function clearCache(): void;
export {};
//# sourceMappingURL=resolve-signature.d.ts.map