import { SignClientTypes } from "@walletconnect/types";
export declare const SIGN_CLIENT_PROTOCOL = "wc";
export declare const SIGN_CLIENT_VERSION = 2;
export declare const SIGN_CLIENT_CONTEXT = "client";
export declare const SIGN_CLIENT_STORAGE_PREFIX: string;
export declare const SIGN_CLIENT_DEFAULT: {
    name: string;
    logger: string;
    controller: boolean;
    relayUrl: string;
};
export declare const SIGN_CLIENT_EVENTS: Record<SignClientTypes.Event, SignClientTypes.Event>;
export declare const SIGN_CLIENT_STORAGE_OPTIONS: {
    database: string;
};
export declare const WALLETCONNECT_DEEPLINK_CHOICE = "WALLETCONNECT_DEEPLINK_CHOICE";
//# sourceMappingURL=client.d.ts.map