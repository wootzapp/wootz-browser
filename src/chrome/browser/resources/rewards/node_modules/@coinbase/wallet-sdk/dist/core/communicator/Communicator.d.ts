import { Message, MessageID } from '../message';
/**
 * Communicates with a popup window for Coinbase keys.coinbase.com (or another url)
 * to send and receive messages.
 *
 * This class is responsible for opening a popup window, posting messages to it,
 * and listening for responses.
 *
 * It also handles cleanup of event listeners and the popup window itself when necessary.
 */
export declare class Communicator {
    private readonly url;
    private popup;
    private listeners;
    constructor(url?: string);
    /**
     * Posts a message to the popup window
     */
    postMessage: (message: Message) => Promise<void>;
    /**
     * Posts a request to the popup window and waits for a response
     */
    postRequestAndWaitForResponse: <M extends Message>(request: Message & {
        id: MessageID;
    }) => Promise<M>;
    /**
     * Listens for messages from the popup window that match a given predicate.
     */
    onMessage: <M extends Message>(predicate: (_: Partial<M>) => boolean) => Promise<M>;
    /**
     * Closes the popup, rejects all requests and clears the listeners
     */
    private disconnect;
    /**
     * Waits for the popup window to fully load and then sends a version message.
     */
    waitForPopupLoaded: () => Promise<Window>;
}
