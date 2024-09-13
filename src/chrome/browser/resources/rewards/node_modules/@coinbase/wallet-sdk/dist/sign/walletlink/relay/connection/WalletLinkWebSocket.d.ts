import { ServerMessage } from '../type/ServerMessage';
export declare enum ConnectionState {
    DISCONNECTED = 0,
    CONNECTING = 1,
    CONNECTED = 2
}
export declare class WalletLinkWebSocket {
    private readonly WebSocketClass;
    private readonly url;
    private webSocket;
    private pendingData;
    private connectionStateListener?;
    setConnectionStateListener(listener: (_: ConnectionState) => void): void;
    private incomingDataListener?;
    setIncomingDataListener(listener: (_: ServerMessage) => void): void;
    /**
     * Constructor
     * @param url WebSocket server URL
     * @param [WebSocketClass] Custom WebSocket implementation
     */
    constructor(url: string, WebSocketClass?: typeof WebSocket);
    /**
     * Make a websocket connection
     * @returns a Promise that resolves when connected
     */
    connect(): Promise<void>;
    /**
     * Disconnect from server
     */
    disconnect(): void;
    /**
     * Send data to server
     * @param data text to send
     */
    sendData(data: string): void;
    private clearWebSocket;
}
