import type { LoginPayload } from "./types.js";
/**
 * Create an EIP-4361 & CAIP-122 compliant message to sign based on the login payload
 * @param payload - The login payload containing the necessary information.
 * @returns The generated login message.
 * @internal
 */
export declare function createLoginMessage(payload: LoginPayload): string;
//# sourceMappingURL=create-login-message.d.ts.map