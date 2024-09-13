import { Object3D, Scene, WebGLRenderer } from 'three';
import { ModelScene } from './ModelScene';
export declare type Side = 'back' | 'bottom';
/**
 * The Shadow class creates a shadow that fits a given scene and follows a
 * target. This shadow will follow the scene without any updates needed so long
 * as the shadow and scene are both parented to the same object (call it the
 * scene) and this scene is passed as the target parameter to the shadow's
 * constructor. We also must constrain the scene to motion within the horizontal
 * plane and call the setRotation() method whenever the scene's Y-axis rotation
 * changes. For motion outside of the horizontal plane, this.needsUpdate must be
 * set to true.
 *
 * The softness of the shadow is controlled by changing its resolution, making
 * softer shadows faster, but less precise.
 */
export declare class Shadow extends Object3D {
    private camera;
    private renderTarget;
    private renderTargetBlur;
    private depthMaterial;
    private horizontalBlurMaterial;
    private verticalBlurMaterial;
    private intensity;
    private softness;
    private floor;
    private blurPlane;
    private boundingBox;
    private size;
    private maxDimension;
    private isAnimated;
    needsUpdate: boolean;
    constructor(scene: ModelScene, softness: number, side: Side);
    /**
     * Update the shadow's size and position for a new scene. Softness is also
     * needed, as this controls the shadow's resolution.
     */
    setScene(scene: ModelScene, softness: number, side: Side): void;
    /**
     * Update the shadow's resolution based on softness (between 0 and 1). Should
     * not be called frequently, as this results in reallocation.
     */
    setSoftness(softness: number): void;
    /**
     * Lower-level version of the above function.
     */
    setMapSize(maxMapSize: number): void;
    /**
     * Set the shadow's intensity (0 to 1), which is just its opacity. Turns off
     * shadow rendering if zero.
     */
    setIntensity(intensity: number): void;
    getIntensity(): number;
    /**
     * An offset can be specified to move the
     * shadow vertically relative to the bottom of the scene. Positive is up, so
     * values are generally negative. A small offset keeps our shadow from
     * z-fighting with any baked-in shadow plane.
     */
    setOffset(offset: number): void;
    render(renderer: WebGLRenderer, scene: Scene): void;
    blurShadow(renderer: WebGLRenderer): void;
    dispose(): void;
}
