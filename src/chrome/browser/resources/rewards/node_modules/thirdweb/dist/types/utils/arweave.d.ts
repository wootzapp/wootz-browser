export type ResolveArweaveSchemeOptions = {
    uri: string;
    gatewayUrl?: string;
};
/**
 * Resolves the scheme of a given Arweave URI and returns the corresponding URL.
 * @param options - The options object containing the Arweave URI
 * @returns The resolved URL
 * @throws Error if the URI scheme is invalid.
 * @example
 * ```ts
 * import { resolveArweaveScheme } from "thirdweb/storage";
 * const url = resolveArweaveScheme({ uri: "ar://<fileId>" });
 * ```
 * @storage
 */
export declare function resolveArweaveScheme(options: ResolveArweaveSchemeOptions): string;
//# sourceMappingURL=arweave.d.ts.map