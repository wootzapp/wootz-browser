import type React from "react";
type QRCodeRendererProps = {
    ecl?: "L" | "M" | "Q" | "H";
    size?: number;
    uri: string;
    clearSize?: number;
    image?: React.ReactNode;
    imageBackground?: string;
};
/**
 * @internal
 */
declare function QRCodeRenderer({ ecl, size: sizeProp, uri, clearSize, image, imageBackground, }: QRCodeRendererProps): import("react/jsx-runtime").JSX.Element;
export default QRCodeRenderer;
//# sourceMappingURL=QRCodeRenderer.d.ts.map