import { WalletLinkEventData, WalletLinkResponseEventData } from '../type/WalletLinkEventData';
import { WalletLinkSession } from '../type/WalletLinkSession';
export interface WalletLinkConnectionUpdateListener {
    linkedUpdated: (linked: boolean) => void;
    handleWeb3ResponseMessage: (message: WalletLinkResponseEventData) => void;
    chainUpdated: (chainId: string, jsonRpcUrl: string) => void;
    accountUpdated: (selectedAddress: string) => void;
    metadataUpdated: (key: string, metadataValue: string) => void;
    resetAndReload: () => void;
}
interface WalletLinkConnectionParams {
    session: WalletLinkSession;
    linkAPIUrl: string;
    listener: WalletLinkConnectionUpdateListener;
    WebSocketClass?: typeof WebSocket;
}
/**
 * Coinbase Wallet Connection
 */
export declare class WalletLinkConnection {
    private destroyed;
    private lastHeartbeatResponse;
    private nextReqId;
    private readonly session;
    private listener?;
    private cipher;
    private ws;
    private http;
    /**
     * Constructor
     * @param session Session
     * @param linkAPIUrl Coinbase Wallet link server URL
     * @param listener WalletLinkConnectionUpdateListener
     * @param [WebSocketClass] Custom WebSocket implementation
     */
    constructor({ session, linkAPIUrl, listener, WebSocketClass, }: WalletLinkConnectionParams);
    /**
     * Make a connection to the server
     */
    connect(): void;
    /**
     * Terminate connection, and mark as destroyed. To reconnect, create a new
     * instance of WalletSDKConnection
     */
    destroy(): void;
    get isDestroyed(): boolean;
    /**
     * true if connected and authenticated, else false
     * runs listener when connected status changes
     */
    private _connected;
    private get connected();
    private set connected(value);
    /**
     * Execute once when connected
     */
    private onceConnected?;
    private setOnceConnected;
    /**
     * true if linked (a guest has joined before)
     * runs listener when linked status changes
     */
    private _linked;
    private get linked();
    private set linked(value);
    /**
     * Execute once when linked
     */
    private onceLinked?;
    private setOnceLinked;
    private handleIncomingEvent;
    private shouldFetchUnseenEventsOnConnect;
    checkUnseenEvents(): Promise<void>;
    private fetchUnseenEventsAPI;
    /**
     * Set session metadata in SessionConfig object
     * @param key
     * @param value
     * @returns a Promise that completes when successful
     */
    setSessionMetadata(key: string, value: string | null): Promise<void>;
    /**
     * Publish an event and emit event ID when successful
     * @param event event name
     * @param unencryptedData unencrypted event data
     * @param callWebhook whether the webhook should be invoked
     * @returns a Promise that emits event ID when successful
     */
    publishEvent(event: string, unencryptedData: WalletLinkEventData, callWebhook?: boolean): Promise<string>;
    private sendData;
    private updateLastHeartbeat;
    private heartbeat;
    private requestResolutions;
    private makeRequest;
    private authenticate;
    private sendIsLinked;
    private sendGetSessionConfig;
    private handleSessionMetadataUpdated;
    private handleDestroyed;
    private handleAccountUpdated;
    private handleMetadataUpdated;
    private handleWalletUsernameUpdated;
    private handleAppVersionUpdated;
    private handleChainUpdated;
}
export {};
