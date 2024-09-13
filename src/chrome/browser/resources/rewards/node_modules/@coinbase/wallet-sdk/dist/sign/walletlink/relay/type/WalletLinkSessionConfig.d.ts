export interface WalletLinkSessionConfig {
    webhookId: string;
    webhookUrl: string;
    metadata: {
        [key: string]: string | undefined;
    };
}
