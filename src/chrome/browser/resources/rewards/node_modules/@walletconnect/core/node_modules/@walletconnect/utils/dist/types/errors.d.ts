export declare type SdkErrorKey = keyof typeof SDK_ERRORS;
export declare type InternalErrorKey = keyof typeof INTERNAL_ERRORS;
export declare const SDK_ERRORS: {
    INVALID_METHOD: {
        message: string;
        code: number;
    };
    INVALID_EVENT: {
        message: string;
        code: number;
    };
    INVALID_UPDATE_REQUEST: {
        message: string;
        code: number;
    };
    INVALID_EXTEND_REQUEST: {
        message: string;
        code: number;
    };
    INVALID_SESSION_SETTLE_REQUEST: {
        message: string;
        code: number;
    };
    UNAUTHORIZED_METHOD: {
        message: string;
        code: number;
    };
    UNAUTHORIZED_EVENT: {
        message: string;
        code: number;
    };
    UNAUTHORIZED_UPDATE_REQUEST: {
        message: string;
        code: number;
    };
    UNAUTHORIZED_EXTEND_REQUEST: {
        message: string;
        code: number;
    };
    USER_REJECTED: {
        message: string;
        code: number;
    };
    USER_REJECTED_CHAINS: {
        message: string;
        code: number;
    };
    USER_REJECTED_METHODS: {
        message: string;
        code: number;
    };
    USER_REJECTED_EVENTS: {
        message: string;
        code: number;
    };
    UNSUPPORTED_CHAINS: {
        message: string;
        code: number;
    };
    UNSUPPORTED_METHODS: {
        message: string;
        code: number;
    };
    UNSUPPORTED_EVENTS: {
        message: string;
        code: number;
    };
    UNSUPPORTED_ACCOUNTS: {
        message: string;
        code: number;
    };
    UNSUPPORTED_NAMESPACE_KEY: {
        message: string;
        code: number;
    };
    USER_DISCONNECTED: {
        message: string;
        code: number;
    };
    SESSION_SETTLEMENT_FAILED: {
        message: string;
        code: number;
    };
    WC_METHOD_UNSUPPORTED: {
        message: string;
        code: number;
    };
};
export declare const INTERNAL_ERRORS: {
    NOT_INITIALIZED: {
        message: string;
        code: number;
    };
    NO_MATCHING_KEY: {
        message: string;
        code: number;
    };
    RESTORE_WILL_OVERRIDE: {
        message: string;
        code: number;
    };
    RESUBSCRIBED: {
        message: string;
        code: number;
    };
    MISSING_OR_INVALID: {
        message: string;
        code: number;
    };
    EXPIRED: {
        message: string;
        code: number;
    };
    UNKNOWN_TYPE: {
        message: string;
        code: number;
    };
    MISMATCHED_TOPIC: {
        message: string;
        code: number;
    };
    NON_CONFORMING_NAMESPACES: {
        message: string;
        code: number;
    };
};
export declare function getInternalError(key: InternalErrorKey, context?: string | number): {
    message: string;
    code: number;
};
export declare function getSdkError(key: SdkErrorKey, context?: string | number): {
    message: string;
    code: number;
};
//# sourceMappingURL=errors.d.ts.map