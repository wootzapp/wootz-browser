/**
 * Override `Register` to customize type options
 *
 * @example
 * import { type EIP1193Provider } from './eip1193'
 *
 * declare module 'mipd' {
 *   export interface Register {
 *     provider: EIP1193Provider
 *   }
 * }
 */
export interface Register {
}
export type DefaultRegister = {
    /** The EIP-1193 Provider. */
    provider: import('viem').EIP1193Provider;
    /** Reverse Domain Name Notation (rDNS) of the Wallet Provider. */
    rdns: 'com.coinbase' | 'com.enkrypt' | 'io.metamask' | 'io.zerion';
};
export type ResolvedRegister = {
    /** The EIP-1193 Provider. */
    provider: Register extends {
        provider: infer provider extends DefaultRegister['provider'];
    } ? provider : DefaultRegister['provider'];
    /** Reverse Domain Name Notation (rDNS) of the Wallet Provider. */
    rdns: Register extends {
        rdns: infer rdns extends string;
    } ? rdns : DefaultRegister['rdns'] | (string & {});
};
export type EIP1193Provider = ResolvedRegister['provider'];
export type Rdns = ResolvedRegister['rdns'];
//# sourceMappingURL=register.d.ts.map