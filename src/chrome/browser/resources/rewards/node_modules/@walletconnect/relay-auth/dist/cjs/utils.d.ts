import { IridiumJWTData, IridiumJWTDecoded, IridiumJWTSigned } from "./types";
export declare function decodeJSON(str: string): any;
export declare function encodeJSON(val: any): string;
export declare function encodeIss(publicKey: Uint8Array): string;
export declare function decodeIss(issuer: string): Uint8Array;
export declare function encodeSig(bytes: Uint8Array): string;
export declare function decodeSig(encoded: string): Uint8Array;
export declare function encodeData(params: IridiumJWTData): Uint8Array;
export declare function decodeData(data: Uint8Array): IridiumJWTData;
export declare function encodeJWT(params: IridiumJWTSigned): string;
export declare function decodeJWT(jwt: string): IridiumJWTDecoded;
//# sourceMappingURL=utils.d.ts.map