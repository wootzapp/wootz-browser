import { WebGLRenderTarget } from 'three';
/**
 * Debug method to save an offscreen render target to an image; filename should
 * have a .png extension to ensure lossless transmission.
 */
export declare const saveTarget: (target: WebGLRenderTarget, filename: string) => void;
