import ModelViewerElementBase from '../model-viewer-base.js';
import { Constructor } from '../utilities.js';
export declare const AUTO_ROTATE_DELAY_DEFAULT = 3000;
export declare interface StagingInterface {
    autoRotate: boolean;
    autoRotateDelay: number;
    readonly turntableRotation: number;
    resetTurntableRotation(theta?: number): void;
}
export declare const StagingMixin: <T extends Constructor<ModelViewerElementBase, object>>(ModelViewerElement: T) => {
    new (...args: any[]): StagingInterface;
    prototype: StagingInterface;
} & object & T;
