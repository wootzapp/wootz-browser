type RedirectDialogProps = {
    title: string;
    buttonText: string;
    onButtonClick: () => void;
};
export declare class RedirectDialog {
    private readonly darkMode;
    private root;
    constructor();
    attach(): void;
    present(props: RedirectDialogProps): void;
    clear(): void;
    private render;
}
export {};
