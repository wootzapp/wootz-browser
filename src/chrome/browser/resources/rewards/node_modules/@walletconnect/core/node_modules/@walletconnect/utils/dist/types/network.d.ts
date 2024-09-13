export declare function isOnline(): Promise<boolean>;
export declare function getBrowserOnlineStatus(): boolean;
export declare function getReactNativeOnlineStatus(): Promise<boolean>;
export declare function getNodeOnlineStatus(): boolean;
export declare function subscribeToNetworkChange(callbackHandler: (connected: boolean) => void): void;
export declare function subscribeToBrowserNetworkChange(callbackHandler: (connected: boolean) => void): void;
export declare function subscribeToReactNativeNetworkChange(callbackHandler: (connected: boolean) => void): void;
//# sourceMappingURL=network.d.ts.map