import { EngineTypes, RelayerTypes } from "@walletconnect/types";
export declare function parseRelayParams(params: any, delimiter?: string): RelayerTypes.ProtocolOptions;
export declare function parseUri(str: string): EngineTypes.UriParameters;
export declare function parseTopic(topic: string): string;
export declare function formatRelayParams(relay: RelayerTypes.ProtocolOptions, delimiter?: string): any;
export declare function formatUri(params: EngineTypes.UriParameters): string;
export declare function getLinkModeURL(universalLink: string, topic: string, encodedEnvelope: string): string;
//# sourceMappingURL=uri.d.ts.map