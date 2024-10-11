interface Tag<T extends string, RealType> {
    __tag__: T;
    __realType__: RealType;
}
export type OpaqueType<T extends string, U> = U & Tag<T, U>;
export declare function OpaqueType<T extends Tag<string, unknown>>(): (value: T extends Tag<string, infer U> ? U : never) => T;
export type HexString = OpaqueType<'HexString', string>;
export declare const HexString: (value: string) => HexString;
export type AddressString = OpaqueType<'AddressString', string>;
export declare const AddressString: (value: string) => AddressString;
export type BigIntString = OpaqueType<'BigIntString', string>;
export declare const BigIntString: (value: string) => BigIntString;
export type IntNumber = OpaqueType<'IntNumber', number>;
export declare function IntNumber(num: number): IntNumber;
export type RegExpString = OpaqueType<'RegExpString', string>;
export declare const RegExpString: (value: string) => RegExpString;
export type Callback<T> = (err: Error | null, result: T | null) => void;
export type Chain = {
    id: number;
    rpcUrl?: string;
};
export {};
