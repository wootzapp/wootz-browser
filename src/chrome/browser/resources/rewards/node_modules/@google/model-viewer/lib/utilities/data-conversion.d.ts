/**
 * Converts a base64 string which represents a data url
 * into a Blob of the same contents.
 */
export declare const dataUrlToBlob: (base64DataUrl: string) => Promise<Blob>;
