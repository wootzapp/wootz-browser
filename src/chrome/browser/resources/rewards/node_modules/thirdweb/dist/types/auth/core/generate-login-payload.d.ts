import type { AuthOptions, LoginPayload } from "./types.js";
/**
 * @auth
 */
export type GenerateLoginPayloadParams = {
    address: string;
    chainId?: number;
};
/**
 * Generates a login payload based on the provided options.
 * @param options - The authentication options.
 * @returns A function that accepts login payload parameters and returns a promise that resolves to a login payload.
 * @internal
 */
export declare function generateLoginPayload(options: AuthOptions): ({ address, chainId, }: GenerateLoginPayloadParams) => Promise<LoginPayload>;
//# sourceMappingURL=generate-login-payload.d.ts.map