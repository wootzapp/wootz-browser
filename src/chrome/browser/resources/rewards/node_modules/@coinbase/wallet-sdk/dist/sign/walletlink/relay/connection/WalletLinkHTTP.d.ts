import { ServerMessage } from '../type/ServerMessage';
export declare class WalletLinkHTTP {
    private readonly linkAPIUrl;
    private readonly sessionId;
    private readonly auth;
    constructor(linkAPIUrl: string, sessionId: string, sessionKey: string);
    private markUnseenEventsAsSeen;
    fetchUnseenEvents(): Promise<ServerMessage<'Event'>[]>;
}
