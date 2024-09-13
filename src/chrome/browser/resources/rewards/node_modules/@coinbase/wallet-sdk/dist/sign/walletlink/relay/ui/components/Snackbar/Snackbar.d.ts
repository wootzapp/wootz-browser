import { FunctionComponent } from 'preact';
export interface SnackbarInstanceProps {
    message?: string;
    menuItems?: SnackbarMenuItem[];
    autoExpand?: boolean;
}
export interface SnackbarMenuItem {
    isRed: boolean;
    info: string;
    svgWidth: string;
    svgHeight: string;
    path: string;
    defaultFillRule: 'inherit' | 'evenodd';
    defaultClipRule: 'inherit' | 'evenodd';
    onClick: () => void;
}
export declare class Snackbar {
    private readonly darkMode;
    private readonly items;
    private nextItemKey;
    private root;
    constructor();
    attach(el: Element): void;
    presentItem(itemProps: SnackbarInstanceProps): () => void;
    clear(): void;
    private render;
}
export declare const SnackbarContainer: FunctionComponent<{
    darkMode: boolean;
}>;
export declare const SnackbarInstance: FunctionComponent<SnackbarInstanceProps>;
